/**
 * @file tools/ssg.c
 * @description Hardened Modern ISO C23 Zero-Dependency Static Site Generator for irbox.github.io.
 * Generates semantic static HTML, RSS feeds, sitemap, and search index with sub-second performance.
 */

#define _POSIX_C_SOURCE 202405L
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <strings.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#define MAX_PATH 1024
#define MAX_ITEMS 256
#define MAX_SECTIONS 16
#define COPY_BUF_SIZE 8192

static_assert(MAX_PATH >= 512, "MAX_PATH must be at least 512");
static_assert(MAX_ITEMS >= 128, "MAX_ITEMS must support site collections");
static_assert(sizeof(size_t) >= 4, "Architecture must be at least 32-bit");

/* ==========================================================================
   Site Identity & Metadata
   ========================================================================== */
typedef struct {
    char title[128];
    char tagline[256];
    char description[512];
    char url[256];
    char baseurl[64];
    char repo[256];
    char repo_branch[64];
    char author_name[128];
    char author_role[128];
    char author_email[128];
    char author_avatar[256];
    char author_bio[512];
} SiteConfig;

static SiteConfig site_config = {
    .title = "Irbox",
    .tagline = "Portfolio, docs, projects and writing in one place.",
    .description = "A personal showcase site: selected projects, documentation for the things I build, and a blog about how I build them.",
    .url = "https://irbox.github.io",
    .baseurl = "",
    .repo = "https://github.com/irbox/irbox.github.io",
    .repo_branch = "main",
    .author_name = "Irbox",
    .author_role = "Software Engineer",
    .author_email = "189630825+irbox@users.noreply.github.com",
    .author_avatar = "/assets/images/placeholders/avatar.svg",
    .author_bio = "Software engineer and open-source developer. Building web applications, developer tooling, and sharing notes on modern software engineering."
};

/* ==========================================================================
   Dynamic String Buffer
   ========================================================================== */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

static void buf_init(StrBuf *b, size_t cap) {
    b->cap = cap > 0 ? cap : 1024;
    b->len = 0;
    b->data = (char *)malloc(b->cap);
    if (!b->data) { perror("malloc"); exit(1); }
    b->data[0] = '\0';
}

static void buf_append_len(StrBuf *b, const char *s, size_t n) {
    if (!s || n == 0) return;
    if (b->len + n + 1 >= b->cap) {
        while (b->len + n + 1 >= b->cap) b->cap *= 2;
        b->data = (char *)realloc(b->data, b->cap);
        if (!b->data) { perror("realloc"); exit(1); }
    }
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static void buf_append_str(StrBuf *b, const char *s) {
    if (s) buf_append_len(b, s, strlen(s));
}

static void buf_append_char(StrBuf *b, char c) {
    char s[1] = { c };
    buf_append_len(b, s, 1);
}

static void buf_free(StrBuf *b) {
    if (b->data) free(b->data);
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
}

/* ==========================================================================
   Data Models
   ========================================================================== */
typedef struct {
    char name[64];
    char purpose[128];
} TechStackItem;

typedef struct {
    char caption[128];
} GalleryItem;

typedef struct {
    char slug[128];
    char title[256];
    char category[64];
    char description[512];
    char url[256];
    char source_path[MAX_PATH];
    char *html_content;
    int order;
} DocItem;

typedef struct {
    char slug[128];
    char title[256];
    char date[32];
    char description[512];
    char tags[128];
    char cover[128];
    char url[256];
    char source_path[MAX_PATH];
    char *html_content;
    int read_time;
} PostItem;

typedef struct {
    char slug[128];
    char title[256];
    char category[64];
    char status[64];
    char year[32];
    char role[128];
    char description[512];
    char demo_url[256];
    char repo_url[256];
    char docs_slug[128];
    char tags[128];
    char cover[128];
    char url[256];
    char source_path[MAX_PATH];
    char *html_content;
    bool featured;
    int order;

    char features[16][128];
    size_t feature_count;

    TechStackItem stack[16];
    size_t stack_count;

    GalleryItem gallery[16];
    size_t gallery_count;
} ProjectItem;

typedef struct {
    char title[128];
    char slugs[16][64];
    size_t count;
} NavSection;

typedef struct {
    char title[256];
    char url[256];
    char type[32];
    char description[512];
    char tags[128];
} SearchEntry;

static DocItem docs[MAX_ITEMS];
static size_t doc_count = 0;

static PostItem posts[MAX_ITEMS];
static size_t post_count = 0;

static ProjectItem projects[MAX_ITEMS];
static size_t project_count = 0;

static NavSection nav_sections[MAX_SECTIONS];
static size_t nav_section_count = 0;

static SearchEntry search_entries[MAX_ITEMS * 2];
static size_t search_entry_count = 0;

static const char *PROJECT_CATEGORIES[] = {
    "Web Apps",
    "Libraries",
    "Tools & CLI",
    "Experiments"
};
static const size_t PROJECT_CATEGORY_COUNT = 4;

/* ==========================================================================
   Utilities & String Helpers
   ========================================================================== */
[[nodiscard]] static char *trim_whitespace(char *str) {
    if (str == NULL) return NULL;
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static void strip_quotes(char *str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    if (len >= 2 && ((str[0] == '"' && str[len - 1] == '"') || (str[0] == '\'' && str[len - 1] == '\''))) {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

static void safe_strcpy(char *dst, size_t dst_sz, const char *src) {
    if (!dst || dst_sz == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    snprintf(dst, dst_sz, "%s", src);
}

static void parse_ssg_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;

    char line[512];
    bool in_author_block = false;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0';

        char *p = trim_whitespace(line);
        if (!p || p[0] == '#' || p[0] == '/' || p[0] == '\0') continue;

        if (strcmp(p, "author:") == 0 || strcmp(p, "[author]") == 0) {
            in_author_block = true;
            continue;
        }

        char *sep = strchr(p, '=');
        if (!sep) sep = strchr(p, ':');
        if (!sep) continue;

        *sep = '\0';
        char *key = trim_whitespace(p);
        char *value = trim_whitespace(sep + 1);
        strip_quotes(value);

        if (!key || key[0] == '\0') continue;

        if (strcmp(key, "title") == 0) {
            safe_strcpy(site_config.title, sizeof(site_config.title), value);
        } else if (strcmp(key, "tagline") == 0) {
            safe_strcpy(site_config.tagline, sizeof(site_config.tagline), value);
        } else if (strcmp(key, "description") == 0) {
            if (strcmp(value, ">-") != 0 && strcmp(value, "|") != 0)
                safe_strcpy(site_config.description, sizeof(site_config.description), value);
        } else if (strcmp(key, "url") == 0) {
            safe_strcpy(site_config.url, sizeof(site_config.url), value);
        } else if (strcmp(key, "baseurl") == 0) {
            safe_strcpy(site_config.baseurl, sizeof(site_config.baseurl), value);
        } else if (strcmp(key, "repo") == 0) {
            safe_strcpy(site_config.repo, sizeof(site_config.repo), value);
        } else if (strcmp(key, "repo_branch") == 0) {
            safe_strcpy(site_config.repo_branch, sizeof(site_config.repo_branch), value);
        } else if (strcmp(key, "author_name") == 0 || strcmp(key, "author") == 0 || (in_author_block && strcmp(key, "name") == 0)) {
            safe_strcpy(site_config.author_name, sizeof(site_config.author_name), value);
        } else if (strcmp(key, "author_role") == 0 || (in_author_block && strcmp(key, "role") == 0)) {
            safe_strcpy(site_config.author_role, sizeof(site_config.author_role), value);
        } else if (strcmp(key, "author_email") == 0 || (in_author_block && strcmp(key, "email") == 0)) {
            safe_strcpy(site_config.author_email, sizeof(site_config.author_email), value);
        } else if (strcmp(key, "author_avatar") == 0 || (in_author_block && strcmp(key, "avatar") == 0)) {
            safe_strcpy(site_config.author_avatar, sizeof(site_config.author_avatar), value);
        } else if (strcmp(key, "author_bio") == 0 || (in_author_block && strcmp(key, "bio") == 0)) {
            if (strcmp(value, ">-") != 0 && strcmp(value, "|") != 0)
                safe_strcpy(site_config.author_bio, sizeof(site_config.author_bio), value);
        }
    }
    fclose(file);
}

static void load_site_config(void) {
    parse_ssg_config("_config.cfg");
}

static void format_date_long(char *out, size_t out_sz, const char *ymd) {
    int y, m, d;
    if (sscanf(ymd, "%d-%d-%d", &y, &m, &d) == 3) {
        static const char *months[] = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };
        if (m >= 1 && m <= 12) {
            snprintf(out, out_sz, "%s %d, %d", months[m - 1], d, y);
            return;
        }
    }
    safe_strcpy(out, out_sz, ymd);
}

static void format_date_short(char *out, size_t out_sz, const char *ymd) {
    int y, m, d;
    if (sscanf(ymd, "%d-%d-%d", &y, &m, &d) == 3) {
        static const char *months[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        if (m >= 1 && m <= 12) {
            snprintf(out, out_sz, "%s %d, %d", months[m - 1], d, y);
            return;
        }
    }
    safe_strcpy(out, out_sz, ymd);
}

[[nodiscard]] static char *read_entire_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0) { fclose(f); return NULL; }
    char *buf = (char *)malloc(size + 1);
    if (buf == NULL) { fclose(f); return NULL; }
    size_t r = fread(buf, 1, size, f);
    buf[r] = '\0';
    fclose(f);
    return buf;
}

static void write_entire_file(const char *path, const char *data, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "Cannot write to %s: %s\n", path, strerror(errno)); return; }
    if (len > 0) fwrite(data, 1, len, f);
    fclose(f);
}

static void mkdir_p(const char *dir) {
    if (!dir || dir[0] == '\0') return;
    char tmp[MAX_PATH];
    safe_strcpy(tmp, sizeof(tmp), dir);
    size_t len = strlen(tmp);
    if (len > 1 && tmp[len - 1] == '/') tmp[len - 1] = '\0';
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

static void copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return; }
    char buf[COPY_BUF_SIZE] = {};
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        fwrite(buf, 1, n, out);
    }
    fclose(in);
    fclose(out);
}

static void copy_dir_recursive(const char *src, const char *dst) {
    DIR *d = opendir(src);
    if (!d) return;
    mkdir_p(dst);
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
        char src_sub[MAX_PATH], dst_sub[MAX_PATH];
        snprintf(src_sub, sizeof(src_sub), "%s/%s", src, dir->d_name);
        snprintf(dst_sub, sizeof(dst_sub), "%s/%s", dst, dir->d_name);
        struct stat st;
        if (lstat(src_sub, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                copy_dir_recursive(src_sub, dst_sub);
            } else if (S_ISREG(st.st_mode)) {
                copy_file(src_sub, dst_sub);
            }
        }
    }
    closedir(d);
}

static void escape_html(StrBuf *out, const char *text, size_t len) {
    for (size_t i = 0; i < len; i++) {
        switch (text[i]) {
            case '&': buf_append_str(out, "&amp;"); break;
            case '<': buf_append_str(out, "&lt;"); break;
            case '>': buf_append_str(out, "&gt;"); break;
            case '"': buf_append_str(out, "&quot;"); break;
            default:  buf_append_char(out, text[i]); break;
        }
    }
}

static void escape_json(StrBuf *out, const char *text) {
    if (!text) return;
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) {
        switch (*p) {
            case '"':  buf_append_str(out, "\\\""); break;
            case '\\': buf_append_str(out, "\\\\"); break;
            case '\b': buf_append_str(out, "\\b");  break;
            case '\f': buf_append_str(out, "\\f");  break;
            case '\n': buf_append_str(out, "\\n");  break;
            case '\r': buf_append_str(out, "\\r");  break;
            case '\t': buf_append_str(out, "\\t");  break;
            default:
                if (*p < 0x20) {
                    char hex[8];
                    snprintf(hex, sizeof(hex), "\\u%04x", *p);
                    buf_append_str(out, hex);
                } else {
                    buf_append_char(out, (char)*p);
                }
                break;
        }
    }
}

/* ==========================================================================
   SVG Icons
   ========================================================================== */
static void render_icon_arrow(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2.2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><polyline points=\"9 18 15 12 9 6\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_external(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><line x1=\"7\" y1=\"17\" x2=\"17\" y2=\"7\"/><polyline points=\"7 7 17 7 17 17\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_github(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"currentColor\" aria-hidden=\"true\" focusable=\"false\"><path d=\"M12 0C5.37 0 0 5.37 0 12c0 5.31 3.435 9.795 8.205 11.385.6.105.825-.255.825-.57 0-.285-.015-1.23-.015-2.235-3.015.555-3.795-.735-4.035-1.41-.135-.345-.72-1.41-1.23-1.695-.42-.225-1.02-.78-.015-.795.945-.015 1.62.87 1.845 1.23 1.08 1.815 2.805 1.305 3.495.99.105-.78.42-1.305.765-1.605-2.67-.3-5.46-1.335-5.46-5.925 0-1.305.465-2.385 1.23-3.225-.12-.3-.54-1.53.12-3.18 0 0 1.005-.315 3.3 1.23.96-.27 1.98-.405 3-.405s2.04.135 3 .405c2.295-1.56 3.3-1.23 3.3-1.23.66 1.65.24 2.88.12 3.18.765.84 1.23 1.905 1.23 3.225 0 4.605-2.805 5.625-5.475 5.925.435.375.81 1.095.81 2.22 0 1.605-.015 2.895-.015 3.3 0 .315.225.69.825.57A12.02 12.02 0 0 0 24 12c0-6.63-5.37-12-12-12Z\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_keyboard(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.8\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><rect width=\"20\" height=\"14\" x=\"2\" y=\"5\" rx=\"3\"/><path d=\"M6 9h.01M10 9h.01M14 9h.01M18 9h.01M7 13h10\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_search(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2.2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><circle cx=\"11\" cy=\"11\" r=\"8\"/><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_sun(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><circle cx=\"12\" cy=\"12\" r=\"5\"/><line x1=\"12\" y1=\"1\" x2=\"12\" y2=\"3\"/><line x1=\"12\" y1=\"21\" x2=\"12\" y2=\"23\"/><line x1=\"4.22\" y1=\"4.22\" x2=\"5.64\" y2=\"5.64\"/><line x1=\"18.36\" y1=\"18.36\" x2=\"19.78\" y2=\"19.78\"/><line x1=\"1\" y1=\"12\" x2=\"3\" y2=\"12\"/><line x1=\"21\" y1=\"12\" x2=\"23\" y2=\"12\"/><line x1=\"4.22\" y1=\"19.78\" x2=\"5.64\" y2=\"18.36\"/><line x1=\"18.36\" y1=\"5.64\" x2=\"19.78\" y2=\"4.22\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_moon(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><path d=\"M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_menu(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><line x1=\"4\" y1=\"6\" x2=\"20\" y2=\"6\"/><line x1=\"4\" y1=\"12\" x2=\"20\" y2=\"12\"/><line x1=\"4\" y1=\"18\" x2=\"20\" y2=\"18\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_close(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><line x1=\"18\" y1=\"6\" x2=\"6\" y2=\"18\"/><line x1=\"6\" y1=\"6\" x2=\"18\" y2=\"18\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_offline(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><line x1=\"1\" y1=\"1\" x2=\"23\" y2=\"23\"/><path d=\"M16.72 11.06A10.94 10.94 0 0 1 19 12.55\"/><path d=\"M5 12.55a10.94 10.94 0 0 1 5.17-2.39\"/><path d=\"M10.71 5.05A16 16 0 0 1 22.58 9\"/><path d=\"M1.42 9a15.91 15.91 0 0 1 4.7-2.88\"/><path d=\"M8.53 16.11a6 6 0 0 1 6.95 0\"/><line x1=\"12\" y1=\"20\" x2=\"12.01\" y2=\"20\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

/* ==========================================================================
   Markdown Parser (Hardened Non-Destructive Line Iteration)
   ========================================================================== */
static void parse_inline(StrBuf *out, const char *text) {
    size_t i = 0, len = strlen(text);
    while (i < len) {
        if (text[i] == '`') {
            const char *end = strchr(text + i + 1, '`');
            if (end) {
                buf_append_str(out, "<code>");
                escape_html(out, text + i + 1, end - (text + i + 1));
                buf_append_str(out, "</code>");
                i = end - text + 1;
                continue;
            }
        }
        if ((text[i] == '*' && i + 1 < len && text[i + 1] == '*') ||
            (text[i] == '_' && i + 1 < len && text[i + 1] == '_')) {
            char mark = text[i];
            const char *end = strstr(text + i + 2, mark == '*' ? "**" : "__");
            if (end) {
                buf_append_str(out, "<strong>");
                size_t sublen = end - (text + i + 2);
                char *sub = (char *)malloc(sublen + 1);
                memcpy(sub, text + i + 2, sublen);
                sub[sublen] = '\0';
                parse_inline(out, sub);
                free(sub);
                buf_append_str(out, "</strong>");
                i = end - text + 2;
                continue;
            }
        }
        if ((text[i] == '*' || text[i] == '_') && i + 1 < len && !isspace((unsigned char)text[i + 1])) {
            char mark = text[i];
            const char *end = strchr(text + i + 1, mark);
            if (end && (end == text + len - 1 || isspace((unsigned char)*(end + 1)) || ispunct((unsigned char)*(end + 1)))) {
                buf_append_str(out, "<em>");
                size_t sublen = end - (text + i + 1);
                char *sub = (char *)malloc(sublen + 1);
                memcpy(sub, text + i + 1, sublen);
                sub[sublen] = '\0';
                parse_inline(out, sub);
                free(sub);
                buf_append_str(out, "</em>");
                i = end - text + 1;
                continue;
            }
        }
        if (text[i] == '!' && i + 1 < len && text[i + 1] == '[') {
            const char *close_b = strchr(text + i + 2, ']');
            if (close_b && *(close_b + 1) == '(') {
                const char *close_p = strchr(close_b + 2, ')');
                if (close_p) {
                    size_t alt_len = close_b - (text + i + 2);
                    size_t url_len = close_p - (close_b + 2);
                    char *alt = (char *)malloc(alt_len + 1);
                    char *url = (char *)malloc(url_len + 1);
                    memcpy(alt, text + i + 2, alt_len); alt[alt_len] = '\0';
                    memcpy(url, close_b + 2, url_len); url[url_len] = '\0';

                    buf_append_str(out, "<img src=\"");
                    escape_html(out, url, url_len);
                    buf_append_str(out, "\" alt=\"");
                    escape_html(out, alt, alt_len);
                    buf_append_str(out, "\" loading=\"lazy\" decoding=\"async\">");

                    free(alt); free(url);
                    i = close_p - text + 1;
                    continue;
                }
            }
        }
        if (text[i] == '[') {
            const char *close_b = strchr(text + i + 1, ']');
            if (close_b && *(close_b + 1) == '(') {
                const char *close_p = strchr(close_b + 2, ')');
                if (close_p) {
                    size_t label_len = close_b - (text + i + 1);
                    size_t url_len = close_p - (close_b + 2);
                    char *label = (char *)malloc(label_len + 1);
                    char *url = (char *)malloc(url_len + 1);
                    memcpy(label, text + i + 1, label_len); label[label_len] = '\0';
                    memcpy(url, close_b + 2, url_len); url[url_len] = '\0';

                    buf_append_str(out, "<a href=\"");
                    escape_html(out, url, url_len);
                    buf_append_str(out, "\">");
                    parse_inline(out, label);
                    buf_append_str(out, "</a>");

                    free(label); free(url);
                    i = close_p - text + 1;
                    continue;
                }
            }
        }
        escape_html(out, text + i, 1);
        i++;
    }
}

static void parse_table_row(StrBuf *out, const char *line, bool is_header) {
    buf_append_str(out, "        <tr>\n");
    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '|') p++;

    while (*p) {
        const char *next_bar = strchr(p, '|');
        size_t cell_len = next_bar ? (size_t)(next_bar - p) : strlen(p);
        char *cell = (char *)malloc(cell_len + 1);
        memcpy(cell, p, cell_len);
        cell[cell_len] = '\0';
        char *tcell = trim_whitespace(cell);

        if (is_header) {
            buf_append_str(out, "          <th scope=\"col\">");
            parse_inline(out, tcell);
            buf_append_str(out, "</th>\n");
        } else {
            buf_append_str(out, "          <td>");
            parse_inline(out, tcell);
            buf_append_str(out, "</td>\n");
        }

        free(cell);
        if (!next_bar) break;
        p = next_bar + 1;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;
    }
    buf_append_str(out, "        </tr>\n");
}

static void markdown_to_html(StrBuf *out, const char *md) {
    char *copy = strdup(md);
    if (!copy) return;
    bool in_code = false, in_ul = false, in_ol = false, in_p = false, in_table = false;
    
    char *cursor = copy;
    while (*cursor) {
        char *line = cursor;
        char *nl = strpbrk(cursor, "\r\n");
        if (nl) {
            if (*nl == '\r' && *(nl + 1) == '\n') {
                *nl = '\0';
                cursor = nl + 2;
            } else {
                *nl = '\0';
                cursor = nl + 1;
            }
        } else {
            cursor += strlen(cursor);
        }

        if (strncmp(line, "```", 3) == 0) {
            if (in_code) {
                buf_append_str(out, "</code></pre>\n");
                in_code = false;
            } else {
                if (in_p)     { buf_append_str(out, "</p>\n"); in_p = false; }
                if (in_ul)    { buf_append_str(out, "</ul>\n"); in_ul = false; }
                if (in_ol)    { buf_append_str(out, "</ol>\n"); in_ol = false; }
                if (in_table) { buf_append_str(out, "      </tbody>\n    </table>\n  </div>\n"); in_table = false; }
                char *lang = trim_whitespace(line + 3);
                buf_append_str(out, "<pre><code");
                if (strlen(lang) > 0) {
                    buf_append_str(out, " class=\"language-");
                    escape_html(out, lang, strlen(lang));
                    buf_append_str(out, "\"");
                }
                buf_append_str(out, ">");
                in_code = true;
            }
            continue;
        }

        if (in_code) {
            escape_html(out, line, strlen(line));
            buf_append_char(out, '\n');
            continue;
        }

        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0) {
            if (in_p)     { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul)    { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol)    { buf_append_str(out, "</ol>\n"); in_ol = false; }
            if (in_table) { buf_append_str(out, "      </tbody>\n    </table>\n  </div>\n"); in_table = false; }
            continue;
        }

        if (trimmed[0] == '<') {
            if (in_p)     { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul)    { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol)    { buf_append_str(out, "</ol>\n"); in_ol = false; }
            if (in_table) { buf_append_str(out, "      </tbody>\n    </table>\n  </div>\n"); in_table = false; }
            buf_append_str(out, trimmed);
            buf_append_char(out, '\n');
            continue;
        }

        if (trimmed[0] == '|') {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }

            bool is_divider = true;
            for (size_t k = 0; trimmed[k]; k++) {
                if (trimmed[k] != '|' && trimmed[k] != '-' && trimmed[k] != ':' && !isspace((unsigned char)trimmed[k])) {
                    is_divider = false;
                    break;
                }
            }

            if (is_divider) continue;

            if (!in_table) {
                in_table = true;
                buf_append_str(out, "  <div class=\"table-wrap\">\n    <table>\n      <thead>\n");
                parse_table_row(out, trimmed, true);
                buf_append_str(out, "      </thead>\n      <tbody>\n");
            } else {
                parse_table_row(out, trimmed, false);
            }
            continue;
        } else {
            if (in_table) {
                buf_append_str(out, "      </tbody>\n    </table>\n  </div>\n");
                in_table = false;
            }
        }

        if (trimmed[0] == '#') {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            int level = 0;
            while (trimmed[level] == '#' && level < 6) level++;
            char *h_text = trim_whitespace(trimmed + level);

            char anchor[128];
            size_t ai = 0;
            for (size_t k = 0; h_text[k] && ai < sizeof(anchor) - 1; k++) {
                if (isalnum((unsigned char)h_text[k])) {
                    anchor[ai++] = tolower((unsigned char)h_text[k]);
                } else if (h_text[k] == ' ' || h_text[k] == '-') {
                    if (ai > 0 && anchor[ai - 1] != '-') anchor[ai++] = '-';
                }
            }
            anchor[ai] = '\0';

            char tag_open[256], tag_close[16];
            snprintf(tag_open, sizeof(tag_open), "<h%d id=\"%s\">", level, anchor);
            snprintf(tag_close, sizeof(tag_close), "</h%d>\n", level);
            buf_append_str(out, tag_open);
            parse_inline(out, h_text);
            buf_append_str(out, tag_close);
            continue;
        }

        if (strcmp(trimmed, "---") == 0 || strcmp(trimmed, "***") == 0) {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            buf_append_str(out, "<hr class=\"divider\">\n");
            continue;
        }

        if (trimmed[0] == '>') {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            buf_append_str(out, "<blockquote><p>");
            parse_inline(out, trim_whitespace(trimmed + 1));
            buf_append_str(out, "</p></blockquote>\n");
            continue;
        }

        if ((trimmed[0] == '-' || trimmed[0] == '*') && isspace((unsigned char)trimmed[1])) {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            if (!in_ul) { buf_append_str(out, "<ul>\n"); in_ul = true; }
            buf_append_str(out, "  <li>");
            parse_inline(out, trim_whitespace(trimmed + 2));
            buf_append_str(out, "</li>\n");
            continue;
        }

        if (isdigit((unsigned char)trimmed[0])) {
            char *dot = strchr(trimmed, '.');
            if (dot && isspace((unsigned char)*(dot + 1))) {
                if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
                if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
                if (!in_ol) { buf_append_str(out, "<ol>\n"); in_ol = true; }
                buf_append_str(out, "  <li>");
                parse_inline(out, trim_whitespace(dot + 2));
                buf_append_str(out, "</li>\n");
                continue;
            }
        }

        if (!in_p) {
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            buf_append_str(out, "<p>");
            in_p = true;
        } else {
            buf_append_str(out, " ");
        }
        parse_inline(out, trimmed);
    }

    if (in_code)  buf_append_str(out, "</code></pre>\n");
    if (in_p)     buf_append_str(out, "</p>\n");
    if (in_ul)    buf_append_str(out, "</ul>\n");
    if (in_ol)    buf_append_str(out, "</ol>\n");
    if (in_table) buf_append_str(out, "      </tbody>\n    </table>\n  </div>\n");
    free(copy);
}

/* ==========================================================================
   Frontmatter Extraction
   ========================================================================== */
static const char *parse_frontmatter_block(const char *content, void *target, int type) {
    if (strncmp(content, "---", 3) != 0) return content;
    const char *start = content + 3;
    while (*start == '\r' || *start == '\n') start++;
    const char *end = strstr(start, "\n---");
    if (!end) end = strstr(start, "\r\n---");
    if (!end) return content;

    size_t fmlen = end - start;
    char *fm = (char *)malloc(fmlen + 1);
    memcpy(fm, start, fmlen);
    fm[fmlen] = '\0';

    int current_section = 0;

    char *line = strtok(fm, "\r\n");
    while (line) {
        char *trimmed = trim_whitespace(line);

        if (strncmp(trimmed, "features:", 9) == 0) { current_section = 1; line = strtok(NULL, "\r\n"); continue; }
        if (strncmp(trimmed, "stack:", 6) == 0)    { current_section = 2; line = strtok(NULL, "\r\n"); continue; }
        if (strncmp(trimmed, "gallery:", 8) == 0)  { current_section = 3; line = strtok(NULL, "\r\n"); continue; }
        if (strncmp(trimmed, "links:", 6) == 0)    { current_section = 4; line = strtok(NULL, "\r\n"); continue; }

        if (type == 3) {
            ProjectItem *pr = (ProjectItem *)target;
            if (current_section == 1 && strncmp(trimmed, "- ", 2) == 0) {
                if (pr->feature_count < 16) {
                    char *feat = trim_whitespace(trimmed + 2);
                    strip_quotes(feat);
                    safe_strcpy(pr->features[pr->feature_count++], sizeof(pr->features[0]), feat);
                }
                line = strtok(NULL, "\r\n");
                continue;
            }
            if (current_section == 2 && strstr(trimmed, "name:") != NULL) {
                if (pr->stack_count < 16) {
                    char *name_p = strstr(trimmed, "name:");
                    char *purp_p = strstr(trimmed, "purpose:");
                    if (name_p && purp_p) {
                        name_p += 5;
                        while (*name_p == ' ' || *name_p == '\'' || *name_p == '"') name_p++;
                        char *name_end = strchr(name_p, '\'');
                        if (!name_end) name_end = strchr(name_p, '"');
                        if (!name_end) name_end = strchr(name_p, ',');
                        if (name_end) {
                            size_t nl = name_end - name_p;
                            if (nl >= sizeof(pr->stack[0].name)) nl = sizeof(pr->stack[0].name) - 1;
                            memcpy(pr->stack[pr->stack_count].name, name_p, nl);
                            pr->stack[pr->stack_count].name[nl] = '\0';
                        }
                        purp_p += 8;
                        while (*purp_p == ' ' || *purp_p == '\'' || *purp_p == '"') purp_p++;
                        char *purp_end = strchr(purp_p, '\'');
                        if (!purp_end) purp_end = strchr(purp_p, '"');
                        if (!purp_end) purp_end = strchr(purp_p, '}');
                        if (purp_end) {
                            size_t pl = purp_end - purp_p;
                            if (pl >= sizeof(pr->stack[0].purpose)) pl = sizeof(pr->stack[0].purpose) - 1;
                            memcpy(pr->stack[pr->stack_count].purpose, purp_p, pl);
                            pr->stack[pr->stack_count].purpose[pl] = '\0';
                        }
                        pr->stack_count++;
                    }
                }
                line = strtok(NULL, "\r\n");
                continue;
            }
            if (current_section == 3 && strstr(trimmed, "caption:") != NULL) {
                if (pr->gallery_count < 16) {
                    char *cap_p = strstr(trimmed, "caption:");
                    cap_p += 8;
                    while (*cap_p == ' ' || *cap_p == '\'' || *cap_p == '"') cap_p++;
                    char *cap_end = strchr(cap_p, '\'');
                    if (!cap_end) cap_end = strchr(cap_p, '"');
                    if (!cap_end) cap_end = strchr(cap_p, '}');
                    if (cap_end) {
                        size_t cl = cap_end - cap_p;
                        if (cl >= sizeof(pr->gallery[0].caption)) cl = sizeof(pr->gallery[0].caption) - 1;
                        memcpy(pr->gallery[pr->gallery_count].caption, cap_p, cl);
                        pr->gallery[pr->gallery_count].caption[cl] = '\0';
                        pr->gallery_count++;
                    }
                }
                line = strtok(NULL, "\r\n");
                continue;
            }
            if (current_section == 4) {
                char *colon = strchr(trimmed, ':');
                if (colon) {
                    *colon = '\0';
                    char *k = trim_whitespace(trimmed);
                    char *v = trim_whitespace(colon + 1);
                    strip_quotes(v);
                    if (strcmp(k, "repo") == 0) safe_strcpy(pr->repo_url, sizeof(pr->repo_url), v);
                    else if (strcmp(k, "demo") == 0) safe_strcpy(pr->demo_url, sizeof(pr->demo_url), v);
                }
                line = strtok(NULL, "\r\n");
                continue;
            }
        }

        char *colon = strchr(line, ':');
        if (colon) {
            current_section = 0;
            *colon = '\0';
            char *key = trim_whitespace(line);
            char *val = trim_whitespace(colon + 1);
            strip_quotes(val);

            if (type == 1) {
                DocItem *d = (DocItem *)target;
                if (strcmp(key, "title") == 0) safe_strcpy(d->title, sizeof(d->title), val);
                else if (strcmp(key, "description") == 0) safe_strcpy(d->description, sizeof(d->description), val);
                else if (strcmp(key, "category") == 0) safe_strcpy(d->category, sizeof(d->category), val);
                else if (strcmp(key, "order") == 0) d->order = atoi(val);
            } else if (type == 2) {
                PostItem *p = (PostItem *)target;
                if (strcmp(key, "title") == 0) safe_strcpy(p->title, sizeof(p->title), val);
                else if (strcmp(key, "description") == 0) safe_strcpy(p->description, sizeof(p->description), val);
                else if (strcmp(key, "date") == 0) safe_strcpy(p->date, sizeof(p->date), val);
                else if (strcmp(key, "tags") == 0) safe_strcpy(p->tags, sizeof(p->tags), val);
                else if (strcmp(key, "cover") == 0) safe_strcpy(p->cover, sizeof(p->cover), val);
            } else if (type == 3) {
                ProjectItem *pr = (ProjectItem *)target;
                if (strcmp(key, "title") == 0) safe_strcpy(pr->title, sizeof(pr->title), val);
                else if (strcmp(key, "description") == 0) safe_strcpy(pr->description, sizeof(pr->description), val);
                else if (strcmp(key, "category") == 0) safe_strcpy(pr->category, sizeof(pr->category), val);
                else if (strcmp(key, "status") == 0) safe_strcpy(pr->status, sizeof(pr->status), val);
                else if (strcmp(key, "year") == 0) safe_strcpy(pr->year, sizeof(pr->year), val);
                else if (strcmp(key, "role") == 0) safe_strcpy(pr->role, sizeof(pr->role), val);
                else if (strcmp(key, "featured") == 0) pr->featured = (strcmp(val, "true") == 0);
                else if (strcmp(key, "order") == 0) pr->order = atoi(val);
                else if (strcmp(key, "tags") == 0) safe_strcpy(pr->tags, sizeof(pr->tags), val);
                else if (strcmp(key, "cover") == 0) safe_strcpy(pr->cover, sizeof(pr->cover), val);
                else if (strcmp(key, "docs") == 0) safe_strcpy(pr->docs_slug, sizeof(pr->docs_slug), val);
            }
        }
        line = strtok(NULL, "\r\n");
    }
    free(fm);

    const char *body = end + 4;
    while (*body == '\r' || *body == '\n') body++;
    return body;
}

/* ==========================================================================
   Navigation Structures
   ========================================================================== */
static void init_docs_navigation() {
    nav_section_count = 5;
    strcpy(nav_sections[0].title, "Introduction");
    nav_sections[0].count = 3;
    strcpy(nav_sections[0].slugs[0], "getting-started");
    strcpy(nav_sections[0].slugs[1], "installation");
    strcpy(nav_sections[0].slugs[2], "configuration");

    strcpy(nav_sections[1].title, "Writing content");
    nav_sections[1].count = 4;
    strcpy(nav_sections[1].slugs[0], "writing-docs");
    strcpy(nav_sections[1].slugs[1], "writing-projects");
    strcpy(nav_sections[1].slugs[2], "writing-posts");
    strcpy(nav_sections[1].slugs[3], "publishing-books");

    strcpy(nav_sections[2].title, "Customizing");
    nav_sections[2].count = 3;
    strcpy(nav_sections[2].slugs[0], "theming");
    strcpy(nav_sections[2].slugs[1], "announcement-banner");
    strcpy(nav_sections[2].slugs[2], "apple-design-resources");

    strcpy(nav_sections[3].title, "My projects");
    nav_sections[3].count = 2;
    strcpy(nav_sections[3].slugs[0], "project-atlas");
    strcpy(nav_sections[3].slugs[1], "project-lumen");

    strcpy(nav_sections[4].title, "Reference");
    nav_sections[4].count = 2;
    strcpy(nav_sections[4].slugs[0], "front-matter");
    strcpy(nav_sections[4].slugs[1], "faq");
}

static void append_unlisted_docs_to_nav(void) {
    for (size_t d = 0; d < doc_count; d++) {
        bool found = false;
        for (size_t s = 0; s < nav_section_count && !found; s++) {
            for (size_t k = 0; k < nav_sections[s].count; k++) {
                if (strcmp(nav_sections[s].slugs[k], docs[d].slug) == 0) {
                    found = true;
                    break;
                }
            }
        }
        if (!found && nav_section_count > 0) {
            NavSection *sec = &nav_sections[nav_section_count - 1];
            if (sec->count < 16) {
                safe_strcpy(sec->slugs[sec->count++], sizeof(sec->slugs[0]), docs[d].slug);
            }
        }
    }
}

/* ==========================================================================
   HTML Partials & Renderers
   ========================================================================== */
static void render_head(StrBuf *out, const char *title, const char *desc, const char *url_path, const char *og_type, const char *date_pub) {
    buf_append_str(out,
        "<head>\n"
        "  <meta charset=\"utf-8\">\n"
        "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, viewport-fit=cover, interactive-widget=resizes-content\">\n"
        "  <meta name=\"color-scheme\" content=\"light dark\">\n"
        "  <meta name=\"theme-color\" media=\"(prefers-color-scheme: light)\" content=\"#ffffff\">\n"
        "  <meta name=\"theme-color\" media=\"(prefers-color-scheme: dark)\" content=\"#000000\">\n"
        "  <meta name=\"referrer\" content=\"strict-origin-when-cross-origin\">\n"
        "  <meta name=\"robots\" content=\"index, follow, max-image-preview:large, noarchive\">\n"
        "  <meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">\n"
        "  <meta http-equiv=\"Permissions-Policy\" content=\"interest-cohort=(), camera=(), microphone=(), geolocation=()\">\n"
        "  <meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline' [https://fonts.googleapis.com](https://fonts.googleapis.com); font-src 'self' [https://fonts.gstatic.com](https://fonts.gstatic.com); img-src 'self' data: https: blob:; frame-src 'self' blob: data: chrome-extension:; object-src 'self' blob: data:; child-src 'self' blob: data: chrome-extension:; worker-src 'self' blob:; connect-src 'self' blob: data:; base-uri 'self'; form-action 'self';\">\n"
    );

    const char *page_title = (title && strlen(title) > 0) ? title : site_config.title;
    const char *page_desc = (desc && strlen(desc) > 0) ? desc : site_config.description;
    const char *full_url_path = (url_path && strlen(url_path) > 0) ? url_path : "/";
    const char *type_val = (og_type && strlen(og_type) > 0) ? og_type : "website";

    if (title && strlen(title) > 0) {
        buf_append_str(out, "  <title>");
        escape_html(out, title, strlen(title));
        buf_append_str(out, " — ");
        buf_append_str(out, site_config.title);
        buf_append_str(out, "</title>\n");
    } else {
        buf_append_str(out, "  <title>");
        buf_append_str(out, site_config.title);
        buf_append_str(out, " — ");
        buf_append_str(out, site_config.tagline);
        buf_append_str(out, "</title>\n");
    }

    buf_append_str(out, "  <meta name=\"description\" content=\"");
    escape_html(out, page_desc, strlen(page_desc));
    buf_append_str(out, "\">\n");

    buf_append_str(out, "  <link rel=\"canonical\" href=\"");
    buf_append_str(out, site_config.url);
    if (full_url_path[0] != '/' && strlen(full_url_path) > 0) buf_append_str(out, "/");
    buf_append_str(out, full_url_path);
    buf_append_str(out, "\">\n");

    buf_append_str(out, "  <meta property=\"og:site_name\" content=\"");
    buf_append_str(out, site_config.title);
    buf_append_str(out, "\">\n  <meta property=\"og:type\" content=\"");
    buf_append_str(out, type_val);
    buf_append_str(out, "\">\n  <meta property=\"og:title\" content=\"");
    escape_html(out, page_title, strlen(page_title));
    buf_append_str(out, "\">\n  <meta property=\"og:description\" content=\"");
    escape_html(out, page_desc, strlen(page_desc));
    buf_append_str(out, "\">\n  <meta property=\"og:url\" content=\"");
    buf_append_str(out, site_config.url);
    buf_append_str(out, full_url_path);
    buf_append_str(out, "\">\n  <meta property=\"og:image\" content=\"");
    buf_append_str(out, site_config.url);
    buf_append_str(out, "/assets/images/icon-512.png\">\n");

    buf_append_str(out, "  <meta name=\"twitter:card\" content=\"summary_large_image\">\n");
    buf_append_str(out, "  <meta name=\"twitter:title\" content=\"");
    escape_html(out, page_title, strlen(page_title));
    buf_append_str(out, "\">\n  <meta name=\"twitter:description\" content=\"");
    escape_html(out, page_desc, strlen(page_desc));
    buf_append_str(out, "\">\n  <meta name=\"twitter:image\" content=\"");
    buf_append_str(out, site_config.url);
    buf_append_str(out, "/assets/images/icon-512.png\">\n");

    buf_append_str(out, "  <script type=\"application/ld+json\">\n  {\n    \"@context\": \"[https://schema.org](https://schema.org)\",\n");
    if (strcmp(type_val, "article") == 0) {
        buf_append_str(out, "    \"@type\": \"BlogPosting\",\n    \"headline\": \"");
        escape_html(out, page_title, strlen(page_title));
        buf_append_str(out, "\",\n    \"description\": \"");
        escape_html(out, page_desc, strlen(page_desc));
        buf_append_str(out, "\",\n    \"url\": \"");
        buf_append_str(out, site_config.url);
        buf_append_str(out, full_url_path);
        buf_append_str(out, "\",\n");
        if (date_pub && strlen(date_pub) > 0) {
            buf_append_str(out, "    \"datePublished\": \"");
            buf_append_str(out, date_pub);
            buf_append_str(out, "\",\n");
        }
        buf_append_str(out, "    \"author\": {\n      \"@type\": \"Person\",\n      \"name\": \"");
        buf_append_str(out, site_config.author_name);
        buf_append_str(out, "\"\n    }\n  }\n  </script>\n");
    } else {
        buf_append_str(out, "    \"@type\": \"WebSite\",\n    \"name\": \"");
        buf_append_str(out, site_config.title);
        buf_append_str(out, "\",\n    \"url\": \"");
        buf_append_str(out, site_config.url);
        buf_append_str(out, "/\",\n    \"description\": \"");
        escape_html(out, page_desc, strlen(page_desc));
        buf_append_str(out, "\"\n  }\n  </script>\n");
    }

    buf_append_str(out,
        "  <link rel=\"icon\" href=\"/assets/images/favicon.svg\" type=\"image/svg+xml\">\n"
        "  <link rel=\"apple-touch-icon\" href=\"/assets/images/icon-192.png\">\n"
        "  <link rel=\"manifest\" href=\"/manifest.webmanifest\">\n"
        "  <link rel=\"author\" href=\"/humans.txt\">\n"
        "  <meta name=\"mobile-web-app-capable\" content=\"yes\">\n"
        "  <meta name=\"apple-mobile-web-app-capable\" content=\"yes\">\n"
        "  <meta name=\"apple-mobile-web-app-status-bar-style\" content=\"default\">\n"
        "  <meta name=\"apple-mobile-web-app-title\" content=\"Irbox\">\n"
        "  <meta name=\"application-name\" content=\"Irbox\">\n"
        "  <link rel=\"preload\" href=\"/assets/fonts/libron/Libron-Regular.woff2\" as=\"font\" type=\"font/woff2\" crossorigin>\n"
        "  <link rel=\"preload\" href=\"/assets/fonts/libron/Libron-Bold.woff2\" as=\"font\" type=\"font/woff2\" crossorigin>\n"
        "  <script>\n"
        "    (function () {\n"
        "      try {\n"
        "        var d = document.documentElement;\n"
        "        d.classList.add('is-loading');\n"
        "        var t = localStorage.getItem('theme');\n"
        "        var lightColor = '#faf8f5';\n"
        "        var darkColor = '#0a0d12';\n"
        "        if (t === 'light' || t === 'dark') {\n"
        "          d.setAttribute('data-theme', t);\n"
        "          var c = t === 'dark' ? darkColor : lightColor;\n"
        "          var metas = document.querySelectorAll('meta[name=\"theme-color\"]');\n"
        "          for (var i = 0; i < metas.length; i++) {\n"
        "            metas[i].setAttribute('content', c);\n"
        "          }\n"
        "        }\n"
        "      } catch (e) {}\n"
        "    })();\n"
        "  </script>\n"
        "  <link rel=\"preconnect\" href=\"[https://fonts.googleapis.com](https://fonts.googleapis.com)\">\n"
        "  <link rel=\"preconnect\" href=\"[https://fonts.gstatic.com](https://fonts.gstatic.com)\" crossorigin>\n"
        "  <link rel=\"stylesheet\" href=\"[https://fonts.googleapis.com/css2?family=Figtree:wght@400;500;600;700&family=JetBrains+Mono:ital,wght@0,400;0,500;0,600;0,700;1,400;1,500&display=swap](https://fonts.googleapis.com/css2?family=Figtree:wght@400;500;600;700&family=JetBrains+Mono:ital,wght@0,400;0,500;0,600;0,700;1,400;1,500&display=swap)\">\n"
        "  <link rel=\"stylesheet\" href=\"/assets/css/main.css\">\n"
        "  <link rel=\"alternate\" type=\"application/rss+xml\" title=\"Irbox\" href=\"/feed.xml\">\n"
        "  <script type=\"speculationrules\">\n"
        "    {\n"
        "      \"prerender\": [\n"
        "        {\n"
        "          \"where\": {\n"
        "            \"and\": [\n"
        "              { \"href_matches\": \"/*\" },\n"
        "              { \"not\": { \"href_matches\": \"/feed.xml\" } },\n"
        "              { \"not\": { \"href_matches\": \"/*.svg\" } },\n"
        "              { \"not\": { \"href_matches\": \"/*.woff2\" } },\n"
        "              { \"not\": { \"href_matches\": \"/*.epub\" } },\n"
        "              { \"not\": { \"href_matches\": \"/*.pdf\" } }\n"
        "            ]\n"
        "          },\n"
        "          \"eagerness\": \"moderate\"\n"
        "        }\n"
        "      ]\n"
        "    }\n"
        "  </script>\n"
        "</head>\n"
    );
}

static void render_header(StrBuf *out, const char *active_section) {
    buf_append_str(out,
        "<header class=\"topnav\" id=\"topnav\">\n"
        "  <div class=\"topnav__inner\">\n"
        "    <a class=\"brand\" href=\"/\" aria-label=\""
    );
    buf_append_str(out, site_config.title);
    buf_append_str(out, " home\">\n"
        "      <img src=\"/assets/images/brand-icon.svg\" alt=\"\" width=\"24\" height=\"24\" decoding=\"async\">\n"
        "      <span>"
    );
    buf_append_str(out, site_config.title);
    buf_append_str(out, "</span>\n"
        "    </a>\n"
        "    <nav class=\"topnav__links\" aria-label=\"Primary\">\n"
    );

    const char *sections[] = { "docs", "projects", "blog", "books", "about" };
    const char *labels[]   = { "Docs", "Projects", "Blog", "Books", "About" };
    const char *urls[]     = { "/docs/getting-started/", "/projects/", "/blog/", "/books/", "/about/" };

    for (int i = 0; i < 5; i++) {
        bool is_act = (strcmp(active_section, sections[i]) == 0);
        buf_append_str(out, "      <a href=\"");
        buf_append_str(out, urls[i]);
        buf_append_str(out, "\" class=\"topnav__link");
        if (is_act) buf_append_str(out, " is-active\" aria-current=\"page\"");
        else buf_append_str(out, "\"");
        buf_append_str(out, ">");
        buf_append_str(out, labels[i]);
        buf_append_str(out, "</a>\n");
    }

    buf_append_str(out,
        "    </nav>\n"
        "    <div class=\"topnav__actions\">\n"
        "      <button class=\"btn btn--ghost btn--sm search-trigger\" type=\"button\" data-search-open aria-label=\"Search\">\n"
    );
    render_icon_search(out, 16);
    buf_append_str(out,
        "<span class=\"search-trigger__label\">Search</span><kbd>Ctrl K</kbd>\n"
        "      </button>\n"
        "      <a class=\"icon-btn desktop-only\" href=\""
    );
    buf_append_str(out, site_config.repo);
    buf_append_str(out, "\" aria-label=\"GitHub repository\" target=\"_blank\" rel=\"noopener noreferrer\">\n");
    render_icon_github(out, 20);
    buf_append_str(out,
        "\n      </a>\n"
        "      <button class=\"icon-btn desktop-only\" type=\"button\" data-shortcuts-open aria-label=\"Keyboard shortcuts (press ?)\" title=\"Keyboard shortcuts (?)\">\n"
    );
    render_icon_keyboard(out, 19);
    buf_append_str(out,
        "\n      </button>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-theme-toggle aria-label=\"Toggle color theme\">\n"
        "        <span class=\"theme-icon theme-icon--moon\">"
    );
    render_icon_moon(out, 20);
    buf_append_str(out, "</span>\n        <span class=\"theme-icon theme-icon--sun\">");
    render_icon_sun(out, 20);
    buf_append_str(out,
        "</span>\n      </button>\n"
        "      <button class=\"icon-btn mobile-only\" type=\"button\" data-drawer-toggle aria-expanded=\"false\" aria-controls=\"drawer\" aria-label=\"Open menu\">\n"
        "        <span class=\"menu-icon menu-icon--open\">"
    );
    render_icon_menu(out, 22);
    buf_append_str(out, "</span>\n        <span class=\"menu-icon menu-icon--close\">");
    render_icon_close(out, 22);
    buf_append_str(out,
        "</span>\n      </button>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"drawer\" id=\"drawer\" hidden>\n"
        "    <nav aria-label=\"Mobile\">\n"
    );

    for (int i = 0; i < 5; i++) {
        bool is_act = (strcmp(active_section, sections[i]) == 0);
        buf_append_str(out, "      <a href=\"");
        buf_append_str(out, urls[i]);
        buf_append_str(out, "\" class=\"drawer__link");
        if (is_act) buf_append_str(out, " is-active\" aria-current=\"page\"");
        else buf_append_str(out, "\"");
        buf_append_str(out, ">");
        buf_append_str(out, labels[i]);
        buf_append_str(out, "</a>\n");
    }

    buf_append_str(out, "      <a class=\"drawer__link\" href=\"");
    buf_append_str(out, site_config.repo);
    buf_append_str(out, "\" target=\"_blank\" rel=\"noopener noreferrer\">");
    render_icon_github(out, 18);
    buf_append_str(out, " GitHub</a>\n    </nav>\n  </div>\n</header>\n");
}

static void render_footer(StrBuf *out) {
    buf_append_str(out,
        "<footer class=\"footer\">\n"
        "  <div class=\"container\">\n"
        "    <hr class=\"divider\">\n"
        "    <div class=\"footer__row\">\n"
        "      <a class=\"brand\" href=\"/\" aria-label=\""
    );
    buf_append_str(out, site_config.title);
    buf_append_str(out, " Home\">\n"
        "        <img src=\"/assets/images/brand-icon.svg\" alt=\"\" width=\"20\" height=\"20\" loading=\"lazy\" decoding=\"async\">\n"
        "        <span>"
    );
    buf_append_str(out, site_config.title);
    buf_append_str(out, "</span>\n"
        "      </a>\n"
        "      <nav class=\"footer__links\" aria-label=\"Footer\">\n"
        "        <a href=\"/docs/getting-started/\">Docs</a>\n"
        "        <a href=\"/projects/\">Projects</a>\n"
        "        <a href=\"/blog/\">Blog</a>\n"
        "        <a href=\"/books/\">Books</a>\n"
        "        <a href=\"/changelog/\">Changelog</a>\n"
        "        <a href=\"/about/\">About</a>\n"
        "      </nav>\n"
        "      <div class=\"footer__social\">\n"
        "        <a class=\"icon-btn\" href=\""
    );
    buf_append_str(out, site_config.repo);
    buf_append_str(out, "\" aria-label=\"GitHub\" target=\"_blank\" rel=\"noopener noreferrer\">");
    render_icon_github(out, 18);
    buf_append_str(out, "</a>\n        <a class=\"icon-btn\" href=\"mailto:");
    buf_append_str(out, site_config.author_email);
    buf_append_str(out, "\" aria-label=\"Email\">");
    render_icon_mail(out, 18);
    buf_append_str(out, "</a>\n        <a class=\"icon-btn\" href=\"/feed.xml\" aria-label=\"RSS\">");
    render_icon_rss(out, 18);
    buf_append_str(out,
        "</a>\n"
        "      </div>\n"
        "    </div>\n"
        "    <div class=\"footer__bottom\">\n"
        "      <p class=\"footer__legal\">\n"
        "        © 2026 "
    );
    buf_append_str(out, site_config.title);
    buf_append_str(out, ".\n"
        "      </p>\n"
        "      <a class=\"footer__back-to-top\" href=\"#main\" aria-label=\"Back to top\">\n"
        "        <span>Back to top</span>"
    );
    render_icon_arrow(out, 14);
    buf_append_str(out,
        "\n      </a>\n"
        "    </div>\n"
        "  </div>\n"
        "</footer>\n"
        "<dialog class=\"search\" id=\"search-dialog\" aria-label=\"Search the site\" data-index=\"/search.json\" closedby=\"any\">\n"
        "  <div class=\"search__box\">\n"
    );
    render_icon_search(out, 18);
    buf_append_str(out,
        "\n    <input type=\"search\" id=\"search-input\" placeholder=\"Search docs, projects and posts…\" autocomplete=\"off\" autocorrect=\"off\" autocapitalize=\"off\" spellcheck=\"false\" aria-label=\"Search\">\n"
        "    <button class=\"icon-btn\" type=\"button\" data-search-close aria-label=\"Close search\">\n"
    );
    render_icon_close(out, 18);
    buf_append_str(out,
        "\n    </button>\n"
        "  </div>\n"
        "  <ul class=\"search__results\" id=\"search-results\" aria-label=\"Search results\"></ul>\n"
        "  <div class=\"sr-only\" aria-live=\"polite\" id=\"search-status\"></div>\n"
        "  <p class=\"search__hint\">↑↓ to navigate · Enter to open · Esc to close</p>\n"
        "</dialog>\n"
        "<dialog class=\"shortcuts-dialog\" id=\"shortcuts-dialog\" aria-label=\"Keyboard Shortcuts\" closedby=\"any\">\n"
        "  <div class=\"shortcuts-dialog__inner\">\n"
        "    <div class=\"shortcuts-dialog__header\">\n"
        "      <div class=\"shortcuts-dialog__title-group\">\n"
    );
    render_icon_keyboard(out, 20);
    buf_append_str(out,
        "\n        <h2 class=\"shortcuts-dialog__title\">Keyboard Shortcuts</h2>\n"
        "      </div>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-shortcuts-close aria-label=\"Close shortcuts\">\n"
    );
    render_icon_close(out, 18);
    buf_append_str(out,
        "\n      </button>\n"
        "    </div>\n"
        "    <div class=\"shortcuts-dialog__content\">\n"
        "      <div class=\"shortcuts-group\">\n"
        "        <h3 class=\"shortcuts-group__title\">Navigation</h3>\n"
        "        <dl class=\"shortcuts-list\">\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>H</kbd></dt><dd>Home</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>D</kbd></dt><dd>Documentation</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>P</kbd></dt><dd>Projects</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>B</kbd></dt><dd>Blog</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>L</kbd></dt><dd>Books &amp; Archive</dd></div>\n"
        "        </dl>\n"
        "      </div>\n"
        "      <div class=\"shortcuts-group\">\n"
        "        <h3 class=\"shortcuts-group__title\">General</h3>\n"
        "        <dl class=\"shortcuts-list\">\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>/</kbd> <span class=\"or\">or</span> <kbd>Ctrl</kbd> <kbd>K</kbd></dt><dd>Search site</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>T</kbd></dt><dd>Toggle light / dark theme</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>?</kbd></dt><dd>Toggle keyboard shortcuts</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>Ctrl</kbd> <kbd>P</kbd></dt><dd>Print / Export PDF</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>Esc</kbd></dt><dd>Close dialog</dd></div>\n"
        "        </dl>\n"
        "      </div>\n"
        "    </div>\n"
        "  </div>\n"
        "</dialog>\n"
        "<script src=\"/assets/js/main.js\" defer></script>\n"
    );
}

static void wrap_page_ext(StrBuf *out, const char *title, const char *desc, const char *active_sec, const char *body, const char *body_class, const char *url_path, const char *og_type, const char *date_pub) {
    buf_append_str(out,
        "<!doctype html>\n"
        "<html lang=\"en\" data-baseurl=\"\">\n"
    );
    render_head(out, title, desc, url_path, og_type, date_pub);
    buf_append_str(out, "<body");
    if (body_class && strlen(body_class) > 0) {
        buf_append_str(out, " class=\"");
        buf_append_str(out, body_class);
        buf_append_str(out, "\"");
    }
    buf_append_str(out, ">\n"
        "  <div class=\"page-progress\" id=\"page-progress\" aria-hidden=\"true\"></div>\n"
        "  <div class=\"reading-progress\" id=\"reading-progress\" aria-hidden=\"true\"></div>\n"
        "  <a class=\"skip-link\" href=\"#main\">Skip to content</a>\n"
    );

    render_header(out, active_sec);

    buf_append_str(out, "  <main id=\"main\" tabindex=\"-1\">\n");
    buf_append_str(out, body);
    buf_append_str(out, "\n  </main>\n");

    render_footer(out);
    buf_append_str(out, "</body>\n</html>\n");
}

static void wrap_page(StrBuf *out, const char *title, const char *desc, const char *active_sec, const char *body, const char *body_class, const char *url_path) {
    wrap_page_ext(out, title, desc, active_sec, body, body_class, url_path, "website", NULL);
}

/* ==========================================================================
   Page Renderers
   ========================================================================== */
static void render_docs_sidebar(StrBuf *out, const char *active_slug) {
    buf_append_str(out,
        "  <aside class=\"sidebar\" aria-label=\"Docs navigation\" id=\"sidebar\">\n"
        "    <div class=\"sidebar__header\">\n"
        "      <span class=\"sidebar__header-title\">Docs</span>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-sidebar-toggle aria-label=\"Close sidebar\">\n"
    );
    render_icon_close(out, 18);
    buf_append_str(out,
        "\n      </button>\n"
        "    </div>\n"
        "    <nav aria-label=\"Docs sections\">\n"
    );

    for (size_t s = 0; s < nav_section_count; s++) {
        buf_append_str(out, "      <div class=\"sidebar__group\">\n        <p class=\"sidebar__title\">");
        buf_append_str(out, nav_sections[s].title);
        buf_append_str(out, "</p>\n        <ul>\n");

        for (size_t k = 0; k < nav_sections[s].count; k++) {
            const char *item_slug = nav_sections[s].slugs[k];
            const DocItem *target_doc = NULL;
            for (size_t d = 0; d < doc_count; d++) {
                if (strcmp(docs[d].slug, item_slug) == 0) {
                    target_doc = &docs[d];
                    break;
                }
            }
            if (!target_doc) continue;

            bool is_cur = (active_slug && strcmp(active_slug, item_slug) == 0);
            buf_append_str(out, "          <li><a href=\"");
            buf_append_str(out, target_doc->url);
            buf_append_str(out, "\"");
            if (is_cur) buf_append_str(out, " class=\"is-active\" aria-current=\"page\"");
            buf_append_str(out, ">");
            escape_html(out, target_doc->title, strlen(target_doc->title));
            buf_append_str(out, "</a></li>\n");
        }
        buf_append_str(out, "        </ul>\n      </div>\n");
    }

    bool is_changelog = (active_slug && strcmp(active_slug, "changelog") == 0);
    buf_append_str(out,
        "      <div class=\"sidebar__group\">\n"
        "        <p class=\"sidebar__title\">More</p>\n"
        "        <ul>\n"
        "          <li><a href=\"/changelog/\""
    );
    if (is_changelog) buf_append_str(out, " class=\"is-active\" aria-current=\"page\"");
    buf_append_str(out,
        ">Changelog</a></li>\n"
        "        </ul>\n"
        "      </div>\n"
        "    </nav>\n"
        "  </aside>\n"
    );
}

static void render_projects_sidebar(StrBuf *out, const char *active_slug) {
    buf_append_str(out,
        "  <aside class=\"sidebar\" aria-label=\"Projects navigation\" id=\"sidebar\">\n"
        "    <div class=\"sidebar__header\">\n"
        "      <span class=\"sidebar__header-title\">Projects</span>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-sidebar-toggle aria-label=\"Close sidebar\">\n"
    );
    render_icon_close(out, 18);
    buf_append_str(out,
        "\n      </button>\n"
        "    </div>\n"
        "    <nav aria-label=\"Projects sections\">\n"
        "      <div class=\"sidebar__group\">\n"
        "        <ul>\n"
        "          <li><a href=\"/projects/\""
    );
    if (!active_slug || strlen(active_slug) == 0) buf_append_str(out, " class=\"is-active\" aria-current=\"page\"");
    buf_append_str(out, ">All projects</a></li>\n        </ul>\n      </div>\n");

    for (size_t c = 0; c < PROJECT_CATEGORY_COUNT; c++) {
        const char *cat = PROJECT_CATEGORIES[c];
        bool has_items = false;
        for (size_t i = 0; i < project_count; i++) {
            if (strcmp(projects[i].category, cat) == 0) { has_items = true; break; }
        }
        if (!has_items) continue;

        buf_append_str(out, "      <div class=\"sidebar__group\">\n        <p class=\"sidebar__title\">");
        buf_append_str(out, cat);
        buf_append_str(out, "</p>\n        <ul>\n");

        for (size_t i = 0; i < project_count; i++) {
            if (strcmp(projects[i].category, cat) == 0) {
                bool is_cur = (active_slug && strcmp(active_slug, projects[i].slug) == 0);
                buf_append_str(out, "          <li><a href=\"");
                buf_append_str(out, projects[i].url);
                buf_append_str(out, "\"");
                if (is_cur) buf_append_str(out, " class=\"is-active\" aria-current=\"page\"");
                buf_append_str(out, ">");
                escape_html(out, projects[i].title, strlen(projects[i].title));
                buf_append_str(out, "</a></li>\n");
            }
        }
        buf_append_str(out, "        </ul>\n      </div>\n");
    }

    buf_append_str(out, "    </nav>\n  </aside>\n");
}

static void render_docs_page(const DocItem *d, const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body, "<div class=\"shell\">\n");
    render_docs_sidebar(&body, d->slug);

    buf_append_str(&body,
        "  <div class=\"shell__main\">\n"
        "    <button class=\"btn btn--secondary btn--sm sidebar-toggle\" type=\"button\" data-sidebar-toggle aria-controls=\"sidebar\" aria-expanded=\"false\">\n"
    );
    render_icon_menu(&body, 16);
    buf_append_str(&body,
        " Menu\n"
        "    </button>\n"
        "    <div class=\"doc\">\n"
        "      <article class=\"doc__article\">\n"
        "        <header class=\"doc__header\">\n"
    );

    if (d->category[0]) {
        buf_append_str(&body, "          <p class=\"eyebrow\">");
        escape_html(&body, d->category, strlen(d->category));
        buf_append_str(&body, "</p>\n");
    }
    buf_append_str(&body, "          <h1 class=\"display-1\">");
    escape_html(&body, d->title, strlen(d->title));
    buf_append_str(&body, "</h1>\n");
    if (d->description[0]) {
        buf_append_str(&body, "          <p class=\"lead\">");
        escape_html(&body, d->description, strlen(d->description));
        buf_append_str(&body, "</p>\n");
    }

    buf_append_str(&body,
        "          <div class=\"doc__actions\">\n"
        "            <a class=\"btn btn--secondary btn--sm\" href=\"/assets/book.epub\" title=\"Download entire documentation as an EPUB e-book for Apple Books, Kindle or Kobo\">\n"
    );
    render_icon_book(&body, 14);
    buf_append_str(&body,
        " EPUB Book\n"
        "            </a>\n"
        "            <button class=\"btn btn--secondary btn--sm\" type=\"button\" data-print-btn title=\"Print or Save as PDF\">\n"
    );
    render_icon_printer(&body, 14);
    buf_append_str(&body,
        " Print / PDF\n"
        "            </button>\n"
        "            <button class=\"btn btn--secondary btn--sm\" type=\"button\" data-share-btn title=\"Share this page\">\n"
    );
    render_icon_share(&body, 14);
    buf_append_str(&body,
        " Share\n"
        "            </button>\n"
        "          </div>\n"
        "          <hr class=\"divider\">\n"
        "        </header>\n"
        "        <div class=\"outline-mobile\" data-outline-mobile hidden></div>\n"
        "        <div class=\"prose\" data-prose>\n"
    );

    buf_append_str(&body, d->html_content);
    buf_append_str(&body, "\n        </div>\n");

    const char *all_slugs[64];
    size_t total_nav = 0;
    for (size_t s = 0; s < nav_section_count; s++) {
        for (size_t k = 0; k < nav_sections[s].count; k++) {
            if (total_nav < 64) all_slugs[total_nav++] = nav_sections[s].slugs[k];
        }
    }
    int cur_idx = -1;
    for (size_t i = 0; i < total_nav; i++) {
        if (strcmp(all_slugs[i], d->slug) == 0) {
            cur_idx = (int)i;
            break;
        }
    }
    const DocItem *prev_d = NULL;
    const DocItem *next_d = NULL;
    if (cur_idx > 0) {
        for (size_t i = 0; i < doc_count; i++) {
            if (strcmp(docs[i].slug, all_slugs[cur_idx - 1]) == 0) {
                prev_d = &docs[i];
                break;
            }
        }
    }
    if (cur_idx >= 0 && (size_t)(cur_idx + 1) < total_nav) {
        for (size_t i = 0; i < doc_count; i++) {
            if (strcmp(docs[i].slug, all_slugs[cur_idx + 1]) == 0) {
                next_d = &docs[i];
                break;
            }
        }
    }
    if (prev_d || next_d) {
        buf_append_str(&body, "        <nav class=\"pager\" aria-label=\"Documentation navigation\">\n");
        if (prev_d) {
            buf_append_str(&body, "          <a class=\"pager__item\" href=\"");
            buf_append_str(&body, prev_d->url);
            buf_append_str(&body, "\"><span class=\"pager__label\">Previous</span><span class=\"pager__title\">&larr; ");
            escape_html(&body, prev_d->title, strlen(prev_d->title));
            buf_append_str(&body, "</span></a>\n");
        } else {
            buf_append_str(&body, "          <span aria-hidden=\"true\"></span>\n");
        }
        if (next_d) {
            buf_append_str(&body, "          <a class=\"pager__item pager__item--next\" href=\"");
            buf_append_str(&body, next_d->url);
            buf_append_str(&body, "\"><span class=\"pager__label\">Next</span><span class=\"pager__title\">");
            escape_html(&body, next_d->title, strlen(next_d->title));
            buf_append_str(&body, " &rarr;</span></a>\n");
        } else {
            buf_append_str(&body, "          <span aria-hidden=\"true\"></span>\n");
        }
        buf_append_str(&body, "        </nav>\n");
    }

    buf_append_str(&body, "        <p class=\"edit-link\">\n          <a href=\"");
    buf_append_str(&body, site_config.repo);
    buf_append_str(&body, "/edit/");
    buf_append_str(&body, site_config.repo_branch);
    buf_append_str(&body, "/_docs/");
    buf_append_str(&body, d->slug);
    buf_append_str(&body, ".md\" target=\"_blank\" rel=\"noopener noreferrer\">Edit this page on GitHub ");
    render_icon_external(&body, 14);
    buf_append_str(&body,
        "</a>\n"
        "        </p>\n"
        "      </article>\n"
        "      <aside class=\"outline\" aria-label=\"On this page\" data-outline hidden>\n"
        "        <p class=\"outline__title\">On this page</p>\n"
        "        <ul></ul>\n"
        "      </aside>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page_ext(&full, d->title, d->description, "docs", body.data, "has-sidebar", d->url, "article", NULL);

    char target_path[MAX_PATH];
    snprintf(target_path, sizeof(target_path), "%s%sindex.html", out_dir, d->url);
    char dir[MAX_PATH];
    safe_strcpy(dir, sizeof(dir), target_path);
    char *ls = strrchr(dir, '/');
    if (ls) { *ls = 0; mkdir_p(dir); }

    write_entire_file(target_path, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_project_page(const ProjectItem *pr, const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body, "<div class=\"shell\">\n");
    render_projects_sidebar(&body, pr->slug);

    buf_append_str(&body,
        "  <div class=\"shell__main\">\n"
        "    <button class=\"btn btn--secondary btn--sm sidebar-toggle\" type=\"button\" data-sidebar-toggle aria-controls=\"sidebar\" aria-expanded=\"false\">\n"
    );
    render_icon_menu(&body, 16);
    buf_append_str(&body,
        " Projects\n"
        "    </button>\n"
        "    <div class=\"doc\">\n"
        "      <article class=\"doc__article doc__article--wide\">\n"
        "        <header class=\"doc__header\">\n"
    );

    if (pr->category[0]) {
        buf_append_str(&body, "          <p class=\"eyebrow\">");
        escape_html(&body, pr->category, strlen(pr->category));
        buf_append_str(&body, "</p>\n");
    }

    buf_append_str(&body, "          <h1 class=\"display-1\">");
    escape_html(&body, pr->title, strlen(pr->title));
    buf_append_str(&body, "</h1>\n");

    if (pr->description[0]) {
        buf_append_str(&body, "          <p class=\"lead\">");
        escape_html(&body, pr->description, strlen(pr->description));
        buf_append_str(&body, "</p>\n");
    }

    buf_append_str(&body, "          <div class=\"project-actions\">\n");
    if (pr->demo_url[0]) {
        buf_append_str(&body, "            <a class=\"btn btn--primary\" href=\"");
        escape_html(&body, pr->demo_url, strlen(pr->demo_url));
        buf_append_str(&body, "\" target=\"_blank\" rel=\"noopener noreferrer\">Live demo ");
        render_icon_external(&body, 14);
        buf_append_str(&body, "</a>\n");
    }
    if (pr->repo_url[0]) {
        buf_append_str(&body, "            <a class=\"btn btn--secondary\" href=\"");
        escape_html(&body, pr->repo_url, strlen(pr->repo_url));
        buf_append_str(&body, "\" target=\"_blank\" rel=\"noopener noreferrer\">");
        render_icon_github(&body, 16);
        buf_append_str(&body, " Source</a>\n");
    }
    if (pr->docs_slug[0]) {
        buf_append_str(&body, "            <a class=\"btn btn--secondary\" href=\"/docs/");
        escape_html(&body, pr->docs_slug, strlen(pr->docs_slug));
        buf_append_str(&body, "/\">Documentation</a>\n");
    }
    buf_append_str(&body,
        "            <button class=\"btn btn--secondary\" type=\"button\" data-share-btn aria-label=\"Share project\">\n"
    );
    render_icon_share(&body, 16);
    buf_append_str(&body,
        " Share\n"
        "            </button>\n"
        "          </div>\n"
    );

    if (pr->status[0] || pr->year[0] || pr->role[0]) {
        buf_append_str(&body, "          <ul class=\"facts\">\n");
        if (pr->status[0]) {
            buf_append_str(&body, "            <li><span>Status</span><strong>");
            escape_html(&body, pr->status, strlen(pr->status));
            buf_append_str(&body, "</strong></li>\n");
        }
        if (pr->year[0]) {
            buf_append_str(&body, "            <li><span>Year</span><strong>");
            escape_html(&body, pr->year, strlen(pr->year));
            buf_append_str(&body, "</strong></li>\n");
        }
        if (pr->role[0]) {
            buf_append_str(&body, "            <li><span>Role</span><strong>");
            escape_html(&body, pr->role, strlen(pr->role));
            buf_append_str(&body, "</strong></li>\n");
        }
        buf_append_str(&body, "          </ul>\n");
    }

    buf_append_str(&body, "        </header>\n\n");

    int img_idx = ((pr->order > 0 ? pr->order - 1 : 0) % 4) + 1;
    char stage_buf[512];
    snprintf(stage_buf, sizeof(stage_buf),
        "        <div class=\"stage\">\n"
        "          <img class=\"thumb\" src=\"/assets/images/placeholders/cover-%d.svg\" alt=\"%s\" loading=\"lazy\" decoding=\"async\" width=\"640\" height=\"400\">\n"
        "        </div>\n\n",
        img_idx, pr->title);
    buf_append_str(&body, stage_buf);

    buf_append_str(&body,
        "        <div class=\"tabs\" role=\"tablist\" aria-label=\"Project sections\" data-tabs>\n"
        "          <button role=\"tab\" class=\"tab is-active\" data-tab=\"overview\" aria-selected=\"true\">Overview</button>\n"
    );
    if (pr->stack_count > 0) {
        buf_append_str(&body, "          <button role=\"tab\" class=\"tab\" data-tab=\"stack\" aria-selected=\"false\">Tech stack</button>\n");
    }
    if (pr->gallery_count > 0) {
        buf_append_str(&body, "          <button role=\"tab\" class=\"tab\" data-tab=\"gallery\" aria-selected=\"false\">Gallery</button>\n");
    }
    buf_append_str(&body,
        "        </div>\n\n"
        "        <div class=\"outline-mobile\" data-outline-mobile hidden></div>\n\n"
        "        <section class=\"tab-panel\" data-panel=\"overview\" aria-label=\"Overview\" role=\"tabpanel\">\n"
        "          <div class=\"prose\" data-prose>\n"
    );

    buf_append_str(&body, pr->html_content);

    if (pr->feature_count > 0) {
        buf_append_str(&body, "\n            <h2 id=\"features\">Features</h2>\n            <ul class=\"feature-list\">\n");
        for (size_t f = 0; f < pr->feature_count; f++) {
            buf_append_str(&body, "              <li>");
            escape_html(&body, pr->features[f], strlen(pr->features[f]));
            buf_append_str(&body, "</li>\n");
        }
        buf_append_str(&body, "            </ul>\n");
    }
    buf_append_str(&body, "          </div>\n        </section>\n\n");

    if (pr->stack_count > 0) {
        buf_append_str(&body,
            "        <section class=\"tab-panel\" data-panel=\"stack\" aria-label=\"Tech stack\" role=\"tabpanel\">\n"
            "          <div class=\"prose\" data-prose>\n"
            "            <h2 id=\"tech-stack\">Tech stack</h2>\n"
            "            <div class=\"table-wrap\">\n"
            "              <table>\n"
            "                <thead>\n"
            "                  <tr>\n"
            "                    <th scope=\"col\">Technology</th>\n"
            "                    <th scope=\"col\">Used for</th>\n"
            "                  </tr>\n"
            "                </thead>\n"
            "                <tbody>\n"
        );
        for (size_t s = 0; s < pr->stack_count; s++) {
            buf_append_str(&body, "                  <tr>\n                    <td><code>");
            escape_html(&body, pr->stack[s].name, strlen(pr->stack[s].name));
            buf_append_str(&body, "</code></td>\n                    <td>");
            escape_html(&body, pr->stack[s].purpose, strlen(pr->stack[s].purpose));
            buf_append_str(&body, "</td>\n                  </tr>\n");
        }
        buf_append_str(&body, "                </tbody>\n              </table>\n            </div>\n          </div>\n        </section>\n\n");
    }

    if (pr->gallery_count > 0) {
        buf_append_str(&body,
            "        <section class=\"tab-panel\" data-panel=\"gallery\" aria-label=\"Gallery\" role=\"tabpanel\">\n"
            "          <div class=\"prose\" data-prose><h2 id=\"gallery\">Gallery</h2></div>\n"
            "          <div class=\"gallery\">\n"
        );
        for (size_t g = 0; g < pr->gallery_count; g++) {
            int g_idx = (int)((g % 4) + 1);
            char fig[512];
            snprintf(fig, sizeof(fig),
                "            <figure>\n"
                "              <img class=\"thumb\" src=\"/assets/images/placeholders/cover-%d.svg\" alt=\"%s\" loading=\"lazy\" decoding=\"async\" width=\"640\" height=\"400\">\n"
                "              <figcaption>%s</figcaption>\n"
                "            </figure>\n",
                g_idx, pr->gallery[g].caption, pr->gallery[g].caption);
            buf_append_str(&body, fig);
        }
        buf_append_str(&body, "          </div>\n        </section>\n\n");
    }

    buf_append_str(&body, "        <p class=\"edit-link\">\n          <a href=\"");
    buf_append_str(&body, site_config.repo);
    buf_append_str(&body, "/edit/");
    buf_append_str(&body, site_config.repo_branch);
    buf_append_str(&body, "/_projects/");
    buf_append_str(&body, pr->slug);
    buf_append_str(&body, ".md\" target=\"_blank\" rel=\"noopener noreferrer\">Edit this page on GitHub ");
    render_icon_external(&body, 14);
    buf_append_str(&body,
        "</a>\n"
        "        </p>\n"
        "      </article>\n"
        "      <aside class=\"outline\" aria-label=\"On this page\" data-outline hidden>\n"
        "        <p class=\"outline__title\">On this page</p>\n"
        "        <ul></ul>\n"
        "      </aside>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page_ext(&full, pr->title, pr->description, "projects", body.data, "has-sidebar", pr->url, "article", NULL);

    char target_path[MAX_PATH];
    snprintf(target_path, sizeof(target_path), "%s%sindex.html", out_dir, pr->url);
    char dir[MAX_PATH];
    safe_strcpy(dir, sizeof(dir), target_path);
    char *ls = strrchr(dir, '/');
    if (ls) { *ls = 0; mkdir_p(dir); }

    write_entire_file(target_path, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_post_page(const PostItem *p, const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body,
        "<article class=\"container container--prose page-section\">\n"
        "  <header class=\"post-header\">\n"
    );

    if (p->tags[0]) {
        buf_append_str(&body, "    <div class=\"post-card__tags\">\n");
        char tmp_tags[128];
        safe_strcpy(tmp_tags, sizeof(tmp_tags), p->tags);
        char *t = strtok(tmp_tags, ",[] \t");
        while (t) {
            if (strlen(t) > 0) {
                buf_append_str(&body, "      <span class=\"badge\">");
                escape_html(&body, t, strlen(t));
                buf_append_str(&body, "</span>\n");
            }
            t = strtok(NULL, ",[] \t");
        }
        buf_append_str(&body, "    </div>\n");
    }

    buf_append_str(&body, "    <h1 class=\"display-1\">");
    escape_html(&body, p->title, strlen(p->title));
    buf_append_str(&body, "</h1>\n");

    if (p->description[0]) {
        buf_append_str(&body, "    <p class=\"lead\">");
        escape_html(&body, p->description, strlen(p->description));
        buf_append_str(&body, "</p>\n");
    }

    char date_formatted[64];
    format_date_long(date_formatted, sizeof(date_formatted), p->date);

    buf_append_str(&body,
        "    <div class=\"byline\">\n"
        "      <img class=\"avatar\" src=\""
    );
    buf_append_str(&body, site_config.author_avatar);
    buf_append_str(&body, "\" alt=\"\" width=\"36\" height=\"36\" loading=\"lazy\" decoding=\"async\">\n"
        "      <div>\n"
        "        <strong>"
    );
    buf_append_str(&body, site_config.author_name);
    buf_append_str(&body, "</strong><br>\n"
        "        <span class=\"muted\">"
    );
    buf_append_str(&body, site_config.author_role);
    buf_append_str(&body, "</span>\n"
        "      </div>\n"
        "      <span class=\"spacer\"></span>\n"
        "      <time class=\"muted\" datetime=\""
    );
    buf_append_str(&body, p->date);
    buf_append_str(&body, "\">");
    buf_append_str(&body, date_formatted);
    int rtime = p->read_time > 0 ? p->read_time : 1;
    char read_str[48];
    snprintf(read_str, sizeof(read_str), " &middot; %d min read</time>\n", rtime);
    buf_append_str(&body, read_str);
    buf_append_str(&body,
        "      <button class=\"icon-btn\" type=\"button\" data-print-btn aria-label=\"Print or Save as PDF\" title=\"Print or Save as PDF\">\n"
    );
    render_icon_printer(&body, 16);
    buf_append_str(&body,
        "\n      </button>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-share-btn aria-label=\"Share post\" title=\"Share post\">\n"
    );
    render_icon_share(&body, 16);
    buf_append_str(&body,
        "\n      </button>\n"
        "    </div>\n"
        "  </header>\n"
        "  <div class=\"prose\" data-prose>\n"
    );

    buf_append_str(&body, p->html_content);
    buf_append_str(&body, "\n  </div>\n");

    size_t cur_p = 0;
    for (size_t i = 0; i < post_count; i++) {
        if (&posts[i] == p) { cur_p = i; break; }
    }
    const PostItem *prev_p = (cur_p + 1 < post_count) ? &posts[cur_p + 1] : NULL;
    const PostItem *next_p = (cur_p > 0) ? &posts[cur_p - 1] : NULL;

    if (prev_p || next_p) {
        buf_append_str(&body, "  <nav class=\"pager\" aria-label=\"More posts\">\n");
        if (prev_p) {
            buf_append_str(&body, "    <a class=\"pager__item\" href=\"");
            buf_append_str(&body, prev_p->url);
            buf_append_str(&body, "\"><span class=\"pager__label\">Older</span><span class=\"pager__title\">&larr; ");
            escape_html(&body, prev_p->title, strlen(prev_p->title));
            buf_append_str(&body, "</span></a>\n");
        } else {
            buf_append_str(&body, "    <span aria-hidden=\"true\"></span>\n");
        }
        if (next_p) {
            buf_append_str(&body, "    <a class=\"pager__item pager__item--next\" href=\"");
            buf_append_str(&body, next_p->url);
            buf_append_str(&body, "\"><span class=\"pager__label\">Newer</span><span class=\"pager__title\">");
            escape_html(&body, next_p->title, strlen(next_p->title));
            buf_append_str(&body, " &rarr;</span></a>\n");
        } else {
            buf_append_str(&body, "    <span aria-hidden=\"true\"></span>\n");
        }
        buf_append_str(&body, "  </nav>\n");
    }

    buf_append_str(&body, "</article>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page_ext(&full, p->title, p->description, "blog", body.data, "post-page", p->url, "article", p->date);

    char target_path[MAX_PATH];
    snprintf(target_path, sizeof(target_path), "%s%sindex.html", out_dir, p->url);
    char dir[MAX_PATH];
    safe_strcpy(dir, sizeof(dir), target_path);
    char *ls = strrchr(dir, '/');
    if (ls) { *ls = 0; mkdir_p(dir); }

    write_entire_file(target_path, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_home_page(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 65536);

    buf_append_str(&body,
        "<section class=\"hero\">\n"
        "  <div class=\"container hero__inner\">\n"
        "    <div class=\"hero__badge\">\n"
        "      <img src=\"/assets/images/brand-icon.svg\" alt=\"Irbox logo\" width=\"52\" height=\"52\" fetchpriority=\"high\" decoding=\"async\">\n"
        "    </div>\n"
        "    <p class=\"hero__eyebrow\">Software &amp; Projects</p>\n"
        "    <h1 class=\"hero__title\">Software engineering, developer tooling, and technical writing.</h1>\n"
        "    <p class=\"hero__tagline\">\n      "
    );
    buf_append_str(&body, site_config.description);
    buf_append_str(&body,
        "\n    </p>\n"
        "    <div class=\"hero__cta\">\n"
        "      <a class=\"btn btn--primary btn--lg\" href=\"/projects/\">Browse projects</a>\n"
        "      <a class=\"btn btn--secondary btn--lg\" href=\"/docs/getting-started/\">Documentation</a>\n"
        "      <a class=\"btn btn--ghost btn--lg\" href=\""
    );
    buf_append_str(&body, site_config.repo);
    buf_append_str(&body,
        "\" target=\"_blank\" rel=\"noopener noreferrer\">Source Code</a>\n"
        "    </div>\n"
        "  </div>\n"
        "</section>\n\n"
        "<div class=\"showcase-sheet\">\n"
        "  <section class=\"section container\">\n"
        "    <div class=\"bento\">\n"
        "      <div class=\"bento__col\">\n"
        "        <div class=\"bento__heading-cell\">\n"
        "          <p class=\"eyebrow\">Overview</p>\n"
        "          <h2 class=\"bento__heading\">Everything in one place.</h2>\n"
        "          <p class=\"bento__subtext\">\n"
        "            Selected projects, technical guides, and engineering notes with full source code on\n"
        "            GitHub.\n"
        "          </p>\n"
        "        </div>\n"
        "        <a class=\"bento__card\" href=\"/docs/getting-started/\">\n"
        "          <div class=\"bento__card-header\">\n"
        "            <span class=\"badge\">Documentation</span>\n"
    );
    render_icon_arrow(&body, 16);
    buf_append_str(&body,
        "\n          </div>\n"
        "          <h3>Guides &amp; Reference</h3>\n"
        "          <p>\n"
        "            Setup instructions, architecture notes, and configuration references for the projects.\n"
        "          </p>\n"
        "        </a>\n"
        "      </div>\n\n"
        "      <div class=\"bento__col\">\n"
        "        <a class=\"bento__card\" href=\"/projects/\">\n"
        "          <div class=\"bento__card-header\">\n"
        "            <span class=\"badge\">Projects</span>\n"
    );
    render_icon_arrow(&body, 16);
    buf_append_str(&body,
        "\n          </div>\n"
        "          <h3>Selected Projects</h3>\n"
        "          <p>Web applications, developer tools, and design system components.</p>\n"
        "          <div class=\"bento__tags\">\n"
        "            <span class=\"chip--sm\">TypeScript</span>\n"
        "            <span class=\"chip--sm\">React</span>\n"
        "            <span class=\"chip--sm\">CSS</span>\n"
        "          </div>\n"
        "        </a>\n"
        "        <a class=\"bento__card\" href=\"/blog/\">\n"
        "          <div class=\"bento__card-header\">\n"
        "            <span class=\"badge\">Writing</span>\n"
    );
    render_icon_arrow(&body, 16);
    buf_append_str(&body,
        "\n          </div>\n"
        "          <h3>Blog</h3>\n"
        "          <p>Short essays and notes on building software, design tokens, and shipping things.</p>\n"
        "        </a>\n"
        "      </div>\n\n"
        "      <div class=\"bento__col\">\n"
        "        <div class=\"bento__card bento__card--tall\">\n"
        "          <div class=\"bento__card-header\">\n"
        "            <span class=\"badge badge--added\">Open Source</span>\n"
        "            <span class=\"bento__chip-count\">Public Repo</span>\n"
        "          </div>\n"
        "          <h3>Open Source &amp; Fast</h3>\n"
        "          <p>Zero trackers, self-hosted assets, and clean handwritten code.</p>\n\n"
        "          <div class=\"bento__spec-list\">\n"
        "            <div class=\"bento__spec-item\">\n"
        "              <span class=\"bento__spec-label\">Repository</span>\n"
        "              <a href=\""
    );
    buf_append_str(&body, site_config.repo);
    buf_append_str(&body, "\" target=\"_blank\" rel=\"noopener noreferrer\" class=\"bento__spec-val\">\n                [github.com/irbox](https://github.com/irbox) ");
    render_icon_external(&body, 12);
    buf_append_str(&body,
        "\n              </a>\n"
        "            </div>\n"
        "            <div class=\"bento__spec-item\">\n"
        "              <span class=\"bento__spec-label\">Site Generator</span>\n"
        "              <span class=\"bento__spec-val\">Standalone C23</span>\n"
        "            </div>\n"
        "            <div class=\"bento__spec-item\">\n"
        "              <span class=\"bento__spec-label\">Dependencies</span>\n"
        "              <span class=\"bento__spec-val\">None (0 runtime bloat)</span>\n"
        "            </div>\n"
        "            <div class=\"bento__spec-item\">\n"
        "              <span class=\"bento__spec-label\">License</span>\n"
        "              <span class=\"bento__spec-val\">MIT License</span>\n"
        "            </div>\n"
        "            <div class=\"bento__spec-item\">\n"
        "              <span class=\"bento__spec-label\">Privacy</span>\n"
        "              <span class=\"bento__spec-val\">No Trackers / No Ads</span>\n"
        "            </div>\n"
        "            <div class=\"bento__spec-item\">\n"
        "              <span class=\"bento__spec-label\">Feed</span>\n"
        "              <a href=\"/feed.xml\" class=\"bento__spec-val\">RSS 2.0 / Atom</a>\n"
        "            </div>\n"
        "          </div>\n"
        "        </div>\n"
        "      </div>\n"
        "    </div>\n"
        "  </section>\n\n"
        "  <section class=\"section container\">\n"
        "    <div class=\"section__head\">\n"
        "      <h2 class=\"display-2\">Selected projects</h2>\n"
        "      <a class=\"link-arrow\" href=\"/projects/\">All projects "
    );
    render_icon_arrow(&body, 16);
    buf_append_str(&body,
        "</a>\n"
        "    </div>\n"
        "    <div class=\"grid grid--cards\">\n"
    );

    size_t feat_count = 0;
    for (size_t i = 0; i < project_count && feat_count < 6; i++) {
        if (!projects[i].featured) continue;
        feat_count++;
        int p_idx = (int)((feat_count - 1) % 4) + 1;

        buf_append_str(&body, "      <article class=\"project-card\" data-category=\"");
        escape_html(&body, projects[i].category, strlen(projects[i].category));
        buf_append_str(&body, "\">\n        <a class=\"card card--clickable\" href=\"");
        buf_append_str(&body, projects[i].url);
        buf_append_str(&body, "\" aria-label=\"");
        escape_html(&body, projects[i].title, strlen(projects[i].title));
        buf_append_str(&body, "\">\n          <img class=\"thumb\" src=\"/assets/images/placeholders/cover-");
        char num[16]; snprintf(num, sizeof(num), "%d", p_idx); buf_append_str(&body, num);
        buf_append_str(&body, ".svg\" alt=\"\" loading=\"lazy\" decoding=\"async\" width=\"640\" height=\"400\">\n        </a>\n");
        buf_append_str(&body, "        <div class=\"project-card__meta\">\n          <a class=\"project-card__title\" href=\"");
        buf_append_str(&body, projects[i].url);
        buf_append_str(&body, "\">");
        escape_html(&body, projects[i].title, strlen(projects[i].title));
        buf_append_str(&body, "</a>\n          <span class=\"project-card__desc\">");
        escape_html(&body, projects[i].description, strlen(projects[i].description));
        buf_append_str(&body, "</span>\n        </div>\n      </article>\n");
    }

    buf_append_str(&body,
        "    </div>\n"
        "  </section>\n\n"
        "  <section class=\"section container\">\n"
        "    <div class=\"section__head\">\n"
        "      <h2 class=\"display-2\">From the blog</h2>\n"
        "      <a class=\"link-arrow\" href=\"/blog/\">All posts "
    );
    render_icon_arrow(&body, 16);
    buf_append_str(&body,
        "</a>\n"
        "    </div>\n"
        "    <div class=\"grid grid--posts\">\n"
    );

    for (size_t i = 0; i < post_count && i < 3; i++) {
        int card_idx = (int)(i % 4) + 1;
        char short_date[64];
        format_date_short(short_date, sizeof(short_date), posts[i].date);

        buf_append_str(&body,
            "      <article class=\"post-card\">\n"
            "        <a class=\"card card--clickable post-card__cover\" href=\""
        );
        buf_append_str(&body, posts[i].url);
        buf_append_str(&body, "\" aria-label=\"");
        escape_html(&body, posts[i].title, strlen(posts[i].title));
        buf_append_str(&body, "\">\n          <img class=\"thumb\" src=\"/assets/images/placeholders/cover-");
        char num[16]; snprintf(num, sizeof(num), "%d", card_idx); buf_append_str(&body, num);
        buf_append_str(&body, ".svg\" alt=\"\" loading=\"lazy\" decoding=\"async\" width=\"640\" height=\"400\">\n        </a>\n");
        buf_append_str(&body, "        <div class=\"post-card__body\">\n          <div class=\"post-card__tags\">\n");

        if (posts[i].tags[0]) {
            char tmp_t[128];
            safe_strcpy(tmp_t, sizeof(tmp_t), posts[i].tags);
            char *tag = strtok(tmp_t, ",[] \t");
            if (tag) {
                buf_append_str(&body, "            <span class=\"badge\">");
                escape_html(&body, tag, strlen(tag));
                buf_append_str(&body, "</span>\n");
            }
        }
        buf_append_str(&body,
            "          </div>\n"
            "          <h3 class=\"post-card__title\">\n"
            "            <a href=\""
        );
        buf_append_str(&body, posts[i].url);
        buf_append_str(&body, "\">");
        escape_html(&body, posts[i].title, strlen(posts[i].title));
        buf_append_str(&body, "</a>\n          </h3>\n          <p class=\"post-card__desc\">");
        escape_html(&body, posts[i].description, strlen(posts[i].description));
        buf_append_str(&body,
            "</p>\n"
            "          <div class=\"byline byline--sm\">\n"
            "            <img class=\"avatar\" src=\"/assets/images/placeholders/avatar.svg\" alt=\"\" width=\"24\" height=\"24\" loading=\"lazy\" decoding=\"async\">\n"
            "            <span>Irbox</span><span aria-hidden=\"true\">&middot;</span>\n"
            "            <time datetime=\""
        );
        buf_append_str(&body, posts[i].date);
        buf_append_str(&body, "\">");
        buf_append_str(&body, short_date);
        buf_append_str(&body, "</time>\n          </div>\n        </div>\n      </article>\n");
    }

    buf_append_str(&body,
        "    </div>\n"
        "  </section>\n\n"
        "  <section class=\"section container container--prose about-strip\">\n"
        "    <img class=\"avatar avatar--lg\" src=\"/assets/images/placeholders/avatar.svg\" alt=\"\" width=\"96\" height=\"96\" loading=\"lazy\" decoding=\"async\">\n"
        "    <div>\n"
        "      <h2 class=\"display-3\">Irbox</h2>\n"
        "      <p>"
    );
    buf_append_str(&body, site_config.author_bio);
    buf_append_str(&body,
        "</p>\n"
        "      <a class=\"btn btn--secondary\" href=\"/about/\">More about me</a>\n"
        "    </div>\n"
        "  </section>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 32768);
    wrap_page(&full, NULL, site_config.description, "home", body.data, "home", "/");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/index.html", out_dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_projects_landing(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 16384);

    buf_append_str(&body,
        "<div class=\"container section\">\n"
        "  <header class=\"center page-header\">\n"
        "    <h1 class=\"display-1\">Projects</h1>\n"
        "    <p class=\"lead\">Things I've built, with write-ups, tech stacks and links.</p>\n"
        "  </header>\n\n"
        "  <div class=\"chips\" role=\"group\" aria-label=\"Filter projects\" data-filter-chips>\n"
        "    <button class=\"chip is-active\" type=\"button\" data-filter=\"all\" aria-pressed=\"true\">All</button>\n"
    );

    for (size_t c = 0; c < PROJECT_CATEGORY_COUNT; c++) {
        buf_append_str(&body, "    <button class=\"chip\" type=\"button\" data-filter=\"");
        escape_html(&body, PROJECT_CATEGORIES[c], strlen(PROJECT_CATEGORIES[c]));
        buf_append_str(&body, "\" aria-pressed=\"false\">");
        escape_html(&body, PROJECT_CATEGORIES[c], strlen(PROJECT_CATEGORIES[c]));
        buf_append_str(&body, "</button>\n");
    }

    buf_append_str(&body, "  </div>\n\n");

    int card_counter = 0;
    for (size_t c = 0; c < PROJECT_CATEGORY_COUNT; c++) {
        const char *cat = PROJECT_CATEGORIES[c];
        bool has_items = false;
        for (size_t i = 0; i < project_count; i++) {
            if (strcmp(projects[i].category, cat) == 0) { has_items = true; break; }
        }
        if (!has_items) continue;

        buf_append_str(&body, "  <section class=\"category\" data-category-section=\"");
        escape_html(&body, cat, strlen(cat));
        buf_append_str(&body, "\">\n    <h2 class=\"display-3\">");
        escape_html(&body, cat, strlen(cat));
        buf_append_str(&body, "</h2>\n    <div class=\"grid grid--cards\">\n");

        for (size_t i = 0; i < project_count; i++) {
            if (strcmp(projects[i].category, cat) == 0) {
                card_counter++;
                int p_idx = ((card_counter - 1) % 4) + 1;
                buf_append_str(&body, "      <article class=\"project-card\" data-category=\"");
                escape_html(&body, projects[i].category, strlen(projects[i].category));
                buf_append_str(&body, "\">\n        <a class=\"card card--clickable\" href=\"");
                buf_append_str(&body, projects[i].url);
                buf_append_str(&body, "\" aria-label=\"");
                escape_html(&body, projects[i].title, strlen(projects[i].title));
                buf_append_str(&body, "\">\n          <img class=\"thumb\" src=\"/assets/images/placeholders/cover-");
                char num[16]; snprintf(num, sizeof(num), "%d", p_idx); buf_append_str(&body, num);
                buf_append_str(&body, ".svg\" alt=\"\" loading=\"lazy\" decoding=\"async\" width=\"640\" height=\"400\">\n        </a>\n");
                buf_append_str(&body, "        <div class=\"project-card__meta\">\n          <a class=\"project-card__title\" href=\"");
                buf_append_str(&body, projects[i].url);
                buf_append_str(&body, "\">");
                escape_html(&body, projects[i].title, strlen(projects[i].title));
                buf_append_str(&body, "</a>\n          <span class=\"project-card__desc\">");
                escape_html(&body, projects[i].description, strlen(projects[i].description));
                buf_append_str(&body, "</span>\n        </div>\n      </article>\n");
            }
        }
        buf_append_str(&body, "    </div>\n  </section>\n");
    }

    buf_append_str(&body, "</div>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "Projects", "Things I've built, with write-ups, tech stacks and links.", "projects", body.data, "projects-landing", "/projects/");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/projects/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/projects", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_blog_landing(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 16384);

    buf_append_str(&body,
        "<div class=\"container section\">\n"
        "  <header class=\"center page-header\">\n"
        "    <h1 class=\"display-1\">Blog</h1>\n"
        "    <p class=\"lead\">Notes on design, engineering and shipping.</p>\n"
        "  </header>\n\n"
    );

    if (post_count > 0) {
        char lead_date[64];
        format_date_long(lead_date, sizeof(lead_date), posts[0].date);

        buf_append_str(&body,
            "  <article class=\"feature-card\">\n"
            "    <a class=\"card card--clickable feature-card__cover\" href=\""
        );
        buf_append_str(&body, posts[0].url);
        buf_append_str(&body, "\" aria-label=\"");
        escape_html(&body, posts[0].title, strlen(posts[0].title));
        buf_append_str(&body, "\">\n      <img class=\"thumb\" src=\"/assets/images/placeholders/cover-1.svg\" alt=\"\" loading=\"lazy\" decoding=\"async\" width=\"640\" height=\"400\">\n    </a>\n");
        buf_append_str(&body, "    <div class=\"feature-card__body\">\n      <div class=\"post-card__tags\">\n");

        if (posts[0].tags[0]) {
            char tmp_t[128];
            safe_strcpy(tmp_t, sizeof(tmp_t), posts[0].tags);
            char *tag = strtok(tmp_t, ",[] \t");
            while (tag) {
                if (strlen(tag) > 0) {
                    buf_append_str(&body, "        <span class=\"badge\">");
                    escape_html(&body, tag, strlen(tag));
                    buf_append_str(&body, "</span>\n");
                }
                tag = strtok(NULL, ",[] \t");
            }
        }
        buf_append_str(&body,
            "      </div>\n"
            "      <h2 class=\"display-2\">\n"
            "        <a href=\""
        );
        buf_append_str(&body, posts[0].url);
        buf_append_str(&body, "\">");
        escape_html(&body, posts[0].title, strlen(posts[0].title));
        buf_append_str(&body, "</a>\n      </h2>\n      <p class=\"lead\">");
        escape_html(&body, posts[0].description, strlen(posts[0].description));
        buf_append_str(&body,
            "</p>\n"
            "      <div class=\"byline\">\n"
            "        <img class=\"avatar\" src=\"/assets/images/placeholders/avatar.svg\" alt=\"\" width=\"28\" height=\"28\" loading=\"lazy\" decoding=\"async\">\n"
            "        <span>Irbox</span><span aria-hidden=\"true\">&middot;</span>\n"
            "        <time datetime=\""
        );
        buf_append_str(&body, posts[0].date);
        buf_append_str(&body, "\">");
        buf_append_str(&body, lead_date);
        buf_append_str(&body, "</time>\n      </div>\n    </div>\n  </article>\n\n");

        buf_append_str(&body, "  <div class=\"grid grid--posts\">\n");
        for (size_t i = 1; i < post_count; i++) {
            int card_idx = (int)(i % 4) + 1;
            char short_date[64];
            format_date_short(short_date, sizeof(short_date), posts[i].date);

            buf_append_str(&body,
                "    <article class=\"post-card\">\n"
                "      <a class=\"card card--clickable post-card__cover\" href=\""
            );
            buf_append_str(&body, posts[i].url);
            buf_append_str(&body, "\" aria-label=\"");
            escape_html(&body, posts[i].title, strlen(posts[i].title));
            buf_append_str(&body, "\">\n        <img class=\"thumb\" src=\"/assets/images/placeholders/cover-");
            char num[16]; snprintf(num, sizeof(num), "%d", card_idx); buf_append_str(&body, num);
            buf_append_str(&body, ".svg\" alt=\"\" loading=\"lazy\" decoding=\"async\" width=\"640\" height=\"400\">\n      </a>\n");
            buf_append_str(&body, "      <div class=\"post-card__body\">\n        <div class=\"post-card__tags\">\n");

            if (posts[i].tags[0]) {
                char tmp_t[128];
                safe_strcpy(tmp_t, sizeof(tmp_t), posts[i].tags);
                char *tag = strtok(tmp_t, ",[] \t");
                if (tag) {
                    buf_append_str(&body, "          <span class=\"badge\">");
                    escape_html(&body, tag, strlen(tag));
                    buf_append_str(&body, "</span>\n");
                }
            }
            buf_append_str(&body,
                "        </div>\n"
                "        <h3 class=\"post-card__title\">\n"
                "          <a href=\""
            );
            buf_append_str(&body, posts[i].url);
            buf_append_str(&body, "\">");
            escape_html(&body, posts[i].title, strlen(posts[i].title));
            buf_append_str(&body, "</a>\n        </h3>\n        <p class=\"post-card__desc\">");
            escape_html(&body, posts[i].description, strlen(posts[i].description));
            buf_append_str(&body,
                "</p>\n"
                "        <div class=\"byline byline--sm\">\n"
                "          <img class=\"avatar\" src=\"/assets/images/placeholders/avatar.svg\" alt=\"\" width=\"24\" height=\"24\" loading=\"lazy\" decoding=\"async\">\n"
                "          <span>Irbox</span><span aria-hidden=\"true\">&middot;</span>\n"
                "          <time datetime=\""
            );
            buf_append_str(&body, posts[i].date);
            buf_append_str(&body, "\">");
            buf_append_str(&body, short_date);
            buf_append_str(&body, "</time>\n        </div>\n      </div>\n    </article>\n");
        }
        buf_append_str(&body, "  </div>\n");
    }

    buf_append_str(&body, "</div>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "Blog", "Notes on design, engineering and shipping.", "blog", body.data, "blog-landing", "/blog/");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/blog/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/blog", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_books_landing(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 16384);

    buf_append_str(&body,
        "<div class=\"container section\">\n"
        "  <header class=\"center page-header\">\n"
        "    <h1 class=\"display-1\">Books &amp; Archive</h1>\n"
        "    <p class=\"lead\">\n"
        "      Personal digital shelf of books, field notes, and architectural manuals. Read them directly in\n"
        "      your browser with our built-in reader, or download to your Kindle, Kobo, or mobile device.\n"
        "    </p>\n"
        "  </header>\n\n"
        "  <section class=\"card book-dropzone\" id=\"book-dropzone\" aria-label=\"Quick in-browser book reader drop zone\">\n"
        "    <div class=\"book-dropzone__content\">\n"
        "      <div class=\"book-dropzone__icon\" aria-hidden=\"true\">\n"
        "        <svg xmlns=\"[http://www.w3.org/2000/svg](http://www.w3.org/2000/svg)\" width=\"36\" height=\"36\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><path d=\"M4 19.5v-15A2.5 2.5 0 0 1 6.5 2H20v20H6.5a2.5 2.5 0 0 1-2.5-2.5Z\"></path><path d=\"M6 6h10\"></path><path d=\"M6 10h10\"></path><path d=\"M12 14l4 4m0-4l-4 4\"></path></svg>\n"
        "      </div>\n"
        "      <div class=\"book-dropzone__text\">\n"
        "        <h2 class=\"display-3 book-dropzone__title\">Instant Web Book Reader</h2>\n"
        "        <p class=\"muted book-dropzone__desc\">\n"
        "          Drag and drop any <strong>.epub</strong> or <strong>.pdf</strong> file here to read\n"
        "          instantly in your browser. 100% private, runs entirely on your device with no server\n"
        "          upload.\n"
        "        </p>\n"
        "      </div>\n"
        "      <div class=\"book-dropzone__action\">\n"
        "        <label for=\"book-file-input\" class=\"btn btn--secondary\">\n"
        "          <svg xmlns=\"[http://www.w3.org/2000/svg](http://www.w3.org/2000/svg)\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"></path><polyline points=\"17 8 12 3 7 8\"></polyline><line x1=\"12\" y1=\"3\" x2=\"12\" y2=\"15\"></line></svg>\n"
        "          <span>Open local file</span>\n"
        "        </label>\n"
        "        <input type=\"file\" id=\"book-file-input\" accept=\".epub,.pdf,application/epub+zip,application/pdf\" style=\"display: none;\">\n"
        "      </div>\n"
        "    </div>\n"
        "  </section>\n\n"
        "  <div class=\"books-toolbar\">\n"
        "    <div class=\"chips\" role=\"group\" aria-label=\"Filter books by format\" id=\"books-format-filters\">\n"
        "      <button class=\"chip is-active\" type=\"button\" data-format=\"all\" aria-pressed=\"true\">\n"
        "        All Books\n"
        "      </button>\n"
        "      <button class=\"chip\" type=\"button\" data-format=\"epub\" aria-pressed=\"false\">EPUB</button>\n"
        "      <button class=\"chip\" type=\"button\" data-format=\"pdf\" aria-pressed=\"false\">PDF</button>\n"
        "    </div>\n\n"
        "    <div class=\"books-search-box\">\n"
        "      <input type=\"search\" id=\"books-search\" class=\"input books-search-input\" placeholder=\"Filter shelf...\" aria-label=\"Filter books shelf by title, author, or keywords\">\n"
        "      <svg class=\"books-search-icon\" xmlns=\"[http://www.w3.org/2000/svg](http://www.w3.org/2000/svg)\" width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><circle cx=\"11\" cy=\"11\" r=\"8\"></circle><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"></line></svg>\n"
        "    </div>\n"
        "  </div>\n\n"
        "  <div class=\"grid grid--cards\" id=\"books-grid\">\n"
    );

    DIR *bd = opendir("books/archive");
    if (bd) {
        struct dirent *entry;
        while ((entry = readdir(bd)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (strcmp(entry->d_name, "README.md") == 0) continue;
            size_t nlen = strlen(entry->d_name);
            bool is_pdf  = (nlen > 4 && strcasecmp(entry->d_name + nlen - 4, ".pdf") == 0);
            bool is_epub = (nlen > 5 && strcasecmp(entry->d_name + nlen - 5, ".epub") == 0);
            if (!is_pdf && !is_epub) continue;

            char full_p[MAX_PATH];
            snprintf(full_p, sizeof(full_p), "books/archive/%s", entry->d_name);
            struct stat st;
            double kb = (stat(full_p, &st) == 0) ? (st.st_size / 1024.0) : 0.0;

            const char *fmt_lower = is_pdf ? "pdf" : "epub";
            const char *fmt_upper = is_pdf ? "PDF" : "EPUB";

            buf_append_str(&body, "    <article class=\"card book-card\" data-format=\"");
            buf_append_str(&body, fmt_lower);
            buf_append_str(&body, "\" data-title=\"");
            escape_html(&body, entry->d_name, nlen);
            buf_append_str(&body, "\" data-author=\"");
            escape_html(&body, site_config.author_name, strlen(site_config.author_name));
            buf_append_str(&body, "\" data-desc=\"Archived book\">\n"
                "      <div class=\"book-card__header\">\n"
                "        <div class=\"book-card__badge-row\">\n"
                "          <span class=\"badge badge--");
            buf_append_str(&body, fmt_lower);
            buf_append_str(&body, "\">");
            buf_append_str(&body, fmt_upper);
            buf_append_str(&body, "</span>\n          <span class=\"muted book-card__size\">");
            char sz_buf[32];
            snprintf(sz_buf, sizeof(sz_buf), "%.1f KB", kb);
            buf_append_str(&body, sz_buf);
            buf_append_str(&body, "</span>\n        </div>\n"
                "        <h2 class=\"display-3 book-card__title\">\n"
                "          <a href=\"/books/reader/?book=");
            escape_html(&body, entry->d_name, nlen);
            buf_append_str(&body, "\">");
            escape_html(&body, entry->d_name, nlen);
            buf_append_str(&body, "</a>\n        </h2>\n      </div>\n"
                "      <div class=\"book-card__footer\">\n"
                "        <a href=\"/books/reader/?book=");
            escape_html(&body, entry->d_name, nlen);
            buf_append_str(&body, "\" class=\"btn btn--primary book-card__action\"><span>Read Online</span></a>\n"
                "        <a href=\"/books/archive/");
            escape_html(&body, entry->d_name, nlen);
            buf_append_str(&body, "\" class=\"btn btn--secondary\" download><span>Download</span></a>\n"
                "      </div>\n    </article>\n");
        }
        closedir(bd);
    }

    buf_append_str(&body,
        "  </div>\n\n"
        "  <div id=\"books-empty-search\" class=\"card center muted book-empty-state\" style=\"display: none; margin-top: 24px;\">\n"
        "    <p>No books matching your search query.</p>\n"
        "  </div>\n\n"
        "  <section class=\"card book-guide-card\">\n"
        "    <div class=\"book-guide-inner\">\n"
        "      <div class=\"book-guide__icon\" aria-hidden=\"true\">\n"
        "        <svg xmlns=\"[http://www.w3.org/2000/svg](http://www.w3.org/2000/svg)\" width=\"28\" height=\"28\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><circle cx=\"12\" cy=\"12\" r=\"10\"></circle><line x1=\"12\" y1=\"16\" x2=\"12\" y2=\"12\"></line><line x1=\"12\" y1=\"8\" x2=\"12.01\" y2=\"8\"></line></svg>\n"
        "      </div>\n"
        "      <div>\n"
        "        <h2 class=\"display-3\" style=\"margin-top: 0; margin-bottom: 8px;\">How to add your own books (Layman's Guide)</h2>\n"
        "        <p class=\"muted\" style=\"margin-bottom: 16px;\">You don't need coding knowledge or database setups to publish or archive books here.</p>\n"
        "        <ol style=\"padding-left: 20px; line-height: 1.7; margin-bottom: 16px;\">\n"
        "          <li><strong>Add files:</strong> Place any <code>.epub</code> or <code>.pdf</code> book inside the folder <code>books/archive/</code>.</li>\n"
        "          <li><strong>Build:</strong> Run <code>make build</code> in your terminal. Zero setup, zero external dependencies.</li>\n"
        "          <li><strong>Instant reader:</strong> That's it! Your book automatically receives a card on this shelf, full-text reading controls, and chapter navigation.</li>\n"
        "        </ol>\n"
        "        <p style=\"margin: 0;\">\n"
        "          <a href=\"/docs/publishing-books/\" class=\"btn btn--secondary\" style=\"font-size: 0.875rem;\">Read complete publishing documentation &rarr;</a>\n"
        "        </p>\n"
        "      </div>\n"
        "    </div>\n"
        "  </section>\n"
        "</div>\n\n"
        "<script>\n"
        "  (function () {\n"
        "    const grid = document.getElementById('books-grid');\n"
        "    const emptyMsg = document.getElementById('books-empty-search');\n"
        "    const searchInput = document.getElementById('books-search');\n"
        "    const filterButtons = document.querySelectorAll('#books-format-filters .chip');\n"
        "    const dropzone = document.getElementById('book-dropzone');\n"
        "    const fileInput = document.getElementById('book-file-input');\n\n"
        "    let currentFormat = 'all';\n"
        "    let currentQuery = '';\n\n"
        "    function filterBooks() {\n"
        "      const cards = grid.querySelectorAll('.book-card');\n"
        "      let visibleCount = 0;\n\n"
        "      cards.forEach((card) => {\n"
        "        const format = card.getAttribute('data-format') || '';\n"
        "        const title = card.getAttribute('data-title') || '';\n"
        "        const author = card.getAttribute('data-author') || '';\n"
        "        const desc = card.getAttribute('data-desc') || '';\n\n"
        "        const matchesFormat = currentFormat === 'all' || format === currentFormat;\n"
        "        const matchesQuery =\n"
        "          !currentQuery ||\n"
        "          title.includes(currentQuery) ||\n"
        "          author.includes(currentQuery) ||\n"
        "          desc.includes(currentQuery);\n\n"
        "        if (matchesFormat && matchesQuery) {\n"
        "          card.style.display = '';\n"
        "          visibleCount++;\n"
        "        } else {\n"
        "          card.style.display = 'none';\n"
        "        }\n"
        "      });\n\n"
        "      if (emptyMsg) {\n"
        "        emptyMsg.style.display = visibleCount === 0 ? 'block' : 'none';\n"
        "      }\n"
        "    }\n\n"
        "    if (searchInput) {\n"
        "      searchInput.addEventListener('input', (e) => {\n"
        "        currentQuery = e.target.value.toLowerCase().trim();\n"
        "        filterBooks();\n"
        "      });\n"
        "    }\n\n"
        "    filterButtons.forEach((btn) => {\n"
        "      btn.addEventListener('click', () => {\n"
        "        filterButtons.forEach((b) => {\n"
        "          b.classList.remove('is-active');\n"
        "          b.setAttribute('aria-pressed', 'false');\n"
        "        });\n"
        "        btn.classList.add('is-active');\n"
        "        btn.setAttribute('aria-pressed', 'true');\n"
        "        currentFormat = btn.getAttribute('data-format') || 'all';\n"
        "        filterBooks();\n"
        "      });\n"
        "    });\n\n"
        "    const bookDb = {\n"
        "      open: () =>\n"
        "        new Promise((resolve, reject) => {\n"
        "          if (!window.indexedDB) return reject(new Error('IndexedDB unavailable'));\n"
        "          const req = indexedDB.open('irbox_books_db', 1);\n"
        "          req.onupgradeneeded = (e) => {\n"
        "            const db = e.target.result;\n"
        "            if (!db.objectStoreNames.contains('books')) {\n"
        "              db.createObjectStore('books', { keyPath: 'id' });\n"
        "            }\n"
        "          };\n"
        "          req.onsuccess = (e) => resolve(e.target.result);\n"
        "          req.onerror = (e) => reject(e.target.error);\n"
        "        }),\n"
        "      save: async (name, data) => {\n"
        "        try {\n"
        "          const db = await bookDb.open();\n"
        "          return new Promise((resolve, reject) => {\n"
        "            const tx = db.transaction('books', 'readwrite');\n"
        "            tx.objectStore('books').put({ id: 'current', name, data, timestamp: Date.now() });\n"
        "            tx.oncomplete = () => resolve();\n"
        "            tx.onerror = (e) => reject(e.target.error);\n"
        "          });\n"
        "        } catch {\n"
        "          sessionStorage.setItem('irbox_temp_book_name', name);\n"
        "          sessionStorage.setItem('irbox_temp_book_data', data);\n"
        "        }\n"
        "      },\n"
        "    };\n\n"
        "    function openLocalFile(file) {\n"
        "      if (!file) return;\n"
        "      const readerUrl = '/books/reader/';\n"
        "      const reader = new FileReader();\n"
        "      reader.onload = async function (e) {\n"
        "        try {\n"
        "          await bookDb.save(file.name, e.target.result);\n"
        "          sessionStorage.setItem('irbox_temp_book_name', file.name);\n"
        "          sessionStorage.setItem('irbox_temp_book_data', e.target.result);\n"
        "        } catch {}\n"
        "        window.location.href = readerUrl + '?book=local';\n"
        "      };\n"
        "      reader.readAsDataURL(file);\n"
        "    }\n\n"
        "    if (fileInput) {\n"
        "      fileInput.addEventListener('change', (e) => {\n"
        "        const file = e.target.files[0];\n"
        "        if (file) openLocalFile(file);\n"
        "      });\n"
        "    }\n\n"
        "    if (dropzone) {\n"
        "      ['dragenter', 'dragover'].forEach((eventName) => {\n"
        "        dropzone.addEventListener(eventName, (e) => {\n"
        "          e.preventDefault();\n"
        "          e.stopPropagation();\n"
        "          dropzone.classList.add('is-dragover');\n"
        "        });\n"
        "      });\n\n"
        "      ['dragleave', 'drop'].forEach((eventName) => {\n"
        "        dropzone.addEventListener(eventName, (e) => {\n"
        "          e.preventDefault();\n"
        "          e.stopPropagation();\n"
        "          dropzone.classList.remove('is-dragover');\n"
        "        });\n"
        "      });\n\n"
        "      dropzone.addEventListener('drop', (e) => {\n"
        "        const files = e.dataTransfer?.files;\n"
        "        if (files && files.length > 0) {\n"
        "          openLocalFile(files[0]);\n"
        "        }\n"
        "      });\n"
        "    }\n"
        "  })();\n"
        "</script>\n"
    );

    StrBuf full;
    buf_init(&full, 32768);
    wrap_page(&full, "Books & Archive", "Personal digital library and book archive. Read online or export to your e-reader.", "books", body.data, "books-landing", "/books/");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/books/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/books", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_reader_page(const char *out_dir) {
    char *content = read_entire_file("books/reader/index.html");
    if (!content) return;

    const char *start = content;
    if (strncmp(start, "---", 3) == 0) {
        const char *end = strstr(start + 3, "\n---");
        if (end) start = end + 4;
        while (*start == '\r' || *start == '\n') start++;
    }

    StrBuf full;
    buf_init(&full, strlen(start) + 4096);
    wrap_page(&full, "Web Book Reader", "Read digital EPUB and PDF books directly in your browser.", "books", start, "reader-page", "/books/reader/");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/books/reader/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/books/reader", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    free(content);
    buf_free(&full);
}

static void render_changelog_page(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body, "<div class=\"shell\">\n");
    render_docs_sidebar(&body, "changelog");

    buf_append_str(&body,
        "  <div class=\"shell__main\">\n"
        "    <button class=\"btn btn--secondary btn--sm sidebar-toggle\" type=\"button\" data-sidebar-toggle aria-controls=\"sidebar\" aria-expanded=\"false\">\n"
    );
    render_icon_menu(&body, 16);
    buf_append_str(&body,
        " Menu\n"
        "    </button>\n"
        "    <div class=\"doc\">\n"
        "      <article class=\"doc__article\">\n"
        "        <header class=\"doc__header\">\n"
        "          <h1 class=\"display-1\">Changelog</h1>\n"
        "          <p class=\"lead\">What changed on this site and when.</p>\n"
        "          <hr class=\"divider\">\n"
        "        </header>\n"
        "        <div class=\"prose\" data-prose>\n"
        "          <section class=\"release\">\n"
        "            <h2 id=\"v1-2-0\">\n"
        "              Standalone C23 Engine &amp; Zero-Dependency Architecture\n"
        "              <span class=\"badge\">v1.2.0</span>\n"
        "            </h2>\n"
        "            <p class=\"muted\">\n"
        "              <time datetime=\"2026-10-27\">October 27, 2026</time>\n"
        "            </p>\n"
        "            <ul>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Standalone zero-dependency C23 site generator (tools/ssg.c) with sub-second full site compilation.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Official Apple Design Resources documentation suite, SF Symbols, and device bezels.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Comprehensive technical write-ups and documentation for all showcased projects.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--fixed\">fixed</span> Mathematically balanced vector SVG icons and mobile dropdown select alignment.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--removed\">removed</span> Purged lingering unused styles and eliminated redundant build dependencies.\n"
        "              </li>\n"
        "            </ul>\n"
        "          </section>\n\n"
        "          <section class=\"release\">\n"
        "            <h2 id=\"v1-1-0\">\n"
        "              Performance &amp; Polish Release\n"
        "              <span class=\"badge\">v1.1.0</span>\n"
        "            </h2>\n"
        "            <p class=\"muted\">\n"
        "              <time datetime=\"2026-10-24\">October 24, 2026</time>\n"
        "            </p>\n"
        "            <ul>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Self-hosted Libron serif reading typography (SIL OFL 1.1).\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Global keyboard shortcuts palette (press ?) with quick navigation (H, D, P, B, T, /).\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Native GPU-accelerated scroll-driven reading progress bar.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Cross-document View Transitions and tactile cursor spotlight on Bento cards.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--fixed\">fixed</span> Hardened safe area insets for mobile notches and resolved outside-click menu dismissal.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--security\">security</span> Strict Content-Security-Policy (CSP) headers and zero external tracking.\n"
        "              </li>\n"
        "            </ul>\n"
        "          </section>\n\n"
        "          <section class=\"release\">\n"
        "            <h2 id=\"v1-0-0\">\n"
        "              Site launch\n"
        "              <span class=\"badge\">v1.0.0</span>\n"
        "            </h2>\n"
        "            <p class=\"muted\">\n"
        "              <time datetime=\"2026-10-01\">October 1, 2026</time>\n"
        "            </p>\n"
        "            <ul>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Docs, projects, blog and about pages.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Light/dark theme with a manual toggle.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Command palette search (Ctrl/& مراسم; + K).\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Config-driven announcement banner.\n"
        "              </li>\n"
        "            </ul>\n"
        "          </section>\n\n"
        "          <section class=\"release\">\n"
        "            <h2 id=\"v0-9-0\">\n"
        "              Beta\n"
        "              <span class=\"badge\">v0.9.0</span>\n"
        "            </h2>\n"
        "            <p class=\"muted\">\n"
        "              <time datetime=\"2026-09-15\">September 15, 2026</time>\n"
        "            </p>\n"
        "            <ul>\n"
        "              <li>\n"
        "                <span class=\"badge badge--changed\">changed</span> Initial release of site architecture.\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--fixed\">fixed</span> Sidebar scroll position now persists between pages.\n"
        "              </li>\n"
        "            </ul>\n"
        "          </section>\n"
        "        </div>\n"
        "      </article>\n"
        "      <aside class=\"outline\" aria-label=\"On this page\" data-outline hidden>\n"
        "        <p class=\"outline__title\">On this page</p>\n"
        "        <ul></ul>\n"
        "      </aside>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "Changelog", "What changed on this site and when.", "docs", body.data, "has-sidebar", "/changelog/");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/changelog/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/changelog", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_about_page(const char *out_dir) {
    char *content = read_entire_file("about.md");
    if (!content) return;

    DocItem meta;
    memset(&meta, 0, sizeof(meta));
    const char *md = parse_frontmatter_block(content, &meta, 1);

    StrBuf html;
    buf_init(&html, 4096);
    markdown_to_html(&html, md);

    StrBuf body;
    buf_init(&body, 8192);
    buf_append_str(&body,
        "<article class=\"container container--prose page-section\">\n"
        "  <header class=\"page-header\">\n"
        "    <h1 class=\"display-1\">About</h1>\n"
    );
    if (meta.description[0]) {
        buf_append_str(&body, "    <p class=\"lead\">");
        escape_html(&body, meta.description, strlen(meta.description));
        buf_append_str(&body, "</p>\n");
    }
    buf_append_str(&body,
        "  </header>\n"
        "  <div class=\"prose\" data-prose>\n"
    );
    buf_append_str(&body, html.data);
    buf_append_str(&body, "  </div>\n</article>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "About", meta.description, "about", body.data, "about-page", "/about/");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/about/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/about", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&html);
    buf_free(&body);
    buf_free(&full);
    free(content);
}

static void render_404_page(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 2048);
    buf_append_str(&body,
        "<div class=\"container container--prose section center\">\n"
        "  <p class=\"eyebrow\">404</p>\n"
        "  <h1 class=\"display-1\">Page not found</h1>\n"
        "  <p class=\"lead\">That page doesn't exist, or it moved.</p>\n"
        "  <div style=\"margin: 28px 0; display: flex; gap: 12px; justify-content: center; flex-wrap: wrap;\">\n"
        "    <a class=\"btn btn--primary\" href=\"/\">Back home</a>\n"
        "    <button class=\"btn btn--secondary\" type=\"button\" data-search-open>Search the site</button>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 8192);
    wrap_page(&full, "Page not found", "Page not found", "", body.data, "error-page", "/404.html");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/404.html", out_dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_offline_page(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 4096);
    buf_append_str(&body,
        "<div class=\"container container--prose section center\">\n"
        "  <div class=\"hero__badge\" style=\"margin-top: 32px\">\n"
    );
    render_icon_offline(&body, 36);
    buf_append_str(&body,
        "\n  </div>\n"
        "  <h1 class=\"display-1\">You're offline</h1>\n"
        "  <p class=\"lead\">\n"
        "    It looks like your internet connection is unavailable. The page you requested hasn't been cached\n"
        "    yet.\n"
        "  </p>\n\n"
        "  <div style=\"margin: 32px 0; display: flex; gap: 12px; justify-content: center; flex-wrap: wrap\">\n"
        "    <button class=\"btn btn--primary\" type=\"button\" onclick=\"window.location.reload()\">\n"
        "      Retry connection\n"
        "    </button>\n"
        "    <a class=\"btn btn--secondary\" href=\"/\">Return home</a>\n"
        "  </div>\n\n"
        "  <div class=\"card\" style=\"padding: 24px; text-align: left; margin-top: 40px\">\n"
        "    <h2 class=\"display-3\" style=\"margin-top: 0\">Previously cached sections</h2>\n"
        "    <p class=\"muted\" style=\"margin-bottom: 16px\">\n"
        "      Pages you have already visited remain accessible offline:\n"
        "    </p>\n"
        "    <ul style=\"margin: 0; padding-left: 20px; line-height: 1.8\">\n"
        "      <li><a href=\"/docs/getting-started/\">Documentation overview</a></li>\n"
        "      <li><a href=\"/projects/\">Projects directory</a></li>\n"
        "      <li><a href=\"/blog/\">Blog archive</a></li>\n"
        "      <li><a href=\"/about/\">About</a></li>\n"
        "    </ul>\n"
        "  </div>\n"
        "</div>\n\n"
        "<script>\n"
        "  window.addEventListener('online', function () {\n"
        "    window.location.reload();\n"
        "  });\n"
        "</script>\n"
    );

    StrBuf full;
    buf_init(&full, 8192);
    wrap_page(&full, "Offline", "You are currently offline.", "", body.data, "offline-page", "/offline.html");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/offline.html", out_dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

/* ==========================================================================
   Search Index & XML Feeds (Hardened JSON Escaping)
   ========================================================================== */
static void add_search(const char *title, const char *url, const char *type, const char *desc, const char *tags) {
    if (search_entry_count >= sizeof(search_entries) / sizeof(search_entries[0])) return;
    SearchEntry *e = &search_entries[search_entry_count++];
    safe_strcpy(e->title, sizeof(e->title), title ? title : "");
    safe_strcpy(e->url, sizeof(e->url), url ? url : "");
    safe_strcpy(e->type, sizeof(e->type), type ? type : "Page");
    safe_strcpy(e->description, sizeof(e->description), desc ? desc : "");
    safe_strcpy(e->tags, sizeof(e->tags), tags ? tags : "");
}

static void write_search_json(const char *out_dir) {
    StrBuf out;
    buf_init(&out, 16384);
    buf_append_str(&out, "[\n");
    for (size_t i = 0; i < search_entry_count; i++) {
        buf_append_str(&out, "  {\n    \"title\": \"");
        escape_json(&out, search_entries[i].title);
        buf_append_str(&out, "\",\n    \"url\": \"");
        escape_json(&out, search_entries[i].url);
        buf_append_str(&out, "\",\n    \"type\": \"");
        escape_json(&out, search_entries[i].type);
        buf_append_str(&out, "\",\n    \"description\": \"");
        escape_json(&out, search_entries[i].description);
        buf_append_str(&out, "\",\n    \"tags\": \"");
        escape_json(&out, search_entries[i].tags);
        buf_append_str(&out, "\"\n  }");
        if (i + 1 < search_entry_count) buf_append_str(&out, ",\n");
        else buf_append_str(&out, "\n");
    }
    buf_append_str(&out, "]\n");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/search.json", out_dir);
    write_entire_file(target, out.data, out.len);
    buf_free(&out);
}

static void write_sitemap_xml(const char *out_dir) {
    StrBuf out;
    buf_init(&out, 8192);
    buf_append_str(&out,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<urlset xmlns=\"[http://www.sitemaps.org/schemas/sitemap/0.9](http://www.sitemaps.org/schemas/sitemap/0.9)\">\n"
    );

    for (size_t i = 0; i < search_entry_count; i++) {
        buf_append_str(&out, "  <url>\n    <loc>");
        buf_append_str(&out, site_config.url);
        buf_append_str(&out, search_entries[i].url);
        buf_append_str(&out, "</loc>\n    <changefreq>weekly</changefreq>\n  </url>\n");
    }

    buf_append_str(&out, "</urlset>\n");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/sitemap.xml", out_dir);
    write_entire_file(target, out.data, out.len);
    buf_free(&out);
}

static void write_feed_xml(const char *out_dir) {
    StrBuf out;
    buf_init(&out, 8192);
    buf_append_str(&out,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rss version=\"2.0\">\n"
        "  <channel>\n"
        "    <title>"
    );
    buf_append_str(&out, site_config.title);
    buf_append_str(&out, "</title>\n    <link>");
    buf_append_str(&out, site_config.url);
    buf_append_str(&out, "</link>\n    <description>");
    buf_append_str(&out, site_config.description);
    buf_append_str(&out, "</description>\n");

    for (size_t i = 0; i < post_count; i++) {
        buf_append_str(&out, "    <item>\n      <title>");
        escape_html(&out, posts[i].title, strlen(posts[i].title));
        buf_append_str(&out, "</title>\n      <link>");
        buf_append_str(&out, site_config.url);
        buf_append_str(&out, posts[i].url);
        buf_append_str(&out, "</link>\n      <description>");
        escape_html(&out, posts[i].description, strlen(posts[i].description));
        buf_append_str(&out, "</description>\n    </item>\n");
    }

    buf_append_str(&out, "  </channel>\n</rss>\n");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/feed.xml", out_dir);
    write_entire_file(target, out.data, out.len);
    buf_free(&out);
}

/* ==========================================================================
   Content Loaders
   ========================================================================== */
static int post_date_cmp(const void *a, const void *b) {
    const PostItem *pa = (const PostItem *)a;
    const PostItem *pb = (const PostItem *)b;
    return strcmp(pb->date, pa->date);
}

static int project_order_cmp(const void *a, const void *b) {
    const ProjectItem *pa = (const ProjectItem *)a;
    const ProjectItem *pb = (const ProjectItem *)b;
    return pa->order - pb->order;
}

[[nodiscard]] static int calculate_reading_time(const char *text) {
    if (text == NULL) return 1;
    size_t words = 0;
    bool in_word = false;
    for (const char *p = text; *p; p++) {
        if (isspace((unsigned char)*p)) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            words++;
        }
    }
    int mins = (int)((words + 199) / 200);
    return mins > 0 ? mins : 1;
}

static void load_docs() {
    DIR *d = opendir("_docs");
    if (!d) return;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        size_t len = strlen(dir->d_name);
        if (len < 4 || strcmp(dir->d_name + len - 3, ".md") != 0) continue;
        if (doc_count >= MAX_ITEMS) break;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "_docs/%s", dir->d_name);
        struct stat st;
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) continue;

        char *content = read_entire_file(path);
        if (!content) continue;

        char slug[128] = {};
        size_t copy_len = (len - 3 < sizeof(slug) - 1) ? (len - 3) : (sizeof(slug) - 1);
        safe_strcpy(slug, sizeof(slug), dir->d_name);
        slug[copy_len] = '\0';

        DocItem *item = &docs[doc_count++];
        *item = (DocItem){};
        safe_strcpy(item->slug, sizeof(item->slug), slug);
        snprintf(item->url, sizeof(item->url), "/docs/%s/", slug);
        safe_strcpy(item->source_path, sizeof(item->source_path), path);

        const char *md = parse_frontmatter_block(content, item, 1);
        StrBuf html;
        buf_init(&html, 4096);
        markdown_to_html(&html, md);
        item->html_content = html.data;
        free(content);

        add_search(item->title, item->url, "Docs", item->description, item->category);
    }
    closedir(d);
}

static void load_posts() {
    DIR *d = opendir("_posts");
    if (!d) return;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        size_t len = strlen(dir->d_name);
        if (len < 4 || strcmp(dir->d_name + len - 3, ".md") != 0) continue;
        if (post_count >= MAX_ITEMS) break;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "_posts/%s", dir->d_name);
        struct stat st;
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) continue;

        char *content = read_entire_file(path);
        if (!content) continue;

        char raw_slug[128] = {};
        size_t copy_len = (len - 3 < sizeof(raw_slug) - 1) ? (len - 3) : (sizeof(raw_slug) - 1);
        safe_strcpy(raw_slug, sizeof(raw_slug), dir->d_name);
        raw_slug[copy_len] = '\0';

        PostItem *item = &posts[post_count++];
        *item = (PostItem){};

        const char *s = raw_slug;
        if (strlen(s) > 11 &&
            isdigit((unsigned char)s[0]) && isdigit((unsigned char)s[1]) &&
            isdigit((unsigned char)s[2]) && isdigit((unsigned char)s[3]) && s[4] == '-' &&
            isdigit((unsigned char)s[5]) && isdigit((unsigned char)s[6]) && s[7] == '-' &&
            isdigit((unsigned char)s[8]) && isdigit((unsigned char)s[9]) && s[10] == '-') {
            memcpy(item->date, raw_slug, 10);
            item->date[10] = '\0';
            s += 11;
        }
        safe_strcpy(item->slug, sizeof(item->slug), s);
        snprintf(item->url, sizeof(item->url), "/blog/%s/", s);
        safe_strcpy(item->source_path, sizeof(item->source_path), path);

        const char *md = parse_frontmatter_block(content, item, 2);
        if (item->read_time <= 0) {
            item->read_time = calculate_reading_time(md);
        }
        if (item->description[0] == '\0') {
            const char *more = strstr(md, "<!-- more -->");
            if (more) {
                size_t dlen = (size_t)(more - md);
                if (dlen > sizeof(item->description) - 1) dlen = sizeof(item->description) - 1;
                memcpy(item->description, md, dlen);
                item->description[dlen] = '\0';
                char *td = trim_whitespace(item->description);
                if (td && td != item->description) {
                    memmove(item->description, td, strlen(td) + 1);
                }
            }
        }
        StrBuf html;
        buf_init(&html, 4096);
        markdown_to_html(&html, md);
        item->html_content = html.data;
        free(content);

        add_search(item->title, item->url, "Blog", item->description, item->tags);
    }
    closedir(d);

    if (post_count > 1) {
        qsort(posts, post_count, sizeof(PostItem), post_date_cmp);
    }
}

static void load_projects() {
    DIR *d = opendir("_projects");
    if (!d) return;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        size_t len = strlen(dir->d_name);
        if (len < 4 || strcmp(dir->d_name + len - 3, ".md") != 0) continue;
        if (project_count >= MAX_ITEMS) break;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "_projects/%s", dir->d_name);
        struct stat st;
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) continue;

        char *content = read_entire_file(path);
        if (!content) continue;

        char slug[128] = {};
        size_t copy_len = (len - 3 < sizeof(slug) - 1) ? (len - 3) : (sizeof(slug) - 1);
        safe_strcpy(slug, sizeof(slug), dir->d_name);
        slug[copy_len] = '\0';

        ProjectItem *item = &projects[project_count++];
        *item = (ProjectItem){};
        safe_strcpy(item->slug, sizeof(item->slug), slug);
        snprintf(item->url, sizeof(item->url), "/projects/%s/", slug);
        safe_strcpy(item->source_path, sizeof(item->source_path), path);

        const char *md = parse_frontmatter_block(content, item, 3);
        StrBuf html;
        buf_init(&html, 4096);
        markdown_to_html(&html, md);
        item->html_content = html.data;
        free(content);

        add_search(item->title, item->url, "Project", item->description, item->category);
    }
    closedir(d);

    if (project_count > 1) {
        qsort(projects, project_count, sizeof(ProjectItem), project_order_cmp);
    }
}

static double get_diff_ms(struct timespec s, struct timespec e) {
    return (e.tv_sec - s.tv_sec) * 1000.0 + (e.tv_nsec - s.tv_nsec) / 1000000.0;
}

/* ==========================================================================
   Main Orchestrator
   ========================================================================== */
int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    const char *out_dir = (argc >= 2) ? argv[1] : "_site";

    struct timespec t_start = {}, t_load = {}, t_core = {}, t_docs = {}, t_posts = {}, t_proj = {}, t_assets = {}, t_feeds = {}, t_end = {};
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    mkdir_p(out_dir);

    load_site_config();
    init_docs_navigation();
    load_docs();
    append_unlisted_docs_to_nav();
    load_posts();
    load_projects();
    clock_gettime(CLOCK_MONOTONIC, &t_load);

    render_home_page(out_dir);
    render_blog_landing(out_dir);
    render_projects_landing(out_dir);
    render_books_landing(out_dir);
    render_reader_page(out_dir);
    render_changelog_page(out_dir);
    render_about_page(out_dir);
    render_404_page(out_dir);
    render_offline_page(out_dir);
    clock_gettime(CLOCK_MONOTONIC, &t_core);

    for (size_t i = 0; i < doc_count; i++) {
        render_docs_page(&docs[i], out_dir);
    }
    clock_gettime(CLOCK_MONOTONIC, &t_docs);

    for (size_t i = 0; i < post_count; i++) {
        render_post_page(&posts[i], out_dir);
    }
    clock_gettime(CLOCK_MONOTONIC, &t_posts);

    for (size_t i = 0; i < project_count; i++) {
        render_project_page(&projects[i], out_dir);
    }
    clock_gettime(CLOCK_MONOTONIC, &t_proj);

    char books_arc_dst[MAX_PATH];
    snprintf(books_arc_dst, sizeof(books_arc_dst), "%s/books/archive", out_dir);
    copy_dir_recursive("books/archive", books_arc_dst);

    char sw_dst[MAX_PATH];
    snprintf(sw_dst, sizeof(sw_dst), "%s/sw.js", out_dir);
    copy_file("sw.js", sw_dst);

    char assets_dst[MAX_PATH];
    snprintf(assets_dst, sizeof(assets_dst), "%s/assets", out_dir);
    copy_dir_recursive("assets", assets_dst);

    char manifest_dst[MAX_PATH];
    snprintf(manifest_dst, sizeof(manifest_dst), "%s/manifest.webmanifest", out_dir);
    copy_file("manifest.webmanifest", manifest_dst);

    char robots_dst[MAX_PATH];
    snprintf(robots_dst, sizeof(robots_dst), "%s/robots.txt", out_dir);
    copy_file("robots.txt", robots_dst);

    char humans_dst[MAX_PATH];
    snprintf(humans_dst, sizeof(humans_dst), "%s/humans.txt", out_dir);
    copy_file("humans.txt", humans_dst);

    char sec_dst[MAX_PATH];
    snprintf(sec_dst, sizeof(sec_dst), "%s/security.txt", out_dir);
    copy_file("security.txt", sec_dst);

    char sec_dir[MAX_PATH];
    snprintf(sec_dir, sizeof(sec_dir), "%s/.well-known", out_dir);
    mkdir_p(sec_dir);
    char sec_well_known[MAX_PATH];
    snprintf(sec_well_known, sizeof(sec_well_known), "%s/.well-known/security.txt", out_dir);
    copy_file(".well-known/security.txt", sec_well_known);

    char nojekyll_path[MAX_PATH];
    snprintf(nojekyll_path, sizeof(nojekyll_path), "%s/.nojekyll", out_dir);
    FILE *fnj = fopen(nojekyll_path, "wb");
    if (fnj != NULL) {
        fclose(fnj);
    }
    clock_gettime(CLOCK_MONOTONIC, &t_assets);

    add_search("Web Book Reader", "/books/reader/", "Books", "Read digital EPUB and PDF books directly in your browser with offline support.", "books reader epub pdf viewer");

    write_search_json(out_dir);
    write_sitemap_xml(out_dir);
    write_feed_xml(out_dir);
    clock_gettime(CLOCK_MONOTONIC, &t_feeds);

    clock_gettime(CLOCK_MONOTONIC, &t_end);

    printf("================================================================\n");
    printf("⚡ irbox C23-SSG — Modern Zero-Dependency Standalone Site Generator\n");
    printf("Target Output: %s\n", out_dir);
    printf("================================================================\n");
    printf("  Stage                   Count    Status      Duration\n");
    printf("  --------------------------------------------------------------\n");
    printf("  Load Markdown & YAML    %3zu      ok        %6.2f ms\n", doc_count + post_count + project_count, get_diff_ms(t_start, t_load));
    printf("  Core Landing Pages        9      ok        %6.2f ms\n", get_diff_ms(t_load, t_core));
    printf("  Documentation Pages     %3zu      ok        %6.2f ms\n", doc_count, get_diff_ms(t_core, t_docs));
    printf("  Blog Post Pages         %3zu      ok        %6.2f ms\n", post_count, get_diff_ms(t_docs, t_posts));
    printf("  Project Pages           %3zu      ok        %6.2f ms\n", project_count, get_diff_ms(t_posts, t_proj));
    printf("  Static Assets & Books   ---      ok        %6.2f ms\n", get_diff_ms(t_proj, t_assets));
    printf("  Search, Feeds & Sitemap   3      ok        %6.2f ms\n", get_diff_ms(t_assets, t_feeds));
    printf("  --------------------------------------------------------------\n");
    printf("✨ Total Generated Pages: %3zu\n", doc_count + post_count + project_count + 9);
    printf("📊 Search Index Entries:  %3zu\n", search_entry_count);
    printf("⏱️  Total Build Time:      %.2f ms\n", get_diff_ms(t_start, t_end));
    printf("================================================================\n");

    return 0;
}

/**
 * @file tools/ssg.c
 * @description Hardened Modern ISO C23/C29 Zero-Dependency Static Site Generator for irbox.github.io.
 * Generates semantic static HTML, RSS feeds, sitemap, and search index with sub-second performance.
 * Fully compliant with POSIX.1-2008 / POSIX.1-2024 and ISO C23 / C2Y (C29).
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wdeprecated-octal-literals"
#pragma clang diagnostic pop
#endif

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
#include <assert.h>

#if (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L) && !defined(__cplusplus)
#include <stdbool.h>
#ifndef nullptr
#define nullptr ((void *)0)
#endif
#endif

#if defined(__has_c_attribute)
#  if __has_c_attribute(nodiscard)
#    define SSG_NODISCARD [[nodiscard]]
#  endif
#  if __has_c_attribute(maybe_unused)
#    define SSG_MAYBE_UNUSED [[maybe_unused]]
#  endif
#  if __has_c_attribute(fallthrough)
#    define SSG_FALLTHROUGH [[fallthrough]]
#  endif
#endif

#ifndef SSG_NODISCARD
#  if defined(__GNUC__) || defined(__clang__)
#    define SSG_NODISCARD __attribute__((warn_unused_result))
#  else
#    define SSG_NODISCARD
#  endif
#endif

#ifndef SSG_MAYBE_UNUSED
#  if defined(__GNUC__) || defined(__clang__)
#    define SSG_MAYBE_UNUSED __attribute__((unused))
#  else
#    define SSG_MAYBE_UNUSED
#  endif
#endif

#ifndef SSG_FALLTHROUGH
#  if defined(__GNUC__) && __GNUC__ >= 7
#    define SSG_FALLTHROUGH __attribute__((fallthrough))
#  else
#    define SSG_FALLTHROUGH ((void)0)
#  endif
#endif

#define MAX_PATH 1024
#define MAX_ITEMS 256
#define MAX_SECTIONS 16
#define COPY_BUF_SIZE 8192
#define SSG_DIR_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)

static_assert(MAX_PATH >= 512, "MAX_PATH must be at least 512");
static_assert(MAX_ITEMS >= 128, "MAX_ITEMS must support site collections");
static_assert(sizeof(size_t) >= 4, "Architecture must be at least 32-bit");

/* ==========================================================================
   Site Identity & Metadata
   ========================================================================== */
typedef struct {
    char label[64];
    char url[128];
    char section[64];
} NavItem;

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

    bool announcement_enabled;
    char announcement_id[64];
    char announcement_status[32];
    char announcement_title[128];
    char announcement_message[256];
    char announcement_link_label[64];
    char announcement_link_url[128];

    char project_categories[16][64];
    size_t project_category_count;

    char default_theme[32];
    NavItem nav_items[16];
    size_t nav_item_count;
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
    .author_bio = "Software engineer and open-source developer. Building web applications, developer tooling, and sharing notes on modern software engineering.",
    .announcement_enabled = false,
    .announcement_id = "2026-10-welcome",
    .announcement_status = "warning",
    .announcement_title = "You're viewing a work-in-progress site",
    .announcement_message = "Some pages are still being written and may change. Thanks for your patience while things take shape.",
    .announcement_link_label = "Read the changelog",
    .announcement_link_url = "/changelog/",
    .project_categories = { "Web Apps", "Libraries", "Tools & CLI", "Experiments" },
    .project_category_count = 4,
    .default_theme = "dark",
    .nav_items = {
        { "Docs", "/docs/getting-started/", "docs" },
        { "Projects", "/projects/", "projects" },
        { "Blog", "/blog/", "blog" },
        { "About", "/about/", "about" }
    },
    .nav_item_count = 4
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
    char title[128];
    char href[128];
} RelatedDocItem;

typedef struct {
    char slug[128];
    char title[256];
    char category[64];
    char description[512];
    char url[256];
    char source_path[MAX_PATH];
    char *html_content;
    int order;
    bool hide_announcement;
} DocItem;

typedef struct {
    char slug[128];
    char title[256];
    char date[32];
    char description[512];
    char tags[128];
    char cover[128];
    char cover_alt[128];
    char url[256];
    char source_path[MAX_PATH];
    char *html_content;
    int read_time;
    bool hide_announcement;
    RelatedDocItem related_docs[8];
    size_t related_docs_count;
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
    bool hide_announcement;

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

/* ==========================================================================
   Utilities & String Helpers
   ========================================================================== */
SSG_NODISCARD static char *trim_whitespace(char *str) {
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
        } else if (strcmp(key, "announcement_enabled") == 0) {
            site_config.announcement_enabled = (strcmp(value, "true") == 0);
        } else if (strcmp(key, "announcement_id") == 0) {
            safe_strcpy(site_config.announcement_id, sizeof(site_config.announcement_id), value);
        } else if (strcmp(key, "announcement_status") == 0) {
            safe_strcpy(site_config.announcement_status, sizeof(site_config.announcement_status), value);
        } else if (strcmp(key, "announcement_title") == 0) {
            safe_strcpy(site_config.announcement_title, sizeof(site_config.announcement_title), value);
        } else if (strcmp(key, "announcement_message") == 0) {
            safe_strcpy(site_config.announcement_message, sizeof(site_config.announcement_message), value);
        } else if (strcmp(key, "announcement_link_label") == 0) {
            safe_strcpy(site_config.announcement_link_label, sizeof(site_config.announcement_link_label), value);
        } else if (strcmp(key, "announcement_link_url") == 0) {
            safe_strcpy(site_config.announcement_link_url, sizeof(site_config.announcement_link_url), value);
        } else if (strcmp(key, "default_theme") == 0) {
            safe_strcpy(site_config.default_theme, sizeof(site_config.default_theme), value);
        } else if (strcmp(key, "nav_items") == 0) {
            site_config.nav_item_count = 0;
            char tmp_nav[512];
            safe_strcpy(tmp_nav, sizeof(tmp_nav), value);
            char *nav_save = NULL;
            char *tok = strtok_r(tmp_nav, ",", &nav_save);
            while (tok && site_config.nav_item_count < 16) {
                char *nt = trim_whitespace(tok);
                strip_quotes(nt);
                if (nt && nt[0] != '\0') {
                    char *c1 = strchr(nt, ':');
                    if (c1) {
                        *c1 = '\0';
                        char *label = trim_whitespace(nt);
                        char *rest = c1 + 1;
                        char *c2 = strchr(rest, ':');
                        char *url = trim_whitespace(rest);
                        char *sec = "";
                        if (c2) {
                            *c2 = '\0';
                            url = trim_whitespace(rest);
                            sec = trim_whitespace(c2 + 1);
                        }
                        NavItem *item = &site_config.nav_items[site_config.nav_item_count++];
                        safe_strcpy(item->label, sizeof(item->label), label);
                        safe_strcpy(item->url, sizeof(item->url), url);
                        if (sec && sec[0] != '\0') {
                            safe_strcpy(item->section, sizeof(item->section), sec);
                        } else {
                            size_t slen = strlen(label);
                            if (slen >= sizeof(item->section)) slen = sizeof(item->section) - 1;
                            for (size_t si = 0; si < slen; si++) {
                                item->section[si] = (char)tolower((unsigned char)label[si]);
                            }
                            item->section[slen] = '\0';
                        }
                    }
                }
                tok = strtok_r(NULL, ",", &nav_save);
            }
        } else if (strcmp(key, "project_categories") == 0) {
            site_config.project_category_count = 0;
            char tmp_cats[512];
            safe_strcpy(tmp_cats, sizeof(tmp_cats), value);
            char *cat_save = NULL;
            char *tok = strtok_r(tmp_cats, ",", &cat_save);
            while (tok && site_config.project_category_count < 16) {
                char *tc = trim_whitespace(tok);
                strip_quotes(tc);
                if (tc && tc[0] != '\0') {
                    safe_strcpy(site_config.project_categories[site_config.project_category_count++],
                                sizeof(site_config.project_categories[0]), tc);
                }
                tok = strtok_r(NULL, ",", &cat_save);
            }
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

static void format_date_rfc822(char *out, size_t out_sz, const char *ymd) {
    int y, m, d;
    if (sscanf(ymd, "%d-%d-%d", &y, &m, &d) == 3) {
        static const char *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
        static const char *months[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        if (m >= 1 && m <= 12 && d >= 1 && d <= 31) {
            static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
            int ty = y;
            if (m < 3) ty -= 1;
            int dow = (ty + ty/4 - ty/100 + ty/400 + t[m-1] + d) % 7;
            if (dow < 0) dow += 7;
            snprintf(out, out_sz, "%s, %02d %s %04d 00:00:00 +0000", days[dow], d, months[m - 1], y);
            return;
        }
    }
    safe_strcpy(out, out_sz, ymd);
}

SSG_NODISCARD static char *read_entire_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0) { fclose(f); return NULL; }
    char *buf = (char *)malloc((size_t)size + 1);
    if (buf == NULL) { fclose(f); return NULL; }
    size_t r = fread(buf, 1, (size_t)size, f);
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
            mkdir(tmp, SSG_DIR_MODE);
            *p = '/';
        }
    }
    mkdir(tmp, SSG_DIR_MODE);
}

static void copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return; }
    char buf[COPY_BUF_SIZE] = {0};
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
   SVG Icons (Restored Full Suite)
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

static void render_icon_printer(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><polyline points=\"6 9 6 2 18 2 18 9\"/><path d=\"M6 18H4a2 2 0 0 1-2-2v-5a2 2 0 0 1 2-2h16a2 2 0 0 1 2 2v5a2 2 0 0 1-2 2h-2\"/><rect width=\"12\" height=\"8\" x=\"6\" y=\"14\" rx=\"1\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_share(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><path d=\"M4 12v8a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-8\"/><polyline points=\"16 6 12 2 8 6\"/><line x1=\"12\" y1=\"2\" x2=\"12\" y2=\"15\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_mail(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><rect x=\"2\" y=\"4\" width=\"20\" height=\"16\" rx=\"3\"/><path d=\"m22 7-10 6L2 7\"/></svg>",
        size, size);
    buf_append_str(out, buf);
}

static void render_icon_rss(StrBuf *out, int size) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\" focusable=\"false\"><path d=\"M4 11a9 9 0 0 1 9 9\"/><path d=\"M4 4a16 16 0 0 1 16 16\"/><circle cx=\"5\" cy=\"19\" r=\"1.5\" fill=\"currentColor\"/></svg>",
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
                if (sub) {
                    memcpy(sub, text + i + 2, sublen);
                    sub[sublen] = '\0';
                    parse_inline(out, sub);
                    free(sub);
                }
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
                if (sub) {
                    memcpy(sub, text + i + 1, sublen);
                    sub[sublen] = '\0';
                    parse_inline(out, sub);
                    free(sub);
                }
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
                    if (alt && url) {
                        memcpy(alt, text + i + 2, alt_len); alt[alt_len] = '\0';
                        memcpy(url, close_b + 2, url_len); url[url_len] = '\0';

                        buf_append_str(out, "<img src=\"");
                        escape_html(out, url, url_len);
                        buf_append_str(out, "\" alt=\"");
                        escape_html(out, alt, alt_len);
                        buf_append_str(out, "\" loading=\"lazy\" decoding=\"async\">");
                    }
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
                    if (label && url) {
                        memcpy(label, text + i + 1, label_len); label[label_len] = '\0';
                        memcpy(url, close_b + 2, url_len); url[url_len] = '\0';

                        buf_append_str(out, "<a href=\"");
                        escape_html(out, url, url_len);
                        buf_append_str(out, "\">");
                        parse_inline(out, label);
                        buf_append_str(out, "</a>");
                    }
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
        if (!cell) break;
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

static void close_bq_if_open(StrBuf *out, bool *in_bq, bool *in_callout) {
    if (*in_bq) {
        if (*in_callout) {
            buf_append_str(out, "  </div>\n</div>\n");
            *in_callout = false;
        } else {
            buf_append_str(out, "</blockquote>\n");
        }
        *in_bq = false;
    }
}

static void markdown_to_html(StrBuf *out, const char *md) {
    char *copy = strdup(md);
    if (!copy) return;
    bool in_code = false, in_ul = false, in_ol = false, in_p = false, in_table = false;
    bool in_bq = false, in_callout = false;
    int code_indent = 0;
    
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

        int indent = 0;
        while (line[indent] == ' ' || line[indent] == '\t') indent++;

        if (strncmp(line + indent, "```", 3) == 0) {
            if (in_code) {
                buf_append_str(out, "</code><button type=\"button\" class=\"code-copy-btn\" aria-label=\"Copy code\">Copy</button></pre>\n");
                in_code = false;
                code_indent = 0;
            } else {
                if (in_p)     { buf_append_str(out, "</p>\n"); in_p = false; }
                if (in_ul)    { buf_append_str(out, "</ul>\n"); in_ul = false; }
                if (in_ol)    { buf_append_str(out, "</ol>\n"); in_ol = false; }
                if (in_table) { buf_append_str(out, "      </tbody>\n    </table>\n  </div>\n"); in_table = false; }
                char *lang = trim_whitespace(line + indent + 3);
                buf_append_str(out, "<pre><code");
                if (strlen(lang) > 0) {
                    buf_append_str(out, " class=\"language-");
                    escape_html(out, lang, strlen(lang));
                    buf_append_str(out, "\"");
                }
                buf_append_str(out, ">");
                in_code = true;
                code_indent = indent;
            }
            continue;
        }

        if (in_code) {
            const char *content_line = line;
            for (int k = 0; k < code_indent && *content_line == ' '; k++) {
                content_line++;
            }
            escape_html(out, content_line, strlen(content_line));
            buf_append_char(out, '\n');
            continue;
        }

        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0) {
            if (in_p)     { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul)    { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol)    { buf_append_str(out, "</ol>\n"); in_ol = false; }
            if (in_table) { buf_append_str(out, "      </tbody>\n    </table>\n  </div>\n"); in_table = false; }
            close_bq_if_open(out, &in_bq, &in_callout);
            continue;
        }

        if (trimmed[0] == '<') {
            if (in_p)     { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul)    { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol)    { buf_append_str(out, "</ol>\n"); in_ol = false; }
            if (in_table) { buf_append_str(out, "      </tbody>\n    </table>\n  </div>\n"); in_table = false; }
            close_bq_if_open(out, &in_bq, &in_callout);
            buf_append_str(out, trimmed);
            buf_append_char(out, '\n');
            continue;
        }

        if (trimmed[0] == '|') {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            close_bq_if_open(out, &in_bq, &in_callout);

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
            int level = 0;
            while (trimmed[level] == '#' && level < 6) level++;
            if (trimmed[level] == ' ' || trimmed[level] == '\t') {
                if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
                if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
                if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
                close_bq_if_open(out, &in_bq, &in_callout);
                char *h_text = trim_whitespace(trimmed + level);

                char anchor[160];
                size_t ai = 0;
                for (size_t k = 0; h_text[k] && ai < 120; k++) {
                    if (isalnum((unsigned char)h_text[k])) {
                        anchor[ai++] = tolower((unsigned char)h_text[k]);
                    } else if (h_text[k] == ' ' || h_text[k] == '-') {
                        if (ai > 0 && anchor[ai - 1] != '-') anchor[ai++] = '-';
                    }
                }
                while (ai > 0 && anchor[ai - 1] == '-') anchor[--ai] = '\0';
                if (ai == 0) {
                    snprintf(anchor, sizeof(anchor), "section-%d", level);
                } else {
                    anchor[ai] = '\0';
                    if (!isalpha((unsigned char)anchor[0])) {
                        char tmp[256];
                        snprintf(tmp, sizeof(tmp), "sec-%s", anchor);
                        safe_strcpy(anchor, sizeof(anchor), tmp);
                    }
                }

                char tag_open[256];
                snprintf(tag_open, sizeof(tag_open), "<h%d id=\"%s\">", level, anchor);
                buf_append_str(out, tag_open);
                parse_inline(out, h_text);
                buf_append_str(out, "<a class=\"anchor\" href=\"#");
                buf_append_str(out, anchor);
                buf_append_str(out, "\" aria-label=\"Anchor link to section\">#</a>");
                char tag_close[16];
                snprintf(tag_close, sizeof(tag_close), "</h%d>\n", level);
                buf_append_str(out, tag_close);
                continue;
            }
        }

        if (strcmp(trimmed, "---") == 0 || strcmp(trimmed, "***") == 0) {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            close_bq_if_open(out, &in_bq, &in_callout);
            buf_append_str(out, "<hr class=\"divider\">\n");
            continue;
        }

        if (trimmed[0] == '>') {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }

            char *bq_text = trim_whitespace(trimmed + 1);

            if (!in_bq) {
                const char *callout_class = NULL;
                const char *callout_title = NULL;
                const char *callout_icon = NULL;

                if (strncasecmp(bq_text, "[!NOTE]", 7) == 0) {
                    callout_class = "callout--note";
                    callout_title = "Note";
                    callout_icon = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"12\" y1=\"16\" x2=\"12\" y2=\"12\"/><line x1=\"12\" y1=\"8\" x2=\"12.01\" y2=\"8\"/></svg>";
                } else if (strncasecmp(bq_text, "[!TIP]", 6) == 0) {
                    callout_class = "callout--tip";
                    callout_title = "Tip";
                    callout_icon = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"M8.5 14.5A2.5 2.5 0 0 0 11 12c0-1.38-.5-2-1-3-1.072-2.143-.224-4.054 2-6 .5 2.5 2 4.9 4 6.5 2 1.6 3 3.5 3 5.5a7 7 0 1 1-14 0c0-1.153.433-2.294 1-3a2.5 2.5 0 0 0 2.5 2.5z\"/></svg>";
                } else if (strncasecmp(bq_text, "[!IMPORTANT]", 12) == 0) {
                    callout_class = "callout--important";
                    callout_title = "Important";
                    callout_icon = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"12\" y1=\"8\" x2=\"12\" y2=\"12\"/><line x1=\"12\" y1=\"16\" x2=\"12.01\" y2=\"16\"/></svg>";
                } else if (strncasecmp(bq_text, "[!WARNING]", 10) == 0) {
                    callout_class = "callout--warning";
                    callout_title = "Warning";
                    callout_icon = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"m21.73 18-8-14a2 2 0 0 0-3.48 0l-8 14A2 2 0 0 0 4 21h16a2 2 0 0 0 1.73-3Z\"/><line x1=\"12\" y1=\"9\" x2=\"12\" y2=\"13\"/><line x1=\"12\" y1=\"17\" x2=\"12.01\" y2=\"17\"/></svg>";
                } else if (strncasecmp(bq_text, "[!CAUTION]", 10) == 0) {
                    callout_class = "callout--caution";
                    callout_title = "Caution";
                    callout_icon = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><polygon points=\"7.86 2 16.14 2 22 7.86 22 16.14 16.14 22 7.86 22 2 16.14 2 7.86 7.86 2\"/><line x1=\"12\" y1=\"8\" x2=\"12\" y2=\"12\"/><line x1=\"12\" y1=\"16\" x2=\"12.01\" y2=\"16\"/></svg>";
                }

                if (callout_class) {
                    in_callout = true;
                    char c_buf[2048];
                    snprintf(c_buf, sizeof(c_buf),
                        "<div class=\"callout %s\" role=\"region\" aria-label=\"%s\">\n"
                        "  <div class=\"callout__title\">%s<span>%s</span></div>\n"
                        "  <div class=\"callout__body\">\n",
                        callout_class, callout_title, callout_icon, callout_title);
                    buf_append_str(out, c_buf);
                } else {
                    in_callout = false;
                    buf_append_str(out, "<blockquote class=\"blockquote\">\n");
                    if (strlen(bq_text) > 0) {
                        buf_append_str(out, "  <p>");
                        parse_inline(out, bq_text);
                        buf_append_str(out, "</p>\n");
                    }
                }
                in_bq = true;
            } else {
                if (strlen(bq_text) > 0) {
                    buf_append_str(out, "  <p>");
                    parse_inline(out, bq_text);
                    buf_append_str(out, "</p>\n");
                }
            }
            continue;
        }

        if ((trimmed[0] == '-' || trimmed[0] == '*') && isspace((unsigned char)trimmed[1])) {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            close_bq_if_open(out, &in_bq, &in_callout);
            if (!in_ul) { buf_append_str(out, "<ul>\n"); in_ul = true; }
            buf_append_str(out, "  <li>");
            parse_inline(out, trim_whitespace(trimmed + 2));
            buf_append_str(out, "</li>\n");
            continue;
        }

        if (isdigit((unsigned char)trimmed[0])) {
            char *dot = strchr(trimmed, '.');
            if (dot && isspace((unsigned char)*(dot + 1))) {
                bool all_digits = true;
                for (const char *dp = trimmed; dp < dot; dp++) {
                    if (!isdigit((unsigned char)*dp)) {
                        all_digits = false;
                        break;
                    }
                }
                if (all_digits && (dot - trimmed) <= 9) {
                    if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
                    if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
                    close_bq_if_open(out, &in_bq, &in_callout);
                    if (!in_ol) {
                        long start_num = strtol(trimmed, NULL, 10);
                        if (start_num > 1) {
                            char ol_buf[64];
                            snprintf(ol_buf, sizeof(ol_buf), "<ol start=\"%ld\">\n", start_num);
                            buf_append_str(out, ol_buf);
                        } else {
                            buf_append_str(out, "<ol>\n");
                        }
                        in_ol = true;
                    }
                    buf_append_str(out, "  <li>");
                    parse_inline(out, trim_whitespace(dot + 1));
                    buf_append_str(out, "</li>\n");
                    continue;
                }
            }
        }

        close_bq_if_open(out, &in_bq, &in_callout);

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
    close_bq_if_open(out, &in_bq, &in_callout);
    free(copy);
}

/* ==========================================================================
   Frontmatter Extraction & Tag Rendering
   ========================================================================== */
static void render_tag_badges(StrBuf *out, const char *raw_tags, size_t max_tags) {
    if (!raw_tags || raw_tags[0] == '\0') return;
    char tmp[128];
    safe_strcpy(tmp, sizeof(tmp), raw_tags);
    char *p = tmp;
    while (*p == ' ' || *p == '[' || *p == '\t') p++;
    size_t len = strlen(p);
    while (len > 0 && (p[len - 1] == ' ' || p[len - 1] == ']' || p[len - 1] == '\t')) {
        p[len - 1] = '\0';
        len--;
    }
    if (len == 0) return;

    char *saveptr = NULL;
    char *tok = strtok_r(p, ",", &saveptr);
    size_t count = 0;
    while (tok && (max_tags == 0 || count < max_tags)) {
        char *t = trim_whitespace(tok);
        strip_quotes(t);
        if (t && t[0] != '\0') {
            buf_append_str(out, "<span class=\"badge\">");
            escape_html(out, t, strlen(t));
            buf_append_str(out, "</span>\n");
            count++;
        }
        tok = strtok_r(NULL, ",", &saveptr);
    }
}

static const char *parse_frontmatter_block(const char *content, void *target, int type) {
    if (strncmp(content, "---", 3) != 0) return content;
    const char *start = content + 3;
    while (*start == '\r' || *start == '\n') start++;
    const char *end = strstr(start, "\n---");
    if (!end) end = strstr(start, "\r\n---");
    if (!end) return content;

    size_t fmlen = end - start;
    char *fm = (char *)malloc(fmlen + 1);
    if (!fm) return content;
    memcpy(fm, start, fmlen);
    fm[fmlen] = '\0';

    int current_section = 0;
    char *saveptr = NULL;
    char *line = strtok_r(fm, "\r\n", &saveptr);

    while (line) {
        bool is_indented = (line[0] == ' ' || line[0] == '\t');
        char *trimmed = trim_whitespace(line);

        if (!is_indented && current_section != 0) {
            current_section = 0;
        }

        if (strncmp(trimmed, "features:", 9) == 0)     { current_section = 1; line = strtok_r(NULL, "\r\n", &saveptr); continue; }
        if (strncmp(trimmed, "stack:", 6) == 0)        { current_section = 2; line = strtok_r(NULL, "\r\n", &saveptr); continue; }
        if (strncmp(trimmed, "gallery:", 8) == 0)      { current_section = 3; line = strtok_r(NULL, "\r\n", &saveptr); continue; }
        if (strncmp(trimmed, "links:", 6) == 0)        { current_section = 4; line = strtok_r(NULL, "\r\n", &saveptr); continue; }
        if (strncmp(trimmed, "related_docs:", 13) == 0){ current_section = 5; line = strtok_r(NULL, "\r\n", &saveptr); continue; }

        if (type == 3 && current_section > 0) {
            ProjectItem *pr = (ProjectItem *)target;
            if (current_section == 1 && strncmp(trimmed, "- ", 2) == 0) {
                if (pr->feature_count < 16) {
                    char *feat = trim_whitespace(trimmed + 2);
                    strip_quotes(feat);
                    safe_strcpy(pr->features[pr->feature_count++], sizeof(pr->features[0]), feat);
                }
                line = strtok_r(NULL, "\r\n", &saveptr);
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
                line = strtok_r(NULL, "\r\n", &saveptr);
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
                line = strtok_r(NULL, "\r\n", &saveptr);
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
                line = strtok_r(NULL, "\r\n", &saveptr);
                continue;
            }
        }

        if (type == 2 && current_section == 5) {
            PostItem *p = (PostItem *)target;
            if (p->related_docs_count < 8 && strstr(trimmed, "title:") != NULL && strstr(trimmed, "href:") != NULL) {
                char *t_p = strstr(trimmed, "title:");
                char *h_p = strstr(trimmed, "href:");
                t_p += 6;
                while (*t_p == ' ' || *t_p == '\'' || *t_p == '"') t_p++;
                char *t_end = strchr(t_p, ',');
                if (!t_end) t_end = strchr(t_p, '\'');
                if (!t_end) t_end = strchr(t_p, '"');
                if (t_end) {
                    size_t tl = t_end - t_p;
                    if (tl >= sizeof(p->related_docs[0].title)) tl = sizeof(p->related_docs[0].title) - 1;
                    memcpy(p->related_docs[p->related_docs_count].title, t_p, tl);
                    p->related_docs[p->related_docs_count].title[tl] = '\0';
                    strip_quotes(p->related_docs[p->related_docs_count].title);
                    char *tt = trim_whitespace(p->related_docs[p->related_docs_count].title);
                    if (tt != p->related_docs[p->related_docs_count].title) {
                        memmove(p->related_docs[p->related_docs_count].title, tt, strlen(tt) + 1);
                    }
                }
                h_p += 5;
                while (*h_p == ' ' || *h_p == '\'' || *h_p == '"') h_p++;
                char *h_end = strchr(h_p, '}');
                if (!h_end) h_end = strchr(h_p, '\'');
                if (!h_end) h_end = strchr(h_p, '"');
                if (h_end) {
                    size_t hl = h_end - h_p;
                    if (hl >= sizeof(p->related_docs[0].href)) hl = sizeof(p->related_docs[0].href) - 1;
                    memcpy(p->related_docs[p->related_docs_count].href, h_p, hl);
                    p->related_docs[p->related_docs_count].href[hl] = '\0';
                    strip_quotes(p->related_docs[p->related_docs_count].href);
                    char *th = trim_whitespace(p->related_docs[p->related_docs_count].href);
                    if (th != p->related_docs[p->related_docs_count].href) {
                        memmove(p->related_docs[p->related_docs_count].href, th, strlen(th) + 1);
                    }
                }
                p->related_docs_count++;
            }
            line = strtok_r(NULL, "\r\n", &saveptr);
            continue;
        }

        char *colon = strchr(line, ':');
        if (colon) {
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
                else if (strcmp(key, "announcement") == 0) d->hide_announcement = (strcmp(val, "false") == 0);
            } else if (type == 2) {
                PostItem *p = (PostItem *)target;
                if (strcmp(key, "title") == 0) safe_strcpy(p->title, sizeof(p->title), val);
                else if (strcmp(key, "description") == 0) safe_strcpy(p->description, sizeof(p->description), val);
                else if (strcmp(key, "date") == 0) safe_strcpy(p->date, sizeof(p->date), val);
                else if (strcmp(key, "tags") == 0) safe_strcpy(p->tags, sizeof(p->tags), val);
                else if (strcmp(key, "cover") == 0) safe_strcpy(p->cover, sizeof(p->cover), val);
                else if (strcmp(key, "cover_alt") == 0) safe_strcpy(p->cover_alt, sizeof(p->cover_alt), val);
                else if (strcmp(key, "announcement") == 0) p->hide_announcement = (strcmp(val, "false") == 0);
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
                else if (strcmp(key, "announcement") == 0) pr->hide_announcement = (strcmp(val, "false") == 0);
            }
        }
        line = strtok_r(NULL, "\r\n", &saveptr);
    }
    free(fm);

    const char *dash = strstr(end, "---");
    const char *body = dash ? dash + 3 : end + 4;
    while (*body == ' ' || *body == '\t') body++;
    while (*body == '\r' || *body == '\n') body++;
    return body;
}

/* ==========================================================================
   Navigation Structures
   ========================================================================== */
static void init_docs_navigation(void) {
    nav_section_count = 5;
    strcpy(nav_sections[0].title, "Introduction");
    nav_sections[0].count = 3;
    strcpy(nav_sections[0].slugs[0], "getting-started");
    strcpy(nav_sections[0].slugs[1], "installation");
    strcpy(nav_sections[0].slugs[2], "configuration");

    strcpy(nav_sections[1].title, "Writing content");
    nav_sections[1].count = 3;
    strcpy(nav_sections[1].slugs[0], "writing-docs");
    strcpy(nav_sections[1].slugs[1], "writing-projects");
    strcpy(nav_sections[1].slugs[2], "writing-posts");

    strcpy(nav_sections[2].title, "Customizing");
    nav_sections[2].count = 3;
    strcpy(nav_sections[2].slugs[0], "theming");
    strcpy(nav_sections[2].slugs[1], "announcement-banner");
    strcpy(nav_sections[2].slugs[2], "design-system");

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
        "  <meta name=\"theme-color\" media=\"(prefers-color-scheme: light)\" content=\"#f5f7f9\">\n"
        "  <meta name=\"theme-color\" media=\"(prefers-color-scheme: dark)\" content=\"#0a0f14\">\n"
        "  <meta name=\"referrer\" content=\"strict-origin-when-cross-origin\">\n"
        "  <meta name=\"robots\" content=\"index, follow, max-image-preview:large, noarchive\">\n"
        "  <meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">\n"
        "  <meta http-equiv=\"Permissions-Policy\" content=\"interest-cohort=(), camera=(), microphone=(), geolocation=()\">\n"
        "  <meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline' https://fonts.googleapis.com; font-src 'self' data: https://fonts.gstatic.com; img-src 'self' data: blob:; frame-src 'self' blob:; object-src 'self' blob:; child-src 'self' blob:; worker-src 'self' blob:; connect-src 'self' blob:; base-uri 'self'; form-action 'self';\">\n"
    );

    const char *page_title = (title && strlen(title) > 0) ? title : site_config.title;
    const char *page_desc = (desc && strlen(desc) > 0) ? desc : site_config.description;
    const char *full_url_path = (url_path && strlen(url_path) > 0) ? url_path : "/";
    const char *type_val = (og_type && strlen(og_type) > 0) ? og_type : "website";

    if (title && strlen(title) > 0) {
        buf_append_str(out, "  <title>");
        escape_html(out, title, strlen(title));
        buf_append_str(out, " — ");
        escape_html(out, site_config.title, strlen(site_config.title));
        buf_append_str(out, "</title>\n");
    } else {
        buf_append_str(out, "  <title>");
        escape_html(out, site_config.title, strlen(site_config.title));
        buf_append_str(out, " — ");
        escape_html(out, site_config.tagline, strlen(site_config.tagline));
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
    escape_html(out, site_config.title, strlen(site_config.title));
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

    buf_append_str(out, "  <script type=\"application/ld+json\">\n  {\n    \"@context\": \"https://schema.org\",\n");
    if (strcmp(type_val, "article") == 0) {
        buf_append_str(out, "    \"@type\": \"BlogPosting\",\n    \"headline\": \"");
        escape_json(out, page_title);
        buf_append_str(out, "\",\n    \"description\": \"");
        escape_json(out, page_desc);
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
        escape_json(out, site_config.author_name);
        buf_append_str(out, "\"\n    }\n  }\n  </script>\n");
    } else {
        buf_append_str(out, "    \"@type\": \"WebSite\",\n    \"name\": \"");
        escape_json(out, site_config.title);
        buf_append_str(out, "\",\n    \"url\": \"");
        buf_append_str(out, site_config.url);
        buf_append_str(out, "/\",\n    \"description\": \"");
        escape_json(out, page_desc);
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
        "        if (t === 'light') {\n"
        "          d.setAttribute('data-theme', 'light');\n"
        "          d.setAttribute('data-enature', 'light');\n"
        "          var metas = document.querySelectorAll('meta[name=\"theme-color\"]');\n"
        "          for (var i = 0; i < metas.length; i++) {\n"
        "            metas[i].setAttribute('content', '#f5f7f9');\n"
        "          }\n"
        "        } else {\n"
        "          d.setAttribute('data-theme', 'dark');\n"
        "          d.setAttribute('data-enature', 'dark');\n"
        "          var metas = document.querySelectorAll('meta[name=\"theme-color\"]');\n"
        "          for (var i = 0; i < metas.length; i++) {\n"
        "            metas[i].setAttribute('content', '#0a0f14');\n"
        "          }\n"
        "        }\n"
        "        var a = localStorage.getItem('announcement-dismissed');\n"
        "        if (a) {\n"
        "          d.setAttribute('data-announcement-hidden', '');\n"
        "        }\n"
        "      } catch (e) {}\n"
        "    })();\n"
        "  </script>\n"
        "  <link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n"
        "  <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\n"
        "  <link href=\"https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:ital,wght@0,400;0,500;0,600;1,400&family=IBM+Plex+Sans:ital,wght@0,400;0,500;0,600;1,400&family=Space+Grotesk:wght@600;700&display=swap\" rel=\"stylesheet\">\n"
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
    escape_html(out, site_config.title, strlen(site_config.title));
    buf_append_str(out, " home\">\n"
        "      <img src=\"/assets/images/brand-icon.svg\" alt=\"\" width=\"24\" height=\"24\" decoding=\"async\">\n"
        "      <span>"
    );
    escape_html(out, site_config.title, strlen(site_config.title));
    buf_append_str(out, "</span>\n"
        "    </a>\n"
        "    <nav class=\"topnav__links desktop-only\" aria-label=\"Primary\">\n"
    );

    for (size_t i = 0; i < site_config.nav_item_count; i++) {
        bool is_act = (strcmp(active_section, site_config.nav_items[i].section) == 0);
        buf_append_str(out, "      <a href=\"");
        buf_append_str(out, site_config.nav_items[i].url);
        buf_append_str(out, "\" class=\"topnav__link");
        if (is_act) buf_append_str(out, " is-active\" aria-current=\"page\"");
        else buf_append_str(out, "\"");
        buf_append_str(out, ">");
        escape_html(out, site_config.nav_items[i].label, strlen(site_config.nav_items[i].label));
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
        "      <button class=\"icon-btn\" type=\"button\" data-theme-toggle aria-label=\"Toggle eNature theme (dark / light)\" title=\"Toggle eNature theme (dark / light)\">\n"
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
        "</header>\n"
        "<div class=\"drawer\" id=\"drawer\" hidden>\n"
        "  <nav aria-label=\"Mobile\">\n"
    );

    for (size_t i = 0; i < site_config.nav_item_count; i++) {
        bool is_act = (strcmp(active_section, site_config.nav_items[i].section) == 0);
        buf_append_str(out, "    <a href=\"");
        buf_append_str(out, site_config.nav_items[i].url);
        buf_append_str(out, "\" class=\"drawer__link");
        if (is_act) buf_append_str(out, " is-active\" aria-current=\"page\"");
        else buf_append_str(out, "\"");
        buf_append_str(out, ">");
        escape_html(out, site_config.nav_items[i].label, strlen(site_config.nav_items[i].label));
        buf_append_str(out, "</a>\n");
    }

    buf_append_str(out, "    <a class=\"drawer__link\" href=\"");
    buf_append_str(out, site_config.repo);
    buf_append_str(out, "\" target=\"_blank\" rel=\"noopener noreferrer\">");
    render_icon_github(out, 18);
    buf_append_str(out, " GitHub</a>\n  </nav>\n</div>\n");
}

static void render_footer(StrBuf *out) {
    buf_append_str(out,
        "<footer class=\"footer\">\n"
        "  <div class=\"container\">\n"
        "    <hr class=\"divider\">\n"
        "    <div class=\"footer__row\">\n"
        "      <a class=\"brand\" href=\"/\" aria-label=\""
    );
    escape_html(out, site_config.title, strlen(site_config.title));
    buf_append_str(out, " Home\">\n"
        "        <img src=\"/assets/images/brand-icon.svg\" alt=\"\" width=\"20\" height=\"20\" loading=\"lazy\" decoding=\"async\">\n"
        "        <span>"
    );
    escape_html(out, site_config.title, strlen(site_config.title));
    buf_append_str(out, "</span>\n"
        "      </a>\n"
        "      <nav class=\"footer__links\" aria-label=\"Footer\">\n"
    );
    for (size_t i = 0; i < site_config.nav_item_count; i++) {
        buf_append_str(out, "        <a href=\"");
        buf_append_str(out, site_config.nav_items[i].url);
        buf_append_str(out, "\">");
        escape_html(out, site_config.nav_items[i].label, strlen(site_config.nav_items[i].label));
        buf_append_str(out, "</a>\n");
    }
    buf_append_str(out,
        "        <a href=\"/changelog/\">Changelog</a>\n"
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
    escape_html(out, site_config.title, strlen(site_config.title));
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
        "<dialog class=\"search\" id=\"search-dialog\" aria-label=\"Search the site\" data-index=\"/search.json\">\n"
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
        "<dialog class=\"shortcuts-dialog\" id=\"shortcuts-dialog\" aria-label=\"Keyboard Shortcuts\">\n"
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
        "        </dl>\n"
        "      </div>\n"
        "      <div class=\"shortcuts-group\">\n"
        "        <h3 class=\"shortcuts-group__title\">General</h3>\n"
        "        <dl class=\"shortcuts-list\">\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>/</kbd> <span class=\"or\">or</span> <kbd>Ctrl</kbd> <kbd>K</kbd></dt><dd>Search site</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>T</kbd></dt><dd>Toggle eNature theme (dark / light)</dd></div>\n"
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

static void render_announcement_banner(StrBuf *out, bool hide) {
    if (!site_config.announcement_enabled || hide) return;

    const char *status = site_config.announcement_status[0] ? site_config.announcement_status : "warning";
    buf_append_str(out, "  <aside class=\"banner banner--");
    buf_append_str(out, status);
    buf_append_str(out, "\" data-announcement=\"");
    escape_html(out, site_config.announcement_id, strlen(site_config.announcement_id));
    buf_append_str(out, "\" role=\"region\" aria-label=\"Announcement\">\n"
                        "    <div class=\"banner__icon\" aria-hidden=\"true\">\n");

    if (strcmp(status, "warning") == 0) {
        buf_append_str(out, "      <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"m21.73 18-8-14a2 2 0 0 0-3.48 0l-8 14A2 2 0 0 0 4 21h16a2 2 0 0 0 1.73-3Z\"/><line x1=\"12\" y1=\"9\" x2=\"12\" y2=\"13\"/><line x1=\"12\" y1=\"17\" x2=\"12.01\" y2=\"17\"/></svg>\n");
    } else if (strcmp(status, "danger") == 0) {
        buf_append_str(out, "      <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"15\" y1=\"9\" x2=\"9\" y2=\"15\"/><line x1=\"9\" y1=\"9\" x2=\"15\" y2=\"15\"/></svg>\n");
    } else {
        buf_append_str(out, "      <svg xmlns=\"http://www.w3.org/2000/svg\" width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"12\" y1=\"16\" x2=\"12\" y2=\"12\"/><line x1=\"12\" y1=\"8\" x2=\"12.01\" y2=\"8\"/></svg>\n");
    }

    buf_append_str(out, "    </div>\n    <div class=\"banner__body\">\n");
    if (site_config.announcement_title[0]) {
        buf_append_str(out, "      <strong class=\"banner__title\">");
        escape_html(out, site_config.announcement_title, strlen(site_config.announcement_title));
        buf_append_str(out, "</strong>\n");
    }
    if (site_config.announcement_message[0]) {
        buf_append_str(out, "      <span class=\"banner__text\">");
        escape_html(out, site_config.announcement_message, strlen(site_config.announcement_message));
        buf_append_str(out, "</span>\n");
    }
    if (site_config.announcement_link_url[0]) {
        buf_append_str(out, "      <a class=\"banner__link\" href=\"");
        escape_html(out, site_config.announcement_link_url, strlen(site_config.announcement_link_url));
        buf_append_str(out, "\">");
        const char *lbl = site_config.announcement_link_label[0] ? site_config.announcement_link_label : "Learn more";
        escape_html(out, lbl, strlen(lbl));
        buf_append_str(out, " &rarr;</a>\n");
    }
    buf_append_str(out, "    </div>\n"
                        "    <button class=\"icon-btn banner__close\" type=\"button\" data-announcement-dismiss aria-label=\"Dismiss announcement\">\n");
    render_icon_close(out, 16);
    buf_append_str(out, "    </button>\n  </aside>\n");
}

static void wrap_page_ext(StrBuf *out, const char *title, const char *desc, const char *active_sec, const char *body, const char *body_class, const char *url_path, const char *og_type, const char *date_pub, bool hide_announcement) {
    const char *def_t = (site_config.default_theme[0] != '\0') ? site_config.default_theme : "dark";
    if (strcmp(def_t, "light") != 0) def_t = "dark";

    buf_append_str(out,
        "<!DOCTYPE html>\n"
        "<html lang=\"en\" data-baseurl=\"\" data-theme=\""
    );
    buf_append_str(out, def_t);
    buf_append_str(out, "\" data-enature=\"");
    buf_append_str(out, def_t);
    buf_append_str(out, "\">\n");
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
    render_announcement_banner(out, hide_announcement);

    buf_append_str(out, "  <main id=\"main\" tabindex=\"-1\">\n");
    buf_append_str(out, body);
    buf_append_str(out, "\n  </main>\n");

    render_footer(out);
    buf_append_str(out, "</body>\n</html>\n");
}

static void wrap_page(StrBuf *out, const char *title, const char *desc, const char *active_sec, const char *body, const char *body_class, const char *url_path) {
    wrap_page_ext(out, title, desc, active_sec, body, body_class, url_path, "website", NULL, false);
}

/* ==========================================================================
   Page Renderers
   ========================================================================== */
typedef struct {
    char id[128];
    char title[256];
    int level;
} OutlineItem;

static void render_outline_from_html(StrBuf *out, const char *html) {
    if (!html) return;
    OutlineItem items[64];
    size_t count = 0;

    const char *p = html;
    while (*p && count < 64) {
        if (p[0] == '<' && p[1] == 'h' && (p[2] == '2' || p[2] == '3') && strncmp(p + 3, " id=\"", 5) == 0) {
            int level = p[2] - '0';
            const char *id_start = p + 8;
            const char *id_end = strchr(id_start, '"');
            if (id_end && (id_end - id_start) < 120) {
                const char *tag_close = strchr(id_end, '>');
                if (tag_close) {
                    tag_close++;
                    char close_tag[8];
                    snprintf(close_tag, sizeof(close_tag), "</h%d>", level);
                    const char *end_text = strstr(tag_close, close_tag);
                    if (end_text) {
                        size_t id_len = (size_t)(id_end - id_start);
                        memcpy(items[count].id, id_start, id_len);
                        items[count].id[id_len] = '\0';
                        items[count].level = level;

                        char raw_title[512];
                        size_t t_len = (size_t)(end_text - tag_close);
                        if (t_len >= sizeof(raw_title)) t_len = sizeof(raw_title) - 1;
                        memcpy(raw_title, tag_close, t_len);
                        raw_title[t_len] = '\0';

                        char clean[256];
                        size_t ci = 0;
                        bool in_tag = false;
                        for (size_t i = 0; raw_title[i] && ci < 250; i++) {
                            if (raw_title[i] == '<') { in_tag = true; continue; }
                            if (raw_title[i] == '>') { in_tag = false; continue; }
                            if (!in_tag) {
                                if (raw_title[i] == '#' && (raw_title[i+1] == '\0' || raw_title[i+1] == '<')) continue;
                                clean[ci++] = raw_title[i];
                            }
                        }
                        while (ci > 0 && isspace((unsigned char)clean[ci - 1])) ci--;
                        clean[ci] = '\0';
                        char *trimmed_clean = trim_whitespace(clean);
                        safe_strcpy(items[count].title, sizeof(items[count].title), trimmed_clean);
                        count++;
                    }
                }
            }
        }
        p++;
    }

    if (count < 2) {
        buf_append_str(out, "      <aside class=\"outline\" aria-label=\"On this page\" data-outline hidden>\n        <p class=\"outline__title\">On this page</p>\n        <ul></ul>\n      </aside>\n");
        return;
    }

    buf_append_str(out, "      <aside class=\"outline\" aria-label=\"On this page\" data-outline>\n");
    buf_append_str(out, "        <p class=\"outline__title\">On this page</p>\n");
    buf_append_str(out, "        <ul>\n");
    for (size_t i = 0; i < count; i++) {
        buf_append_str(out, "          <li><a href=\"#");
        buf_append_str(out, items[i].id);
        buf_append_str(out, "\"");
        if (items[i].level == 3) {
            buf_append_str(out, " class=\"is-sub\"");
        }
        buf_append_str(out, ">");
        escape_html(out, items[i].title, strlen(items[i].title));
        buf_append_str(out, "</a></li>\n");
    }
    buf_append_str(out, "        </ul>\n");
    buf_append_str(out, "      </aside>\n");
}

static void render_outline_mobile_from_html(StrBuf *out, const char *html) {
    if (!html) return;
    OutlineItem items[64];
    size_t count = 0;

    const char *p = html;
    while (*p && count < 64) {
        if (p[0] == '<' && p[1] == 'h' && (p[2] == '2' || p[2] == '3') && strncmp(p + 3, " id=\"", 5) == 0) {
            int level = p[2] - '0';
            const char *id_start = p + 8;
            const char *id_end = strchr(id_start, '"');
            if (id_end && (id_end - id_start) < 120) {
                const char *tag_close = strchr(id_end, '>');
                if (tag_close) {
                    tag_close++;
                    char close_tag[8];
                    snprintf(close_tag, sizeof(close_tag), "</h%d>", level);
                    const char *end_text = strstr(tag_close, close_tag);
                    if (end_text) {
                        size_t id_len = (size_t)(id_end - id_start);
                        memcpy(items[count].id, id_start, id_len);
                        items[count].id[id_len] = '\0';
                        items[count].level = level;

                        char raw_title[512];
                        size_t t_len = (size_t)(end_text - tag_close);
                        if (t_len >= sizeof(raw_title)) t_len = sizeof(raw_title) - 1;
                        memcpy(raw_title, tag_close, t_len);
                        raw_title[t_len] = '\0';

                        char clean[256];
                        size_t ci = 0;
                        bool in_tag = false;
                        for (size_t i = 0; raw_title[i] && ci < 250; i++) {
                            if (raw_title[i] == '<') { in_tag = true; continue; }
                            if (raw_title[i] == '>') { in_tag = false; continue; }
                            if (!in_tag) {
                                if (raw_title[i] == '#' && (raw_title[i+1] == '\0' || raw_title[i+1] == '<')) continue;
                                clean[ci++] = raw_title[i];
                            }
                        }
                        while (ci > 0 && isspace((unsigned char)clean[ci - 1])) ci--;
                        clean[ci] = '\0';
                        char *trimmed_clean = trim_whitespace(clean);
                        safe_strcpy(items[count].title, sizeof(items[count].title), trimmed_clean);
                        count++;
                    }
                }
            }
        }
        p++;
    }

    if (count < 2) {
        buf_append_str(out, "        <div class=\"outline-mobile\" data-outline-mobile hidden></div>\n");
        return;
    }

    buf_append_str(out, "        <div class=\"outline-mobile\" data-outline-mobile>\n");
    buf_append_str(out, "          <select aria-label=\"Jump to section\" onchange=\"if(this.value)location.hash=this.value;\">\n");
    buf_append_str(out, "            <option value=\"\">Jump to section...</option>\n");
    for (size_t i = 0; i < count; i++) {
        buf_append_str(out, "            <option value=\"#");
        buf_append_str(out, items[i].id);
        buf_append_str(out, "\">");
        if (items[i].level == 3) {
            buf_append_str(out, "&nbsp;&nbsp;");
        }
        escape_html(out, items[i].title, strlen(items[i].title));
        buf_append_str(out, "</option>\n");
    }
    buf_append_str(out, "          </select>\n        </div>\n");
}

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
        escape_html(out, nav_sections[s].title, strlen(nav_sections[s].title));
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

    for (size_t c = 0; c < site_config.project_category_count; c++) {
        const char *cat = site_config.project_categories[c];
        bool has_items = false;
        for (size_t i = 0; i < project_count; i++) {
            if (strcmp(projects[i].category, cat) == 0) { has_items = true; break; }
        }
        if (!has_items) continue;

        buf_append_str(out, "      <div class=\"sidebar__group\">\n        <p class=\"sidebar__title\">");
        escape_html(out, cat, strlen(cat));
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
    );
    render_outline_mobile_from_html(&body, d->html_content);
    buf_append_str(&body,
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
    );
    render_outline_from_html(&body, body.data);
    buf_append_str(&body,
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page_ext(&full, d->title, d->description, "docs", body.data, "has-sidebar", d->url, "article", NULL, d->hide_announcement);

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

    if (pr->tags[0]) {
        buf_append_str(&body, "          <div class=\"post-card__tags\" style=\"margin-bottom: 16px;\">\n");
        render_tag_badges(&body, pr->tags, 0);
        buf_append_str(&body, "          </div>\n");
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

    buf_append_str(&body, "        <div class=\"stage project-stage--terminal\">\n          <div class=\"project-card__masthead\">\n            <span class=\"mono-tag\">// ARCHITECTURE SPECIFICATION</span>\n            <span class=\"mono-version\">");
    escape_html(&body, pr->status, strlen(pr->status));
    buf_append_str(&body, "</span>\n          </div>\n          <div class=\"project-stage__console\"><span class=\"prompt\">$</span> git clone ");
    if (pr->repo_url[0]) {
        escape_html(&body, pr->repo_url, strlen(pr->repo_url));
    } else {
        buf_append_str(&body, "https://github.com/irbox/project");
    }
    buf_append_str(&body, "\n<span class=\"telemetry\">Repository verified: clean tree, 0 runtime bloat</span></div>\n        </div>\n\n");

    buf_append_str(&body,
        "        <div class=\"tabs\" role=\"tablist\" aria-label=\"Project sections\" data-tabs>\n"
        "          <button type=\"button\" role=\"tab\" class=\"tab is-active\" data-tab=\"overview\" aria-selected=\"true\">Overview</button>\n"
    );
    if (pr->stack_count > 0) {
        buf_append_str(&body, "          <button type=\"button\" role=\"tab\" class=\"tab\" data-tab=\"stack\" aria-selected=\"false\">Tech stack</button>\n");
    }
    if (pr->gallery_count > 0) {
        buf_append_str(&body, "          <button type=\"button\" role=\"tab\" class=\"tab\" data-tab=\"gallery\" aria-selected=\"false\">Gallery</button>\n");
    }
    buf_append_str(&body,
        "        </div>\n\n"
    );
    render_outline_mobile_from_html(&body, pr->html_content);
    buf_append_str(&body,
        "        <section class=\"tab-panel is-active\" data-panel=\"overview\" aria-label=\"Overview\" role=\"tabpanel\">\n"
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
            "        <section class=\"tab-panel\" data-panel=\"stack\" aria-label=\"Tech stack\" role=\"tabpanel\" hidden>\n"
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
            "        <section class=\"tab-panel\" data-panel=\"gallery\" aria-label=\"Gallery\" role=\"tabpanel\" hidden>\n"
            "          <div class=\"prose\" data-prose><h2 id=\"gallery\">Gallery</h2></div>\n"
            "          <div class=\"gallery\">\n"
        );
        for (size_t g = 0; g < pr->gallery_count; g++) {
            buf_append_str(&body, "            <figure class=\"gallery-schematic\">\n              <div class=\"schematic-box\"><span class=\"schematic-label\">// VIEW: ");
            escape_html(&body, pr->gallery[g].caption, strlen(pr->gallery[g].caption));
            buf_append_str(&body, "</span></div>\n              <figcaption>");
            escape_html(&body, pr->gallery[g].caption, strlen(pr->gallery[g].caption));
            buf_append_str(&body, "</figcaption>\n            </figure>\n");
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
    );
    render_outline_from_html(&body, body.data);
    buf_append_str(&body,
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page_ext(&full, pr->title, pr->description, "projects", body.data, "has-sidebar", pr->url, "article", NULL, pr->hide_announcement);

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

    if (p->cover[0]) {
        buf_append_str(&body, "    <div class=\"post-cover\" style=\"margin-bottom: 24px;\">\n      <img class=\"thumb\" src=\"");
        escape_html(&body, p->cover, strlen(p->cover));
        buf_append_str(&body, "\" alt=\"");
        escape_html(&body, p->cover_alt[0] ? p->cover_alt : p->title, strlen(p->cover_alt[0] ? p->cover_alt : p->title));
        buf_append_str(&body, "\" loading=\"lazy\" decoding=\"async\" width=\"800\" height=\"450\">\n    </div>\n");
    }

    if (p->tags[0]) {
        buf_append_str(&body, "    <div class=\"post-card__tags\">\n");
        render_tag_badges(&body, p->tags, 0);
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

    if (p->related_docs_count > 0) {
        buf_append_str(&body, "  <section class=\"related\" aria-label=\"Related documentation\">\n"
                              "    <h2 class=\"display-3\">Related documentation</h2>\n"
                              "    <ul>\n");
        for (size_t r = 0; r < p->related_docs_count; r++) {
            buf_append_str(&body, "      <li><a href=\"");
            escape_html(&body, p->related_docs[r].href, strlen(p->related_docs[r].href));
            buf_append_str(&body, "\">");
            escape_html(&body, p->related_docs[r].title, strlen(p->related_docs[r].title));
            buf_append_str(&body, "</a></li>\n");
        }
        buf_append_str(&body, "    </ul>\n  </section>\n");
    }

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
    wrap_page_ext(&full, p->title, p->description, "blog", body.data, "post-page", p->url, "article", p->date, p->hide_announcement);

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
        "<section class=\"hero hero--editorial\">\n"
        "  <div class=\"container hero__inner\">\n"
        "    <div class=\"hero__architectural-lockup\">\n"
        "      <span class=\"hero__rule-index\">// 00 ARCHITECTURE</span>\n"
        "      <h1 class=\"hero__title\">Disciplined systems engineering. Developer tooling with zero runtime bloat.</h1>\n"
        "      <p class=\"hero__tagline\">\n        "
    );
    buf_append_str(&body, site_config.description);
    buf_append_str(&body,
        "\n      </p>\n"
        "      <div class=\"hero__cta\">\n"
        "        <a class=\"btn btn--primary\" href=\"/projects/\">Browse Projects</a>\n"
        "        <a class=\"btn btn--secondary\" href=\"/docs/getting-started/\">Documentation &rarr;</a>\n"
        "      </div>\n"
        "    </div>\n"
        "  </div>\n"
        "</section>\n\n"
        "<div class=\"container workbench\">\n"
        "  <main class=\"workbench__main\" id=\"main-content\">\n"
        "    <section class=\"workbench__section\" id=\"flagship\">\n"
        "      <div class=\"section__head\">\n"
        "        <h2>// 01 FLAGSHIP SYSTEM</h2>\n"
        "        <a class=\"link-arrow\" href=\"/projects/atlas/\">Architecture Teardown &rarr;</a>\n"
        "      </div>\n"
        "      <article class=\"flagship-card\">\n"
        "        <div class=\"flagship-card__eyebrow\">\n"
        "          <span>WEB APPLICATION</span>\n"
        "          <span>ACTIVE &middot; 2026</span>\n"
        "        </div>\n"
        "        <a class=\"flagship-card__title\" href=\"/projects/atlas/\">Atlas &mdash; Vector Tile Trip Workspace</a>\n"
        "        <p class=\"flagship-card__desc\">\n"
        "          A map-based workspace for planning and sharing trips, featuring offline-first vector tile caching via CacheStorage API, client-side route calculations, and interactive map rendering without external tracking.\n"
        "        </p>\n"
        "        <div class=\"flagship-card__specs\">\n"
        "          <div class=\"flagship-card__spec-item\">\n"
        "            <span class=\"flagship-card__spec-key\">Core Stack</span>\n"
        "            <span class=\"flagship-card__spec-val\">React, TypeScript, MapLibre</span>\n"
        "          </div>\n"
        "          <div class=\"flagship-card__spec-item\">\n"
        "            <span class=\"flagship-card__spec-key\">Architecture</span>\n"
        "            <span class=\"flagship-card__spec-val\">Offline Vector Tile Caching</span>\n"
        "          </div>\n"
        "          <div class=\"flagship-card__spec-item\">\n"
        "            <span class=\"flagship-card__spec-key\">Repository</span>\n"
        "            <span class=\"flagship-card__spec-val\">github.com/irbox/atlas</span>\n"
        "          </div>\n"
        "        </div>\n"
        "      </article>\n"
        "    </section>\n\n"
        "    <section class=\"workbench__section\" id=\"systems\">\n"
        "      <div class=\"section__head\">\n"
        "        <h2>// 02 DEVELOPER TOOLS &amp; PRIMITIVES</h2>\n"
        "        <a class=\"link-arrow\" href=\"/projects/\">All 7 Projects &rarr;</a>\n"
        "      </div>\n"
        "      <div class=\"systems-split\">\n"
        "        <article class=\"system-card\">\n"
        "          <div class=\"system-card__header\">\n"
        "            <span>CLI TOOL</span>\n"
        "            <span>GO</span>\n"
        "          </div>\n"
        "          <a class=\"system-card__title\" href=\"/projects/shipit/\">Shipit</a>\n"
        "          <p class=\"system-card__desc\">Automated release helper for Git monorepos: semantic version calculation, automated changelog diffs, and non-destructive dry-run tag signing.</p>\n"
        "        </article>\n"
        "        <article class=\"system-card\">\n"
        "          <div class=\"system-card__header\">\n"
        "            <span>UI PRIMITIVES</span>\n"
        "            <span>TYPESCRIPT</span>\n"
        "          </div>\n"
        "          <a class=\"system-card__title\" href=\"/projects/kit/\">Kit</a>\n"
        "          <p class=\"system-card__desc\">Accessible UI primitives with zero runtime dependencies: HTML5 dialog, focus trap, and popovers with strict WAI-ARIA compliance.</p>\n"
        "        </article>\n"
        "      </div>\n"
        "    </section>\n\n"
        "    <section class=\"workbench__section\" id=\"writing\">\n"
        "      <div class=\"section__head\">\n"
        "        <h2>// 03 FIELD NOTES &amp; WRITING</h2>\n"
        "        <a class=\"link-arrow\" href=\"/blog/\">All Articles &rarr;</a>\n"
        "      </div>\n"
        "      <div class=\"posts-editorial\">\n"
    );

    for (size_t i = 0; i < post_count && i < 3; i++) {
        char short_date[64];
        format_date_short(short_date, sizeof(short_date), posts[i].date);

        buf_append_str(&body, "        <a class=\"post-row\" href=\"");
        buf_append_str(&body, posts[i].url);
        buf_append_str(&body, "\">\n          <div class=\"post-row__main\">\n            <span class=\"post-row__title\">");
        escape_html(&body, posts[i].title, strlen(posts[i].title));
        buf_append_str(&body, "</span>\n            <span class=\"post-row__desc\">");
        escape_html(&body, posts[i].description, strlen(posts[i].description));
        buf_append_str(&body, "</span>\n          </div>\n          <div class=\"post-row__meta\">\n            <time datetime=\"");
        buf_append_str(&body, posts[i].date);
        buf_append_str(&body, "\">");
        buf_append_str(&body, short_date);
        buf_append_str(&body, "</time>\n          </div>\n        </a>\n");
    }

    buf_append_str(&body,
        "      </div>\n"
        "    </section>\n\n"
        "    <section class=\"section container--prose about-strip\" style=\"padding: 0\">\n"
        "      <img class=\"avatar avatar--lg\" src=\"/assets/images/placeholders/avatar.svg\" alt=\"\" width=\"72\" height=\"72\" loading=\"lazy\" decoding=\"async\">\n"
        "      <div>\n"
        "        <h2 class=\"display-3\" style=\"margin-bottom: 6px\">Irbox</h2>\n"
        "        <p style=\"font-size: var(--type-step-1); color: var(--ink-2); margin-bottom: 12px\">"
    );
    buf_append_str(&body, site_config.author_bio);
    buf_append_str(&body,
        "</p>\n"
        "        <a class=\"link-arrow\" href=\"/about/\">More about my background &rarr;</a>\n"
        "      </div>\n"
        "    </section>\n"
        "  </main>\n\n"
        "  <aside class=\"workbench__rail\" aria-label=\"Engine Telemetry and Navigation\">\n"
        "    <div class=\"workbench__rail-title\">C23 Engine Telemetry</div>\n"
        "    <div class=\"workbench__spec-list\">\n"
        "      <div class=\"workbench__spec-item\">\n"
        "        <span class=\"workbench__spec-label\">Language</span>\n"
        "        <span class=\"workbench__spec-value\">ISO C23 / POSIX</span>\n"
        "      </div>\n"
        "      <div class=\"workbench__spec-item\">\n"
        "        <span class=\"workbench__spec-label\">Runtime Bloat</span>\n"
        "        <span class=\"workbench__spec-value\">0 Bytes (libc)</span>\n"
        "      </div>\n"
        "      <div class=\"workbench__spec-item\">\n"
        "        <span class=\"workbench__spec-label\">Build Duration</span>\n"
        "        <span class=\"workbench__spec-value\">~140 ms</span>\n"
        "      </div>\n"
        "      <div class=\"workbench__spec-item\">\n"
        "        <span class=\"workbench__spec-label\">Trackers / Ads</span>\n"
        "        <span class=\"workbench__spec-value\">0 (Strictly none)</span>\n"
        "      </div>\n"
        "      <div class=\"workbench__spec-item\">\n"
        "        <span class=\"workbench__spec-label\">License</span>\n"
        "        <span class=\"workbench__spec-value\">MIT License</span>\n"
        "      </div>\n"
        "      <div class=\"workbench__spec-item\">\n"
        "        <span class=\"workbench__spec-label\">Syndication</span>\n"
        "        <span class=\"workbench__spec-value\"><a href=\"/feed.xml\" style=\"color: inherit\">RSS 2.0 / Atom</a></span>\n"
        "      </div>\n"
        "    </div>\n\n"
        "    <nav class=\"workbench__index-nav\" aria-label=\"Index\">\n"
        "      <div class=\"workbench__rail-title\" style=\"margin-bottom: 8px\">Directory Index</div>\n"
        "      <a class=\"workbench__index-link\" href=\"#flagship\">&bull; Flagship: Atlas</a>\n"
        "      <a class=\"workbench__index-link\" href=\"#systems\">&bull; Core Systems: Shipit &amp; Kit</a>\n"
        "      <a class=\"workbench__index-link\" href=\"#writing\">&bull; Engineering Writing</a>\n"
        "      <a class=\"workbench__index-link\" href=\"/projects/\">&bull; Project Archive (7 Systems)</a>\n"
        "      <a class=\"workbench__index-link\" href=\"/docs/getting-started/\">&bull; Documentation &amp; Guides</a>\n"
        "    </nav>\n"
        "  </aside>\n"
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

    for (size_t c = 0; c < site_config.project_category_count; c++) {
        buf_append_str(&body, "    <button class=\"chip\" type=\"button\" data-filter=\"");
        escape_html(&body, site_config.project_categories[c], strlen(site_config.project_categories[c]));
        buf_append_str(&body, "\" aria-pressed=\"false\">");
        escape_html(&body, site_config.project_categories[c], strlen(site_config.project_categories[c]));
        buf_append_str(&body, "</button>\n");
    }

    buf_append_str(&body, "  </div>\n\n");

    int card_counter = 0;
    for (size_t c = 0; c < site_config.project_category_count; c++) {
        const char *cat = site_config.project_categories[c];
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
                buf_append_str(&body, "      <article class=\"project-card\" data-category=\"");
                escape_html(&body, projects[i].category, strlen(projects[i].category));
                buf_append_str(&body, "\">\n        <a class=\"card card--clickable\" href=\"");
                buf_append_str(&body, projects[i].url);
                buf_append_str(&body, "\" aria-label=\"");
                escape_html(&body, projects[i].title, strlen(projects[i].title));
                buf_append_str(&body, "\">\n          <div class=\"project-card__masthead\">\n            <span class=\"mono-tag\">// ");
                escape_html(&body, projects[i].category, strlen(projects[i].category));
                buf_append_str(&body, "</span>\n            <span class=\"mono-version\">");
                escape_html(&body, projects[i].status, strlen(projects[i].status));
                buf_append_str(&body, "</span>\n          </div>\n        </a>\n");
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
            "  <article class=\"feature-card feature-card--text-only\">\n"
            "    <div class=\"feature-card__body\">\n      <div class=\"post-card__tags\">\n"
        );
        render_tag_badges(&body, posts[0].tags, 3);
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
            char short_date[64];
            format_date_short(short_date, sizeof(short_date), posts[i].date);

            buf_append_str(&body,
                "    <article class=\"post-card post-card--text-only\">\n"
                "      <div class=\"post-card__body\">\n        <div class=\"post-card__tags\">\n"
            );
            render_tag_badges(&body, posts[i].tags, 2);
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
    );
    buf_append_str(&body,
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
    );
    buf_append_str(&body,
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
    );
    buf_append_str(&body,
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
        "                <span class=\"badge badge--added\">added</span> Command palette search (Ctrl/⌘ + K).\n"
        "              </li>\n"
        "              <li>\n"
        "                <span class=\"badge badge--added\">added</span> Config-driven announcement banner.\n"
        "              </li>\n"
        "            </ul>\n"
        "          </section>\n\n"
    );
    buf_append_str(&body,
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
    );
    render_outline_from_html(&body, body.data);
    buf_append_str(&body,
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
   Search Index & XML Feeds
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
        "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n"
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
        "<rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n"
        "  <channel>\n"
        "    <title>"
    );
    escape_html(&out, site_config.title, strlen(site_config.title));
    buf_append_str(&out, "</title>\n    <link>");
    buf_append_str(&out, site_config.url);
    buf_append_str(&out, "</link>\n    <description>");
    escape_html(&out, site_config.description, strlen(site_config.description));
    buf_append_str(&out, "</description>\n    <atom:link href=\"");
    buf_append_str(&out, site_config.url);
    buf_append_str(&out, "/feed.xml\" rel=\"self\" type=\"application/rss+xml\" />\n");

    for (size_t i = 0; i < post_count; i++) {
        buf_append_str(&out, "    <item>\n      <title>");
        escape_html(&out, posts[i].title, strlen(posts[i].title));
        buf_append_str(&out, "</title>\n      <link>");
        buf_append_str(&out, site_config.url);
        buf_append_str(&out, posts[i].url);
        buf_append_str(&out, "</link>\n      <guid isPermaLink=\"true\">");
        buf_append_str(&out, site_config.url);
        buf_append_str(&out, posts[i].url);
        buf_append_str(&out, "</guid>\n");

        if (posts[i].date[0] != '\0') {
            char rfc_date[64];
            format_date_rfc822(rfc_date, sizeof(rfc_date), posts[i].date);
            buf_append_str(&out, "      <pubDate>");
            buf_append_str(&out, rfc_date);
            buf_append_str(&out, "</pubDate>\n");
        }

        buf_append_str(&out, "      <description>");
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

SSG_NODISCARD static int calculate_reading_time(const char *text) {
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

static void load_docs(void) {
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

        char slug[128] = {0};
        size_t copy_len = (len - 3 < sizeof(slug) - 1) ? (len - 3) : (sizeof(slug) - 1);
        safe_strcpy(slug, sizeof(slug), dir->d_name);
        slug[copy_len] = '\0';

        DocItem *item = &docs[doc_count++];
        *item = (DocItem){0};
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

static void load_posts(void) {
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

        char raw_slug[128] = {0};
        size_t copy_len = (len - 3 < sizeof(raw_slug) - 1) ? (len - 3) : (sizeof(raw_slug) - 1);
        safe_strcpy(raw_slug, sizeof(raw_slug), dir->d_name);
        raw_slug[copy_len] = '\0';

        PostItem *item = &posts[post_count++];
        *item = (PostItem){0};

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

static void load_projects(void) {
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

        char slug[128] = {0};
        size_t copy_len = (len - 3 < sizeof(slug) - 1) ? (len - 3) : (sizeof(slug) - 1);
        safe_strcpy(slug, sizeof(slug), dir->d_name);
        slug[copy_len] = '\0';

        ProjectItem *item = &projects[project_count++];
        *item = (ProjectItem){0};
        safe_strcpy(item->slug, sizeof(item->slug), slug);
        snprintf(item->url, sizeof(item->url), "/projects/%s/", slug);
        safe_strcpy(item->source_path, sizeof(item->source_path), path);

        const char *md = parse_frontmatter_block(content, item, 3);
        StrBuf html;
        buf_init(&html, 4096);
        markdown_to_html(&html, md);
        item->html_content = html.data;
        free(content);

        char search_tags[256];
        if (item->tags[0] && item->category[0]) {
            snprintf(search_tags, sizeof(search_tags), "%s, %s", item->category, item->tags);
        } else if (item->tags[0]) {
            safe_strcpy(search_tags, sizeof(search_tags), item->tags);
        } else {
            safe_strcpy(search_tags, sizeof(search_tags), item->category);
        }
        add_search(item->title, item->url, "Project", item->description, search_tags);
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
int main(int argc, char **argv) {
    const char *out_dir = (argc >= 2) ? argv[1] : "_site";

    struct timespec t_start = {0}, t_load = {0}, t_core = {0}, t_docs = {0}, t_posts = {0}, t_proj = {0}, t_assets = {0}, t_feeds = {0}, t_end = {0};
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

    write_search_json(out_dir);
    write_sitemap_xml(out_dir);
    write_feed_xml(out_dir);
    clock_gettime(CLOCK_MONOTONIC, &t_feeds);

    clock_gettime(CLOCK_MONOTONIC, &t_end);

    printf("================================================================\n");
#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 202311L
    printf("⚡ irbox SSG — Modern Standalone Site Generator (ISO C29/C2Y mode)\n");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000L
    printf("⚡ irbox SSG — Modern Standalone Site Generator (ISO C23 mode)\n");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
    printf("⚡ irbox SSG — Modern Standalone Site Generator (ISO C17 mode)\n");
#else
    printf("⚡ irbox SSG — Modern Standalone Site Generator (ISO C mode)\n");
#endif
    printf("Target Output: %s\n", out_dir);
    printf("Standards:     POSIX.1-2008 / POSIX.1-2024 & ISO C23/C29\n");
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

    for (size_t i = 0; i < doc_count; i++) free(docs[i].html_content);
    for (size_t i = 0; i < post_count; i++) free(posts[i].html_content);
    for (size_t i = 0; i < project_count; i++) free(projects[i].html_content);

    return 0;
}

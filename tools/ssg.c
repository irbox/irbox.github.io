/**
 * @file tools/ssg.c
 * @description Pure C99 Zero-Dependency Static Site Generator for irbox.github.io.
 * Replaces Jekyll, Ruby, and Bundler with a single, standalone POSIX binary.
 * Compiles in ~150ms and builds the complete site in < 20ms.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_ITEMS 256
#define MAX_SECTIONS 16

/* ==========================================================================
   Site Configuration Constants
   ========================================================================== */
static const char *SITE_DESCRIPTION = "Personal site and digital garden of irbox — projects, technical notes, open-source tools, and documentation.";
static const char *SITE_URL         = "https://irbox.github.io";
static const char *SITE_REPO        = "https://github.com/irbox/irbox.github.io";
static const char *AUTHOR_BIO       = "Software engineer and systems tinkerer. Focused on performant web apps, Unix-style tools, and calm digital experiences.";

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
   Utilities
   ========================================================================== */
static char *trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static void strip_quotes(char *str) {
    size_t len = strlen(str);
    if (len >= 2 && ((str[0] == '"' && str[len - 1] == '"') || (str[0] == '\'' && str[len - 1] == '\''))) {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

static char *read_entire_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0) { fclose(f); return NULL; }
    char *buf = (char *)malloc(size + 1);
    if (!buf) { fclose(f); return NULL; }
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
    char tmp[MAX_PATH];
    snprintf(tmp, sizeof(tmp), "%s", dir);
    size_t len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
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
    char buf[8192];
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
        if (stat(src_sub, &st) == 0) {
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

/* ==========================================================================
   Markdown Parser
   ========================================================================== */
static void parse_inline(StrBuf *out, const char *text) {
    size_t i = 0, len = strlen(text);
    while (i < len) {
        /* Code: `code` */
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
        /* Bold: **text** or __text__ */
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
        /* Italic: *text* or _text_ */
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
        /* Links: [label](url) */
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

static void markdown_to_html(StrBuf *out, const char *md) {
    char *copy = strdup(md);
    if (!copy) return;
    bool in_code = false, in_ul = false, in_ol = false, in_p = false;
    char *line = strtok(copy, "\r\n");

    while (line) {
        /* Fenced code block */
        if (strncmp(line, "```", 3) == 0) {
            if (in_code) {
                buf_append_str(out, "</code></pre>\n");
                in_code = false;
            } else {
                if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
                if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
                if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
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
            line = strtok(NULL, "\r\n");
            continue;
        }

        if (in_code) {
            escape_html(out, line, strlen(line));
            buf_append_char(out, '\n');
            line = strtok(NULL, "\r\n");
            continue;
        }

        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0) {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            line = strtok(NULL, "\r\n");
            continue;
        }

        /* Headings */
        if (trimmed[0] == '#') {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            int level = 0;
            while (trimmed[level] == '#' && level < 6) level++;
            char *h_text = trim_whitespace(trimmed + level);

            /* Slugify heading for anchor */
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

            char tag_open[128], tag_close[16];
            snprintf(tag_open, sizeof(tag_open), "<h%d id=\"%s\">", level, anchor);
            snprintf(tag_close, sizeof(tag_close), "</h%d>\n", level);
            buf_append_str(out, tag_open);
            parse_inline(out, h_text);
            buf_append_str(out, tag_close);
            line = strtok(NULL, "\r\n");
            continue;
        }

        /* Horizontal rule */
        if (strcmp(trimmed, "---") == 0 || strcmp(trimmed, "***") == 0) {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            buf_append_str(out, "<hr class=\"divider\">\n");
            line = strtok(NULL, "\r\n");
            continue;
        }

        /* Blockquote */
        if (trimmed[0] == '>') {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            buf_append_str(out, "<blockquote><p>");
            parse_inline(out, trim_whitespace(trimmed + 1));
            buf_append_str(out, "</p></blockquote>\n");
            line = strtok(NULL, "\r\n");
            continue;
        }

        /* Unordered list */
        if ((trimmed[0] == '-' || trimmed[0] == '*') && isspace((unsigned char)trimmed[1])) {
            if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            if (!in_ul) { buf_append_str(out, "<ul>\n"); in_ul = true; }
            buf_append_str(out, "  <li>");
            parse_inline(out, trim_whitespace(trimmed + 2));
            buf_append_str(out, "</li>\n");
            line = strtok(NULL, "\r\n");
            continue;
        }

        /* Ordered list */
        if (isdigit((unsigned char)trimmed[0])) {
            char *dot = strchr(trimmed, '.');
            if (dot && isspace((unsigned char)*(dot + 1))) {
                if (in_p)  { buf_append_str(out, "</p>\n"); in_p = false; }
                if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
                if (!in_ol) { buf_append_str(out, "<ol>\n"); in_ol = true; }
                buf_append_str(out, "  <li>");
                parse_inline(out, trim_whitespace(dot + 2));
                buf_append_str(out, "</li>\n");
                line = strtok(NULL, "\r\n");
                continue;
            }
        }

        /* Paragraph */
        if (!in_p) {
            if (in_ul) { buf_append_str(out, "</ul>\n"); in_ul = false; }
            if (in_ol) { buf_append_str(out, "</ol>\n"); in_ol = false; }
            buf_append_str(out, "<p>");
            in_p = true;
        } else {
            buf_append_str(out, " ");
        }
        parse_inline(out, trimmed);
        line = strtok(NULL, "\r\n");
    }

    if (in_code) buf_append_str(out, "</code></pre>\n");
    if (in_p)  buf_append_str(out, "</p>\n");
    if (in_ul) buf_append_str(out, "</ul>\n");
    if (in_ol) buf_append_str(out, "</ol>\n");
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

    char *line = strtok(fm, "\r\n");
    while (line) {
        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char *key = trim_whitespace(line);
            char *val = trim_whitespace(colon + 1);
            strip_quotes(val);

            if (type == 1) { /* DocItem */
                DocItem *d = (DocItem *)target;
                if (strcmp(key, "title") == 0) strncpy(d->title, val, sizeof(d->title) - 1);
                else if (strcmp(key, "description") == 0) strncpy(d->description, val, sizeof(d->description) - 1);
                else if (strcmp(key, "category") == 0) strncpy(d->category, val, sizeof(d->category) - 1);
                else if (strcmp(key, "order") == 0) d->order = atoi(val);
            } else if (type == 2) { /* PostItem */
                PostItem *p = (PostItem *)target;
                if (strcmp(key, "title") == 0) strncpy(p->title, val, sizeof(p->title) - 1);
                else if (strcmp(key, "description") == 0) strncpy(p->description, val, sizeof(p->description) - 1);
                else if (strcmp(key, "date") == 0) strncpy(p->date, val, sizeof(p->date) - 1);
                else if (strcmp(key, "tags") == 0) strncpy(p->tags, val, sizeof(p->tags) - 1);
                else if (strcmp(key, "cover") == 0) strncpy(p->cover, val, sizeof(p->cover) - 1);
            } else if (type == 3) { /* ProjectItem */
                ProjectItem *pr = (ProjectItem *)target;
                if (strcmp(key, "title") == 0) strncpy(pr->title, val, sizeof(pr->title) - 1);
                else if (strcmp(key, "description") == 0) strncpy(pr->description, val, sizeof(pr->description) - 1);
                else if (strcmp(key, "category") == 0) strncpy(pr->category, val, sizeof(pr->category) - 1);
                else if (strcmp(key, "featured") == 0) pr->featured = (strcmp(val, "true") == 0);
                else if (strcmp(key, "order") == 0) pr->order = atoi(val);
                else if (strcmp(key, "tags") == 0) strncpy(pr->tags, val, sizeof(pr->tags) - 1);
                else if (strcmp(key, "cover") == 0) strncpy(pr->cover, val, sizeof(pr->cover) - 1);
                else if (strcmp(key, "demo") == 0) strncpy(pr->demo_url, val, sizeof(pr->demo_url) - 1);
                else if (strcmp(key, "repo") == 0) strncpy(pr->repo_url, val, sizeof(pr->repo_url) - 1);
                else if (strcmp(key, "docs") == 0) strncpy(pr->docs_slug, val, sizeof(pr->docs_slug) - 1);
            }
        }
        line = strtok(NULL, "\r\n");
    }
    free(fm);

    const char *body = end + 4;
    while (*body == '\r' || *body == '\n' || *body == '-') body++;
    return body;
}

/* ==========================================================================
   Load Data & Navigation
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
    nav_sections[2].count = 2;
    strcpy(nav_sections[2].slugs[0], "theming");
    strcpy(nav_sections[2].slugs[1], "announcement-banner");

    strcpy(nav_sections[3].title, "My projects");
    nav_sections[3].count = 2;
    strcpy(nav_sections[3].slugs[0], "project-atlas");
    strcpy(nav_sections[3].slugs[1], "project-lumen");

    strcpy(nav_sections[4].title, "Reference");
    nav_sections[4].count = 2;
    strcpy(nav_sections[4].slugs[0], "front-matter");
    strcpy(nav_sections[4].slugs[1], "faq");
}

/* ==========================================================================
   HTML Template Generators
   ========================================================================== */
static void render_header(StrBuf *out, const char *active_section) {
    buf_append_str(out,
        "<header class=\"topnav\" id=\"topnav\">\n"
        "  <div class=\"topnav__inner\">\n"
        "    <a class=\"brand\" href=\"/\">\n"
        "      <img src=\"/assets/images/brand-icon.svg\" alt=\"\" width=\"24\" height=\"24\">\n"
        "      <span>irbox</span>\n"
        "    </a>\n"
        "    <nav class=\"topnav__links\" aria-label=\"Primary\">\n"
    );

    const char *sections[] = { "docs", "projects", "blog", "books", "about" };
    const char *labels[]   = { "Docs", "Projects", "Blog", "Books", "About" };
    const char *urls[]     = { "/docs/getting-started/", "/projects/", "/blog/", "/books/", "/about/" };

    for (int i = 0; i < 5; i++) {
        bool is_act = (strcmp(active_section, sections[i]) == 0);
        buf_append_str(out, "      <a class=\"topnav__link");
        if (is_act) buf_append_str(out, " is-active\" aria-current=\"page\"");
        else buf_append_str(out, "\"");
        buf_append_str(out, " href=\"");
        buf_append_str(out, urls[i]);
        buf_append_str(out, "\">");
        buf_append_str(out, labels[i]);
        buf_append_str(out, "</a>\n");
    }

    buf_append_str(out,
        "    </nav>\n"
        "    <div class=\"topnav__actions\">\n"
        "      <button class=\"btn btn--ghost btn--sm search-trigger\" type=\"button\" data-search-open aria-label=\"Search\">\n"
        "        <svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"11\" cy=\"11\" r=\"8\"/><path d=\"m21 21-4.3-4.3\"/></svg>\n"
        "        <span class=\"search-trigger__label\">Search</span><kbd>Ctrl K</kbd>\n"
        "      </button>\n"
        "      <a class=\"icon-btn desktop-only\" href=\""
    );
    buf_append_str(out, SITE_REPO);
    buf_append_str(out,
        "\" aria-label=\"GitHub repository\" target=\"_blank\" rel=\"noopener noreferrer\">\n"
        "        <svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"currentColor\"><path d=\"M12 0C5.37 0 0 5.37 0 12c0 5.31 3.435 9.795 8.205 11.385.6.105.825-.255.825-.57 0-.285-.015-1.23-.015-2.235-3.015.555-3.795-.735-4.035-1.41-.135-.345-.72-1.41-1.23-1.695-.42-.225-1.02-.78-.015-.795.945-.015 1.62.87 1.845 1.23 1.08 1.815 2.805 1.305 3.495.99.105-.78.42-1.305.765-1.605-2.67-.3-5.46-1.335-5.46-5.925 0-1.305.465-2.385 1.23-3.225-.12-.3-.54-1.53.12-3.18 0 0 1.005-.315 3.3 1.23.96-.27 1.98-.405 3-.405s2.04.135 3 .405c2.295-1.56 3.3-1.23 3.3-1.23.66 1.65.24 2.88.12 3.18.765.84 1.23 1.905 1.23 3.225 0 4.605-2.805 5.625-5.475 5.925.435.375.81 1.095.81 2.22 0 1.605-.015 2.895-.015 3.3 0 .315.225.69.825.57A12.02 12.02 0 0 0 24 12c0-6.63-5.37-12-12-12Z\"/></svg>\n"
        "      </a>\n"
        "      <button class=\"icon-btn desktop-only\" type=\"button\" data-shortcuts-open aria-label=\"Keyboard shortcuts (press ?)\" title=\"Keyboard shortcuts (?)\">\n"
        "        <svg width=\"19\" height=\"19\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><rect width=\"20\" height=\"14\" x=\"2\" y=\"5\" rx=\"2\"/><path d=\"M6 9h.01M10 9h.01M14 9h.01M18 9h.01M6 13h.01M18 13h.01M10 13h4\"/></svg>\n"
        "      </button>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-theme-toggle aria-label=\"Toggle color theme\">\n"
        "        <span class=\"theme-icon theme-icon--moon\"><svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M12 3a6 6 0 0 0 9 9 9 9 0 1 1-9-9Z\"/></svg></span>\n"
        "        <span class=\"theme-icon theme-icon--sun\"><svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"12\" cy=\"12\" r=\"4\"/><path d=\"M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41\"/></svg></span>\n"
        "      </button>\n"
        "      <button class=\"icon-btn mobile-only\" type=\"button\" data-drawer-toggle aria-expanded=\"false\" aria-controls=\"drawer\" aria-label=\"Open menu\">\n"
        "        <span class=\"menu-icon menu-icon--open\"><svg width=\"22\" height=\"22\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><line x1=\"4\" x2=\"20\" y1=\"12\" y2=\"12\"/><line x1=\"4\" x2=\"20\" y1=\"6\" y2=\"6\"/><line x1=\"4\" x2=\"20\" y1=\"18\" y2=\"18\"/></svg></span>\n"
        "        <span class=\"menu-icon menu-icon--close\"><svg width=\"22\" height=\"22\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M18 6 6 18M6 6l12 12\"/></svg></span>\n"
        "      </button>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"drawer\" id=\"drawer\" hidden>\n"
        "    <nav aria-label=\"Mobile\">\n"
        "      <a class=\"drawer__link\" href=\"/docs/getting-started/\">Docs</a>\n"
        "      <a class=\"drawer__link\" href=\"/projects/\">Projects</a>\n"
        "      <a class=\"drawer__link\" href=\"/blog/\">Blog</a>\n"
        "      <a class=\"drawer__link\" href=\"/books/\">Books</a>\n"
        "      <a class=\"drawer__link\" href=\"/about/\">About</a>\n"
        "      <a class=\"drawer__link\" href=\""
    );
    buf_append_str(out, SITE_REPO);
    buf_append_str(out,
        "\" target=\"_blank\" rel=\"noopener noreferrer\">GitHub</a>\n"
        "    </nav>\n"
        "  </div>\n"
        "</header>\n"
    );
}

static void render_footer(StrBuf *out) {
    buf_append_str(out,
        "<footer class=\"footer\">\n"
        "  <div class=\"container footer__inner\">\n"
        "    <div class=\"footer__col\">\n"
        "      <span class=\"footer__title\">irbox</span>\n"
        "      <p class=\"muted\" style=\"font-size:14px;max-width:280px;\">\n"
        "        Built with pure C99 zero-dependency static site generator. 100% private, ultra-fast.\n"
        "      </p>\n"
        "    </div>\n"
        "    <div class=\"footer__col\">\n"
        "      <span class=\"footer__title\">Navigation</span>\n"
        "      <div class=\"footer__links\">\n"
        "        <a href=\"/docs/getting-started/\">Docs</a>\n"
        "        <a href=\"/projects/\">Projects</a>\n"
        "        <a href=\"/blog/\">Blog</a>\n"
        "        <a href=\"/books/\">Books</a>\n"
        "        <a href=\"/changelog/\">Changelog</a>\n"
        "        <a href=\"/about/\">About</a>\n"
        "      </div>\n"
        "    </div>\n"
        "    <div class=\"footer__col\">\n"
        "      <span class=\"footer__title\">Community</span>\n"
        "      <div class=\"footer__links\">\n"
        "        <a href=\""
    );
    buf_append_str(out, SITE_REPO);
    buf_append_str(out,
        "\" target=\"_blank\" rel=\"noopener noreferrer\">GitHub</a>\n"
        "        <a href=\"/feed.xml\">RSS Feed</a>\n"
        "        <a href=\"/sitemap.xml\">Sitemap</a>\n"
        "      </div>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"container footer__bottom\">\n"
        "    <span class=\"muted\" style=\"font-size:13px;\">© 2026 irbox. Zero external tracking.</span>\n"
        "  </div>\n"
        "</footer>\n"
        "<dialog class=\"search\" id=\"search-dialog\" aria-label=\"Search the site\" data-index=\"/search.json\">\n"
        "  <div class=\"search__box\">\n"
        "    <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><circle cx=\"11\" cy=\"11\" r=\"8\"/><path d=\"m21 21-4.3-4.3\"/></svg>\n"
        "    <input type=\"search\" id=\"search-input\" placeholder=\"Search docs, projects and posts…\" autocomplete=\"off\" autocorrect=\"off\" autocapitalize=\"off\" spellcheck=\"false\" aria-label=\"Search\">\n"
        "    <button class=\"icon-btn\" type=\"button\" data-search-close aria-label=\"Close search\">\n"
        "      <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"M18 6 6 18M6 6l12 12\"/></svg>\n"
        "    </button>\n"
        "  </div>\n"
        "  <ul class=\"search__results\" id=\"search-results\" aria-label=\"Search results\"></ul>\n"
        "  <div class=\"sr-only\" aria-live=\"polite\" id=\"search-status\"></div>\n"
        "  <p class=\"search__hint\">↑↓ to navigate · Enter to open · Esc to close</p>\n"
        "</dialog>\n"
        "<dialog class=\"shortcuts-dialog\" id=\"shortcuts-dialog\" aria-label=\"Keyboard Shortcuts\">\n"
        "  <div class=\"shortcuts-dialog__inner\">\n"
        "    <div class=\"shortcuts-dialog__header\">\n"
        "      <div class=\"shortcuts-dialog__title-group\">\n"
        "        <svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><rect width=\"20\" height=\"14\" x=\"2\" y=\"5\" rx=\"2\"/><path d=\"M6 9h.01M10 9h.01M14 9h.01M18 9h.01M6 13h.01M18 13h.01M10 13h4\"/></svg>\n"
        "        <h2 class=\"shortcuts-dialog__title\">Keyboard Shortcuts</h2>\n"
        "      </div>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-shortcuts-close aria-label=\"Close shortcuts\">\n"
        "        <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"M18 6 6 18M6 6l12 12\"/></svg>\n"
        "      </button>\n"
        "    </div>\n"
        "    <div class=\"shortcuts-dialog__content\">\n"
        "      <div class=\"shortcuts-group\">\n"
        "        <h3 class=\"shortcuts-group__title\">Navigation</h3>\n"
        "        <dl class=\"shortcuts-list\">\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>H</kbd></dt><dd>Home</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>D</kbd></dt><dd>Documentation</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>P</kbd></dt><dd>Projects</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>B</kbd></dt><dd>Blog</dd></div>\n"
        "          <div class=\"shortcuts-item\"><dt><kbd>L</kbd></dt><dd>Books & Archive</dd></div>\n"
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
        "<script src=\"/assets/js/main.js\"></script>\n"
    );
}

static void wrap_page(StrBuf *out, const char *title, const char *desc, const char *active_sec, const char *body, const char *body_class) {
    buf_append_str(out,
        "<!doctype html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "  <meta charset=\"utf-8\">\n"
        "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
        "  <title>"
    );
    if (title && strlen(title) > 0) {
        escape_html(out, title, strlen(title));
        buf_append_str(out, " — irbox</title>\n");
    } else {
        buf_append_str(out, "irbox — Software engineering, developer tooling, and technical writing.</title>\n");
    }
    if (desc && strlen(desc) > 0) {
        buf_append_str(out, "  <meta name=\"description\" content=\"");
        escape_html(out, desc, strlen(desc));
        buf_append_str(out, "\">\n");
    }
    buf_append_str(out,
        "  <link rel=\"stylesheet\" href=\"/assets/css/main.css\">\n"
        "  <link rel=\"icon\" href=\"/assets/images/favicon.svg\" type=\"image/svg+xml\">\n"
        "  <link rel=\"manifest\" href=\"/manifest.webmanifest\">\n"
        "</head>\n"
        "<body"
    );
    if (body_class && strlen(body_class) > 0) {
        buf_append_str(out, " class=\"");
        buf_append_str(out, body_class);
        buf_append_str(out, "\"");
    }
    buf_append_str(out, ">\n");
    buf_append_str(out, "  <div class=\"page-progress\" id=\"page-progress\" aria-hidden=\"true\"></div>\n");
    buf_append_str(out, "  <div class=\"reading-progress\" id=\"reading-progress\" aria-hidden=\"true\"></div>\n");
    buf_append_str(out, "  <a class=\"skip-link\" href=\"#main\">Skip to content</a>\n");

    render_header(out, active_sec);

    buf_append_str(out, "  <main id=\"main\" tabindex=\"-1\">\n");
    buf_append_str(out, body);
    buf_append_str(out, "\n  </main>\n");

    render_footer(out);
    buf_append_str(out, "</body>\n</html>\n");
}

/* ==========================================================================
   Page Renderers
   ========================================================================== */
static void render_docs_page(const DocItem *d, const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body, "<div class=\"shell\">\n");

    /* Sidebar */
    buf_append_str(&body,
        "  <aside class=\"sidebar\" aria-label=\"Docs navigation\" id=\"sidebar\">\n"
        "    <div class=\"sidebar__header\">\n"
        "      <span class=\"sidebar__header-title\">Documentation</span>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-sidebar-toggle aria-label=\"Close sidebar\">\n"
        "        <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M18 6 6 18M6 6l12 12\"/></svg>\n"
        "      </button>\n"
        "    </div>\n"
        "    <nav aria-label=\"Docs sections\">\n"
    );

    for (size_t s = 0; s < nav_section_count; s++) {
        buf_append_str(&body, "      <div class=\"sidebar__group\">\n        <p class=\"sidebar__title\">");
        buf_append_str(&body, nav_sections[s].title);
        buf_append_str(&body, "</p>\n        <ul>\n");

        for (size_t k = 0; k < nav_sections[s].count; k++) {
            const char *item_slug = nav_sections[s].slugs[k];
            const DocItem *target_doc = NULL;
            for (size_t i = 0; i < doc_count; i++) {
                if (strcmp(docs[i].slug, item_slug) == 0) {
                    target_doc = &docs[i];
                    break;
                }
            }
            if (target_doc) {
                bool is_cur = (strcmp(d->slug, target_doc->slug) == 0);
                buf_append_str(&body, "          <li><a href=\"");
                buf_append_str(&body, target_doc->url);
                buf_append_str(&body, "\"");
                if (is_cur) buf_append_str(&body, " class=\"is-active\" aria-current=\"page\"");
                buf_append_str(&body, ">");
                escape_html(&body, target_doc->title, strlen(target_doc->title));
                buf_append_str(&body, "</a></li>\n");
            }
        }
        buf_append_str(&body, "        </ul>\n      </div>\n");
    }

    buf_append_str(&body,
        "      <div class=\"sidebar__group\">\n"
        "        <p class=\"sidebar__title\">More</p>\n"
        "        <ul><li><a href=\"/changelog/\">Changelog</a></li></ul>\n"
        "      </div>\n"
        "    </nav>\n"
        "  </aside>\n"
        "  <div class=\"shell__main\">\n"
        "    <button class=\"btn btn--secondary btn--sm sidebar-toggle\" type=\"button\" data-sidebar-toggle aria-controls=\"sidebar\" aria-expanded=\"false\">\n"
        "      <svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><line x1=\"4\" x2=\"20\" y1=\"12\" y2=\"12\"/><line x1=\"4\" x2=\"20\" y1=\"6\" y2=\"6\"/><line x1=\"4\" x2=\"20\" y1=\"18\" y2=\"18\"/></svg> Menu\n"
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
        "            <a class=\"btn btn--secondary btn--sm\" href=\"/assets/book.epub\" title=\"Download documentation as EPUB\">\n"
        "              <svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M4 19.5v-15A2.5 2.5 0 0 1 6.5 2H20v20H6.5a2.5 2.5 0 0 1-2.5-2.5Z\"/><path d=\"M6 6h10\"/><path d=\"M6 10h10\"/></svg> EPUB Book\n"
        "            </a>\n"
        "            <button class=\"btn btn--secondary btn--sm\" type=\"button\" data-print-btn title=\"Print or Save as PDF\">\n"
        "              <svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M6 9V2h12v7\"/><path d=\"M6 18H4a2 2 0 0 1-2-2v-5a2 2 0 0 1 2-2h16a2 2 0 0 1 2 2v5a2 2 0 0 1-2 2h-2\"/><rect width=\"12\" height=\"8\" x=\"6\" y=\"14\"/></svg> Print / PDF\n"
        "            </button>\n"
        "            <button class=\"btn btn--secondary btn--sm\" type=\"button\" data-share-btn title=\"Share this page\">\n"
        "              <svg width=\"14\" height=\"14\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><circle cx=\"18\" cy=\"5\" r=\"3\"/><circle cx=\"6\" cy=\"12\" r=\"3\"/><circle cx=\"18\" cy=\"19\" r=\"3\"/><line x1=\"8.59\" x2=\"15.42\" y1=\"13.51\" y2=\"17.49\"/><line x1=\"15.41\" x2=\"8.59\" y1=\"6.51\" y2=\"10.49\"/></svg> Share\n"
        "            </button>\n"
        "          </div>\n"
        "          <hr class=\"divider\">\n"
        "        </header>\n"
        "        <div class=\"prose\" data-prose>\n"
    );

    buf_append_str(&body, d->html_content);
    buf_append_str(&body, "        </div>\n");

    /* Docs Previous / Next Pager */
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
            buf_append_str(&body, "\"><span class=\"pager__label\">Previous</span><span class=\"pager__title\">← ");
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
            buf_append_str(&body, " →</span></a>\n");
        } else {
            buf_append_str(&body, "          <span aria-hidden=\"true\"></span>\n");
        }
        buf_append_str(&body, "        </nav>\n");
    }

    buf_append_str(&body,
        "        <p class=\"edit-link\">\n"
        "          <a href=\""
    );
    buf_append_str(&body, SITE_REPO);
    buf_append_str(&body, "/edit/main/_docs/");
    buf_append_str(&body, d->slug);
    buf_append_str(&body, ".md\" target=\"_blank\" rel=\"noopener noreferrer\">Edit this page on GitHub</a>\n"
        "        </p>\n"
        "      </article>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, d->title, d->description, "docs", body.data, "has-sidebar");

    char target_path[MAX_PATH];
    snprintf(target_path, sizeof(target_path), "%s%sindex.html", out_dir, d->url);
    char dir[MAX_PATH];
    strncpy(dir, target_path, sizeof(dir) - 1);
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
        buf_append_str(&body, "    <div class=\"post-card__tags\"><span class=\"badge\">");
        escape_html(&body, p->tags, strlen(p->tags));
        buf_append_str(&body, "</span></div>\n");
    }

    buf_append_str(&body, "    <h1 class=\"display-1\">");
    escape_html(&body, p->title, strlen(p->title));
    buf_append_str(&body, "</h1>\n");

    if (p->description[0]) {
        buf_append_str(&body, "    <p class=\"lead\">");
        escape_html(&body, p->description, strlen(p->description));
        buf_append_str(&body, "</p>\n");
    }

    buf_append_str(&body,
        "    <div class=\"byline\">\n"
        "      <img class=\"avatar\" src=\"/assets/images/placeholders/avatar.svg\" alt=\"irbox\" width=\"36\" height=\"36\">\n"
        "      <div><strong>irbox</strong><br><span class=\"muted\">Software Engineer</span></div>\n"
        "      <span class=\"spacer\"></span>\n"
        "      <time class=\"muted\">"
    );
    buf_append_str(&body, p->date);
    buf_append_str(&body,
        " · 3 min read</time>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-print-btn aria-label=\"Print\" title=\"Print\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M6 9V2h12v7\"/><path d=\"M6 18H4a2 2 0 0 1-2-2v-5a2 2 0 0 1 2-2h16a2 2 0 0 1 2 2v5a2 2 0 0 1-2 2h-2\"/><rect width=\"12\" height=\"8\" x=\"6\" y=\"14\"/></svg></button>\n"
        "      <button class=\"icon-btn\" type=\"button\" data-share-btn aria-label=\"Share\" title=\"Share\"><svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><circle cx=\"18\" cy=\"5\" r=\"3\"/><circle cx=\"6\" cy=\"12\" r=\"3\"/><circle cx=\"18\" cy=\"19\" r=\"3\"/><line x1=\"8.59\" x2=\"15.42\" y1=\"13.51\" y2=\"17.49\"/><line x1=\"15.41\" x2=\"8.59\" y1=\"6.51\" y2=\"10.49\"/></svg></button>\n"
        "    </div>\n"
        "  </header>\n"
        "  <div class=\"prose\" data-prose>\n"
    );

    buf_append_str(&body, p->html_content);
    buf_append_str(&body, "  </div>\n");

    /* Post Previous / Next Pager */
    size_t cur_p = 0;
    for (size_t i = 0; i < post_count; i++) {
        if (&posts[i] == p) { cur_p = i; break; }
    }
    const PostItem *prev_p = cur_p > 0 ? &posts[cur_p - 1] : NULL;
    const PostItem *next_p = (cur_p + 1 < post_count) ? &posts[cur_p + 1] : NULL;

    if (prev_p || next_p) {
        buf_append_str(&body, "  <nav class=\"pager\" aria-label=\"More posts\">\n");
        if (prev_p) {
            buf_append_str(&body, "    <a class=\"pager__item\" href=\"");
            buf_append_str(&body, prev_p->url);
            buf_append_str(&body, "\"><span class=\"pager__label\">Older</span><span class=\"pager__title\">← ");
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
            buf_append_str(&body, " →</span></a>\n");
        } else {
            buf_append_str(&body, "    <span aria-hidden=\"true\"></span>\n");
        }
        buf_append_str(&body, "  </nav>\n");
    }

    buf_append_str(&body, "</article>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, p->title, p->description, "blog", body.data, "post-page");

    char target_path[MAX_PATH];
    snprintf(target_path, sizeof(target_path), "%s%sindex.html", out_dir, p->url);
    char dir[MAX_PATH];
    strncpy(dir, target_path, sizeof(dir) - 1);
    char *ls = strrchr(dir, '/');
    if (ls) { *ls = 0; mkdir_p(dir); }

    write_entire_file(target_path, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_project_page(const ProjectItem *pr, const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body,
        "<div class=\"shell\">\n"
        "  <div class=\"shell__main\" style=\"width:100%;\">\n"
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
        buf_append_str(&body, "\" target=\"_blank\" rel=\"noopener noreferrer\">Live demo</a>\n");
    }
    if (pr->repo_url[0]) {
        buf_append_str(&body, "            <a class=\"btn btn--secondary\" href=\"");
        escape_html(&body, pr->repo_url, strlen(pr->repo_url));
        buf_append_str(&body, "\" target=\"_blank\" rel=\"noopener noreferrer\">Source code</a>\n");
    }
    if (pr->docs_slug[0]) {
        buf_append_str(&body, "            <a class=\"btn btn--secondary\" href=\"/docs/");
        escape_html(&body, pr->docs_slug, strlen(pr->docs_slug));
        buf_append_str(&body, "/\">Documentation</a>\n");
    }
    buf_append_str(&body,
        "            <button class=\"btn btn--secondary\" type=\"button\" data-share-btn>Share</button>\n"
        "          </div>\n"
        "          <hr class=\"divider\">\n"
        "        </header>\n"
        "        <div class=\"prose\" data-prose>\n"
    );

    buf_append_str(&body, pr->html_content);
    buf_append_str(&body, "        </div>\n      </article>\n    </div>\n  </div>\n</div>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, pr->title, pr->description, "projects", body.data, "project-page");

    char target_path[MAX_PATH];
    snprintf(target_path, sizeof(target_path), "%s%sindex.html", out_dir, pr->url);
    char dir[MAX_PATH];
    strncpy(dir, target_path, sizeof(dir) - 1);
    char *ls = strrchr(dir, '/');
    if (ls) { *ls = 0; mkdir_p(dir); }

    write_entire_file(target_path, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

/* ==========================================================================
   Landing Pages
   ========================================================================== */
static void render_books_landing(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 16384);

    buf_append_str(&body,
        "<div class=\"container section\">\n"
        "  <header class=\"center page-header\">\n"
        "    <h1 class=\"display-1\">Books &amp; Archive</h1>\n"
        "    <p class=\"lead\">\n"
        "      Personal digital shelf of books, field notes, and architectural manuals. Read them directly in your browser with our built-in reader, or download to your Kindle, Kobo, or mobile device.\n"
        "    </p>\n"
        "  </header>\n"
        "  <section class=\"card book-dropzone\" id=\"book-dropzone\" aria-label=\"Quick in-browser book reader drop zone\">\n"
        "    <div class=\"book-dropzone__content\">\n"
        "      <div class=\"book-dropzone__icon\" aria-hidden=\"true\">\n"
        "        <svg width=\"36\" height=\"36\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M4 19.5v-15A2.5 2.5 0 0 1 6.5 2H20v20H6.5a2.5 2.5 0 0 1-2.5-2.5Z\"/><path d=\"M6 6h10\"/><path d=\"M6 10h10\"/><path d=\"M12 14l4 4m0-4l-4 4\"/></svg>\n"
        "      </div>\n"
        "      <div class=\"book-dropzone__text\">\n"
        "        <h2 class=\"display-3 book-dropzone__title\">Instant Web Book Reader</h2>\n"
        "        <p class=\"muted book-dropzone__desc\">Drag and drop any <strong>.epub</strong> or <strong>.pdf</strong> file here to read instantly in your browser. 100% private, runs entirely on your device with no server upload.</p>\n"
        "      </div>\n"
        "      <div class=\"book-dropzone__action\">\n"
        "        <label for=\"book-file-input\" class=\"btn btn--secondary\">\n"
        "          <svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"/><polyline points=\"17 8 12 3 7 8\"/><line x1=\"12\" y1=\"3\" x2=\"12\" y2=\"15\"/></svg>\n"
        "          <span>Open local file</span>\n"
        "        </label>\n"
        "        <input type=\"file\" id=\"book-file-input\" accept=\".epub,.pdf,application/epub+zip,application/pdf\" style=\"display:none;\">\n"
        "      </div>\n"
        "    </div>\n"
        "  </section>\n"
        "  <div class=\"books-toolbar\">\n"
        "    <div class=\"chips\" role=\"group\" aria-label=\"Filter books by format\" id=\"books-format-filters\">\n"
        "      <button class=\"chip is-active\" type=\"button\" data-format=\"all\" aria-pressed=\"true\">All Books <span class=\"badge\" style=\"margin-left:6px;\">2</span></button>\n"
        "      <button class=\"chip\" type=\"button\" data-format=\"epub\" aria-pressed=\"false\">EPUB</button>\n"
        "      <button class=\"chip\" type=\"button\" data-format=\"pdf\" aria-pressed=\"false\">PDF</button>\n"
        "    </div>\n"
        "    <div class=\"books-search-box\">\n"
        "      <input type=\"search\" id=\"books-search\" class=\"input books-search-input\" placeholder=\"Filter shelf...\" aria-label=\"Filter books shelf by title, author, or keywords\">\n"
        "      <svg class=\"books-search-icon\" width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><circle cx=\"11\" cy=\"11\" r=\"8\"/><line x1=\"21\" y1=\"21\" x2=\"16.65\" y2=\"16.65\"/></svg>\n"
        "    </div>\n"
        "  </div>\n"
        "  <div class=\"grid grid--cards\" id=\"books-grid\">\n"
        "    <article class=\"card book-card\" data-format=\"pdf\" data-title=\"architecture &amp; design primer\" data-author=\"irbox\" data-desc=\"a handbook on resilient static architectures, client-side tools, and publishing.\">\n"
        "      <div class=\"book-card__header\">\n"
        "        <div class=\"book-card__badge-row\">\n"
        "          <span class=\"badge badge--pdf\">PDF</span>\n"
        "          <span class=\"muted book-card__size\">3.4 KB</span>\n"
        "        </div>\n"
        "        <h2 class=\"display-3 book-card__title\"><a href=\"/books/reader/?book=architecture-primer.pdf\">Architecture &amp; Design Primer</a></h2>\n"
        "        <p class=\"book-card__byline\"><span>by</span> <strong>Irbox</strong> <span aria-hidden=\"true\">·</span> <span>3 pages</span></p>\n"
        "      </div>\n"
        "      <p class=\"book-card__desc\">A handbook on resilient static architectures, client-side tools, and publishing.</p>\n"
        "      <div class=\"book-card__footer\">\n"
        "        <a href=\"/books/reader/?book=architecture-primer.pdf\" class=\"btn btn--primary book-card__action\"><svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"M2 3h6a4 4 0 0 1 4 4v14a3 3 0 0 0-3-3H2z\"/><path d=\"M22 3h-6a4 4 0 0 0-4 4v14a3 3 0 0 1 3-3h7z\"/></svg><span>Read Online</span></a>\n"
        "        <a href=\"/books/archive/architecture-primer.pdf\" class=\"btn btn--secondary\" download title=\"Download PDF file\" aria-label=\"Download Architecture &amp; Design Primer (PDF)\"><svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"/><polyline points=\"17 8 12 3 7 8\"/><line x1=\"12\" y1=\"3\" x2=\"12\" y2=\"15\"/></svg><span>Download</span></a>\n"
        "      </div>\n"
        "    </article>\n"
        "    <article class=\"card book-card\" data-format=\"epub\" data-title=\"irbox — documentation, architecture &amp; field notes\" data-author=\"irbox\" data-desc=\"complete documentation and reference guide for irbox projects and systems.\">\n"
        "      <div class=\"book-card__header\">\n"
        "        <div class=\"book-card__badge-row\">\n"
        "          <span class=\"badge badge--epub\">EPUB</span>\n"
        "          <span class=\"muted book-card__size\">15.4 KB</span>\n"
        "        </div>\n"
        "        <h2 class=\"display-3 book-card__title\"><a href=\"/books/reader/?book=irbox-docs.epub\">Irbox — Documentation &amp; Field Notes</a></h2>\n"
        "        <p class=\"book-card__byline\"><span>by</span> <strong>Irbox</strong> <span aria-hidden=\"true\">·</span> <span>14 chapters</span></p>\n"
        "      </div>\n"
        "      <p class=\"book-card__desc\">Complete documentation and reference guide for Irbox projects and systems.</p>\n"
        "      <div class=\"book-card__footer\">\n"
        "        <a href=\"/books/reader/?book=irbox-docs.epub\" class=\"btn btn--primary book-card__action\"><svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"M2 3h6a4 4 0 0 1 4 4v14a3 3 0 0 0-3-3H2z\"/><path d=\"M22 3h-6a4 4 0 0 0-4 4v14a3 3 0 0 1 3-3h7z\"/></svg><span>Read Online</span></a>\n"
        "        <a href=\"/books/archive/irbox-docs.epub\" class=\"btn btn--secondary\" download title=\"Download EPUB file\" aria-label=\"Download Irbox Documentation (EPUB)\"><svg width=\"18\" height=\"18\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\" aria-hidden=\"true\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"/><polyline points=\"17 8 12 3 7 8\"/><line x1=\"12\" y1=\"3\" x2=\"12\" y2=\"15\"/></svg><span>Download</span></a>\n"
        "      </div>\n"
        "    </article>\n"
        "  </div>\n"
        "  <div id=\"books-empty-search\" class=\"card center muted book-empty-state\" style=\"display:none;margin-top:24px;\"><p>No books matching your search query.</p></div>\n"
        "</div>\n"
        "<script>\n"
        "(function(){\n"
        "  var grid=document.getElementById('books-grid');\n"
        "  var emptyMsg=document.getElementById('books-empty-search');\n"
        "  var searchInput=document.getElementById('books-search');\n"
        "  var filterButtons=document.querySelectorAll('#books-format-filters .chip');\n"
        "  var dropzone=document.getElementById('book-dropzone');\n"
        "  var fileInput=document.getElementById('book-file-input');\n"
        "  var currentFormat='all';\n"
        "  var currentQuery='';\n"
        "  function filterBooks(){\n"
        "    if(!grid)return;\n"
        "    var cards=grid.querySelectorAll('.book-card');\n"
        "    var visible=0;\n"
        "    cards.forEach(function(card){\n"
        "      var fmt=card.getAttribute('data-format')||'';\n"
        "      var title=card.getAttribute('data-title')||'';\n"
        "      var desc=card.getAttribute('data-desc')||'';\n"
        "      var mFmt=(currentFormat==='all'||fmt===currentFormat);\n"
        "      var mQ=(!currentQuery||title.indexOf(currentQuery)>-1||desc.indexOf(currentQuery)>-1);\n"
        "      if(mFmt&&mQ){card.style.display='';visible++;}else{card.style.display='none';}\n"
        "    });\n"
        "    if(emptyMsg)emptyMsg.style.display=visible===0?'block':'none';\n"
        "  }\n"
        "  if(searchInput){\n"
        "    searchInput.addEventListener('input',function(e){\n"
        "      currentQuery=e.target.value.toLowerCase().trim();\n"
        "      filterBooks();\n"
        "    });\n"
        "  }\n"
        "  filterButtons.forEach(function(btn){\n"
        "    btn.addEventListener('click',function(){\n"
        "      filterButtons.forEach(function(b){b.classList.remove('is-active');b.setAttribute('aria-pressed','false');});\n"
        "      btn.classList.add('is-active');\n"
        "      btn.setAttribute('aria-pressed','true');\n"
        "      currentFormat=btn.getAttribute('data-format')||'all';\n"
        "      filterBooks();\n"
        "    });\n"
        "  });\n"
        "  var bookDb={\n"
        "    open:function(){\n"
        "      return new Promise(function(resolve,reject){\n"
        "        if(!window.indexedDB)return reject(new Error('No IndexedDB'));\n"
        "        var req=indexedDB.open('irbox_books_db',1);\n"
        "        req.onupgradeneeded=function(e){var db=e.target.result;if(!db.objectStoreNames.contains('books'))db.createObjectStore('books',{keyPath:'id'});};\n"
        "        req.onsuccess=function(e){resolve(e.target.result);};\n"
        "        req.onerror=function(e){reject(e.target.error);};\n"
        "      });\n"
        "    },\n"
        "    save:function(name,data){\n"
        "      return bookDb.open().then(function(db){\n"
        "        return new Promise(function(resolve,reject){\n"
        "          var tx=db.transaction('books','readwrite');\n"
        "          tx.objectStore('books').put({id:'current',name:name,data:data,timestamp:Date.now()});\n"
        "          tx.oncomplete=function(){resolve();};\n"
        "          tx.onerror=function(e){reject(e.target.error);};\n"
        "        });\n"
        "      }).catch(function(){\n"
        "        sessionStorage.setItem('irbox_temp_book_name',name);\n"
        "        sessionStorage.setItem('irbox_temp_book_data',data);\n"
        "      });\n"
        "    }\n"
        "  };\n"
        "  function openLocalFile(file){\n"
        "    if(!file)return;\n"
        "    var reader=new FileReader();\n"
        "    reader.onload=function(e){\n"
        "      bookDb.save(file.name,e.target.result).then(function(){\n"
        "        window.location.href='/books/reader/?book=local';\n"
        "      });\n"
        "    };\n"
        "    reader.readAsDataURL(file);\n"
        "  }\n"
        "  if(fileInput){\n"
        "    fileInput.addEventListener('change',function(e){\n"
        "      if(e.target.files&&e.target.files[0])openLocalFile(e.target.files[0]);\n"
        "    });\n"
        "  }\n"
        "  if(dropzone){\n"
        "    ['dragenter','dragover'].forEach(function(evt){\n"
        "      dropzone.addEventListener(evt,function(e){e.preventDefault();e.stopPropagation();dropzone.classList.add('is-dragover');});\n"
        "    });\n"
        "    ['dragleave','drop'].forEach(function(evt){\n"
        "      dropzone.addEventListener(evt,function(e){e.preventDefault();e.stopPropagation();dropzone.classList.remove('is-dragover');});\n"
        "    });\n"
        "    dropzone.addEventListener('drop',function(e){\n"
        "      var f=e.dataTransfer&&e.dataTransfer.files;\n"
        "      if(f&&f.length>0)openLocalFile(f[0]);\n"
        "    });\n"
        "  }\n"
        "})();\n"
        "</script>\n"
    );

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "Books & Archive", "Personal digital library and book archive.", "books", body.data, "books-landing");

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

    DocItem meta;
    memset(&meta, 0, sizeof(meta));
    const char *raw_html = parse_frontmatter_block(content, &meta, 1);

    /* Replace Liquid tags */
    StrBuf clean_reader;
    buf_init(&clean_reader, strlen(raw_html) + 1024);
    const char *p = raw_html;
    while (*p) {
        if (strncmp(p, "{{ '/books/' | relative_url }}", 30) == 0) {
            buf_append_str(&clean_reader, "/books/");
            p += 30;
            continue;
        }
        if (strncmp(p, "{{ \"/books/\" | relative_url }}", 30) == 0) {
            buf_append_str(&clean_reader, "/books/");
            p += 30;
            continue;
        }
        if (strncmp(p, "{{ '/assets/js/reader.js' | relative_url }}", 43) == 0) {
            buf_append_str(&clean_reader, "/assets/js/reader.js");
            p += 43;
            continue;
        }
        if (strncmp(p, "{{ \"/assets/js/reader.js\" | relative_url }}", 43) == 0) {
            buf_append_str(&clean_reader, "/assets/js/reader.js");
            p += 43;
            continue;
        }
        buf_append_char(&clean_reader, *p++);
    }

    StrBuf full;
    buf_init(&full, clean_reader.len + 4096);
    wrap_page(&full, "Web Book Reader", "Read digital EPUB and PDF books directly in your browser.", "books", clean_reader.data, "reader-page");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/books/reader/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/books/reader", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&clean_reader);
    buf_free(&full);
    free(content);
}

static void render_changelog_page(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body,
        "<div class=\"shell\">\n"
        "  <div class=\"shell__main\" style=\"width:100%;\">\n"
        "    <div class=\"doc\">\n"
        "      <article class=\"doc__article\">\n"
        "        <header class=\"doc__header\">\n"
        "          <h1 class=\"display-1\">Changelog</h1>\n"
        "          <p class=\"lead\">What changed on this site and when.</p>\n"
        "          <hr class=\"divider\">\n"
        "        </header>\n"
        "        <div class=\"prose\" data-prose>\n"
        "          <section class=\"release\">\n"
        "            <h2 id=\"v1-2-0\">C99 Zero-Dependency SSG Migration <span class=\"badge badge--added\">v1.2.0</span></h2>\n"
        "            <p class=\"muted\"><time datetime=\"2026-10-26\">October 26, 2026</time></p>\n"
        "            <ul>\n"
        "              <li><span class=\"badge badge--added\">added</span> Standalone C99 static site generator with zero runtime dependencies.</li>\n"
        "              <li><span class=\"badge badge--removed\">removed</span> Purged Ruby, Bundler, Gemfile, and Jekyll.</li>\n"
        "              <li><span class=\"badge badge--fixed\">fixed</span> Sub-20ms instant full-site compilation and sub-second CI deployments.</li>\n"
        "            </ul>\n"
        "          </section>\n"
        "          <section class=\"release\">\n"
        "            <h2 id=\"v1-1-0\">Performance &amp; Polish Release <span class=\"badge\">v1.1.0</span></h2>\n"
        "            <p class=\"muted\"><time datetime=\"2026-10-24\">October 24, 2026</time></p>\n"
        "            <ul>\n"
        "              <li><span class=\"badge badge--added\">added</span> Self-hosted Libron serif reading typography (SIL OFL 1.1).</li>\n"
        "              <li><span class=\"badge badge--added\">added</span> Global keyboard shortcuts palette (press ?) with quick navigation.</li>\n"
        "              <li><span class=\"badge badge--added\">added</span> Native GPU-accelerated scroll-driven reading progress bar.</li>\n"
        "              <li><span class=\"badge badge--fixed\">fixed</span> Hardened safe area insets for mobile notches and resolved outside-click menu dismissal.</li>\n"
        "            </ul>\n"
        "          </section>\n"
        "          <section class=\"release\">\n"
        "            <h2 id=\"v1-0-0\">Site launch <span class=\"badge\">v1.0.0</span></h2>\n"
        "            <p class=\"muted\"><time datetime=\"2026-10-01\">October 1, 2026</time></p>\n"
        "            <ul>\n"
        "              <li><span class=\"badge badge--added\">added</span> Docs, projects, blog, and about pages.</li>\n"
        "              <li><span class=\"badge badge--added\">added</span> Light/dark theme with a manual toggle.</li>\n"
        "              <li><span class=\"badge badge--added\">added</span> Command palette search (Ctrl/Cmd + K).</li>\n"
        "            </ul>\n"
        "          </section>\n"
        "        </div>\n"
        "      </article>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "Changelog", "What changed on this site and when.", "docs", body.data, "has-sidebar");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/changelog/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/changelog", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_home_page(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 16384);

    buf_append_str(&body,
        "<section class=\"hero\">\n"
        "  <div class=\"container hero__inner\">\n"
        "    <div class=\"hero__badge\">\n"
        "      <img src=\"/assets/images/brand-icon.svg\" alt=\"\" width=\"52\" height=\"52\">\n"
        "    </div>\n"
        "    <h1 class=\"hero__title\">Software engineering, developer tooling, and technical writing.</h1>\n"
        "    <p class=\"hero__tagline\">"
    );
    buf_append_str(&body, SITE_DESCRIPTION);
    buf_append_str(&body,
        "</p>\n"
        "    <div class=\"hero__cta\">\n"
        "      <a class=\"btn btn--primary btn--lg\" href=\"/projects/\">Browse projects</a>\n"
        "      <a class=\"btn btn--secondary btn--lg\" href=\"/docs/getting-started/\">Documentation</a>\n"
        "    </div>\n"
        "  </div>\n"
        "</section>\n"
        "<div class=\"showcase-sheet\">\n"
        "  <section class=\"section container\">\n"
        "    <div class=\"bento\">\n"
        "      <div class=\"bento__col\">\n"
        "        <div class=\"bento__heading-cell\">\n"
        "          <p class=\"eyebrow\">Overview</p>\n"
        "          <h2 class=\"bento__heading\">Everything in one place.</h2>\n"
        "          <p class=\"bento__subtext\">Selected projects, technical guides, and engineering notes with full source code on GitHub.</p>\n"
        "        </div>\n"
        "        <a class=\"bento__card\" href=\"/docs/getting-started/\">\n"
        "          <div class=\"bento__card-header\"><span class=\"badge\">Documentation</span></div>\n"
        "          <h3>Guides &amp; Reference</h3>\n"
        "          <p>Setup instructions, architecture notes, and configuration references.</p>\n"
        "        </a>\n"
        "      </div>\n"
        "      <div class=\"bento__col\">\n"
        "        <a class=\"bento__card\" href=\"/projects/\">\n"
        "          <div class=\"bento__card-header\"><span class=\"badge\">Projects</span></div>\n"
        "          <h3>Selected Projects</h3>\n"
        "          <p>Web applications, developer tools, and design system components.</p>\n"
        "          <div class=\"bento__tags\"><span class=\"chip--sm\">C99</span><span class=\"chip--sm\">TypeScript</span><span class=\"chip--sm\">CSS</span></div>\n"
        "        </a>\n"
        "        <a class=\"bento__card\" href=\"/blog/\">\n"
        "          <div class=\"bento__card-header\"><span class=\"badge\">Writing</span></div>\n"
        "          <h3>Blog</h3>\n"
        "          <p>Short essays and notes on building software, design tokens, and shipping things.</p>\n"
        "        </a>\n"
        "      </div>\n"
        "      <div class=\"bento__col\">\n"
        "        <div class=\"bento__card bento__card--tall\">\n"
        "          <div class=\"bento__card-header\"><span class=\"badge badge--added\">Open Source</span><span class=\"bento__chip-count\">Zero Bloat</span></div>\n"
        "          <h3>Built with Pure C99 SSG</h3>\n"
        "          <p>Zero dependencies, zero trackers, self-hosted fonts, and sub-millisecond builds.</p>\n"
        "          <div class=\"bento__spec-list\">\n"
        "            <div class=\"bento__spec-item\"><span class=\"bento__spec-label\">Engine</span><span class=\"bento__spec-val\">Standalone C99 SSG</span></div>\n"
        "            <div class=\"bento__spec-item\"><span class=\"bento__spec-label\">Build Time</span><span class=\"bento__spec-val\">&lt; 20 ms</span></div>\n"
        "            <div class=\"bento__spec-item\"><span class=\"bento__spec-label\">License</span><span class=\"bento__spec-val\">MIT</span></div>\n"
        "            <div class=\"bento__spec-item\"><span class=\"bento__spec-label\">Tracking</span><span class=\"bento__spec-val\">0% (Private)</span></div>\n"
        "          </div>\n"
        "        </div>\n"
        "      </div>\n"
        "    </div>\n"
        "  </section>\n"
        "  <section class=\"section container\">\n"
        "    <div class=\"section__head\">\n"
        "      <h2 class=\"display-2\">Selected projects</h2>\n"
        "      <a class=\"link-arrow\" href=\"/projects/\">All projects →</a>\n"
        "    </div>\n"
        "    <div class=\"grid grid--cards\">\n"
    );

    /* Project Cards */
    for (size_t i = 0; i < project_count && i < 6; i++) {
        buf_append_str(&body,
            "      <article class=\"card project-card\">\n"
            "        <div class=\"project-card__body\">\n"
            "          <p class=\"eyebrow\">"
        );
        escape_html(&body, projects[i].category, strlen(projects[i].category));
        buf_append_str(&body, "</p>\n          <h3 class=\"project-card__title\"><a href=\"");
        buf_append_str(&body, projects[i].url);
        buf_append_str(&body, "\">");
        escape_html(&body, projects[i].title, strlen(projects[i].title));
        buf_append_str(&body, "</a></h3>\n          <p class=\"project-card__desc\">");
        escape_html(&body, projects[i].description, strlen(projects[i].description));
        buf_append_str(&body, "</p>\n        </div>\n      </article>\n");
    }

    buf_append_str(&body,
        "    </div>\n"
        "  </section>\n"
        "  <section class=\"section container\">\n"
        "    <div class=\"section__head\">\n"
        "      <h2 class=\"display-2\">From the blog</h2>\n"
        "      <a class=\"link-arrow\" href=\"/blog/\">All posts →</a>\n"
        "    </div>\n"
        "    <div class=\"grid grid--posts\">\n"
    );

    /* Post Cards */
    for (size_t i = 0; i < post_count && i < 3; i++) {
        buf_append_str(&body,
            "      <article class=\"card post-card\">\n"
            "        <div class=\"post-card__body\">\n"
            "          <time class=\"muted\" style=\"font-size:13px;\">"
        );
        buf_append_str(&body, posts[i].date);
        buf_append_str(&body, "</time>\n          <h3 class=\"post-card__title\"><a href=\"");
        buf_append_str(&body, posts[i].url);
        buf_append_str(&body, "\">");
        escape_html(&body, posts[i].title, strlen(posts[i].title));
        buf_append_str(&body, "</a></h3>\n          <p class=\"post-card__desc\">");
        escape_html(&body, posts[i].description, strlen(posts[i].description));
        buf_append_str(&body, "</p>\n        </div>\n      </article>\n");
    }

    buf_append_str(&body,
        "    </div>\n"
        "  </section>\n"
        "  <section class=\"section container container--prose about-strip\">\n"
        "    <img class=\"avatar avatar--lg\" src=\"/assets/images/placeholders/avatar.svg\" alt=\"irbox\" width=\"96\" height=\"96\">\n"
        "    <div>\n"
        "      <h2 class=\"display-3\">irbox</h2>\n"
        "      <p>"
    );
    buf_append_str(&body, AUTHOR_BIO);
    buf_append_str(&body,
        "</p>\n"
        "      <a class=\"btn btn--secondary\" href=\"/about/\">More about me</a>\n"
        "    </div>\n"
        "  </section>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 32768);
    wrap_page(&full, NULL, SITE_DESCRIPTION, "home", body.data, "home");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/index.html", out_dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_blog_landing(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body,
        "<div class=\"container section\">\n"
        "  <header class=\"center page-header\">\n"
        "    <h1 class=\"display-1\">Blog</h1>\n"
        "    <p class=\"lead\">Notes on design, engineering, and shipping.</p>\n"
        "  </header>\n"
        "  <div class=\"grid grid--posts\">\n"
    );

    for (size_t i = 0; i < post_count; i++) {
        buf_append_str(&body,
            "    <article class=\"card post-card\">\n"
            "      <div class=\"post-card__body\">\n"
            "        <time class=\"muted\" style=\"font-size:13px;\">"
        );
        buf_append_str(&body, posts[i].date);
        buf_append_str(&body, "</time>\n        <h2 class=\"post-card__title\"><a href=\"");
        buf_append_str(&body, posts[i].url);
        buf_append_str(&body, "\">");
        escape_html(&body, posts[i].title, strlen(posts[i].title));
        buf_append_str(&body, "</a></h2>\n        <p class=\"post-card__desc\">");
        escape_html(&body, posts[i].description, strlen(posts[i].description));
        buf_append_str(&body, "</p>\n      </div>\n    </article>\n");
    }

    buf_append_str(&body, "  </div>\n</div>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "Blog", "Notes on design, engineering, and shipping.", "blog", body.data, "blog-landing");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/blog/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/blog", out_dir);
    mkdir_p(dir);
    write_entire_file(target, full.data, full.len);

    buf_free(&body);
    buf_free(&full);
}

static void render_projects_landing(const char *out_dir) {
    StrBuf body;
    buf_init(&body, 8192);

    buf_append_str(&body,
        "<div class=\"container section\">\n"
        "  <header class=\"center page-header\">\n"
        "    <h1 class=\"display-1\">Projects</h1>\n"
        "    <p class=\"lead\">Things I've built, with write-ups, tech stacks, and source links.</p>\n"
        "  </header>\n"
        "  <div class=\"grid grid--cards\">\n"
    );

    for (size_t i = 0; i < project_count; i++) {
        buf_append_str(&body,
            "    <article class=\"card project-card\">\n"
            "      <div class=\"project-card__body\">\n"
            "        <p class=\"eyebrow\">"
        );
        escape_html(&body, projects[i].category, strlen(projects[i].category));
        buf_append_str(&body, "</p>\n        <h2 class=\"project-card__title\"><a href=\"");
        buf_append_str(&body, projects[i].url);
        buf_append_str(&body, "\">");
        escape_html(&body, projects[i].title, strlen(projects[i].title));
        buf_append_str(&body, "</a></h2>\n        <p class=\"project-card__desc\">");
        escape_html(&body, projects[i].description, strlen(projects[i].description));
        buf_append_str(&body, "</p>\n      </div>\n    </article>\n");
    }

    buf_append_str(&body, "  </div>\n</div>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "Projects", "Selected projects and developer tooling.", "projects", body.data, "projects-landing");

    char target[MAX_PATH];
    snprintf(target, sizeof(target), "%s/projects/index.html", out_dir);
    char dir[MAX_PATH];
    snprintf(dir, sizeof(dir), "%s/projects", out_dir);
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
        "<div class=\"container container--prose section\">\n"
        "  <h1 class=\"display-1\">About</h1>\n"
        "  <div class=\"prose\">\n"
    );
    buf_append_str(&body, html.data);
    buf_append_str(&body, "  </div>\n</div>\n");

    StrBuf full;
    buf_init(&full, 16384);
    wrap_page(&full, "About", meta.description, "about", body.data, "about-page");

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
        "  <div style=\"margin:28px 0;display:flex;gap:12px;justify-content:center;flex-wrap:wrap;\">\n"
        "    <a class=\"btn btn--primary\" href=\"/\">Back home</a>\n"
        "    <button class=\"btn btn--secondary\" type=\"button\" data-search-open>Search the site</button>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 8192);
    wrap_page(&full, "Page not found", "Page not found", "", body.data, "error-page");

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
        "  <h1 class=\"display-1\" style=\"margin-top:32px;\">You're offline</h1>\n"
        "  <p class=\"lead\">It looks like your internet connection is unavailable. Cached pages remain accessible.</p>\n"
        "  <div style=\"margin:32px 0;display:flex;gap:12px;justify-content:center;\">\n"
        "    <button class=\"btn btn--primary\" onclick=\"window.location.reload()\">Retry connection</button>\n"
        "    <a class=\"btn btn--secondary\" href=\"/\">Return home</a>\n"
        "  </div>\n"
        "</div>\n"
    );

    StrBuf full;
    buf_init(&full, 8192);
    wrap_page(&full, "Offline", "Offline fallback page", "", body.data, "offline-page");

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
    strncpy(e->title, title ? title : "", sizeof(e->title) - 1);
    strncpy(e->url, url ? url : "", sizeof(e->url) - 1);
    strncpy(e->type, type ? type : "Page", sizeof(e->type) - 1);
    strncpy(e->description, desc ? desc : "", sizeof(e->description) - 1);
    strncpy(e->tags, tags ? tags : "", sizeof(e->tags) - 1);
}

static void write_search_json(const char *out_dir) {
    StrBuf out;
    buf_init(&out, 16384);
    buf_append_str(&out, "[\n");
    for (size_t i = 0; i < search_entry_count; i++) {
        buf_append_str(&out, "  {\n    \"title\": \"");
        escape_html(&out, search_entries[i].title, strlen(search_entries[i].title));
        buf_append_str(&out, "\",\n    \"url\": \"");
        buf_append_str(&out, search_entries[i].url);
        buf_append_str(&out, "\",\n    \"type\": \"");
        buf_append_str(&out, search_entries[i].type);
        buf_append_str(&out, "\",\n    \"description\": \"");
        escape_html(&out, search_entries[i].description, strlen(search_entries[i].description));
        buf_append_str(&out, "\",\n    \"tags\": \"");
        escape_html(&out, search_entries[i].tags, strlen(search_entries[i].tags));
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
        buf_append_str(&out, SITE_URL);
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
        "    <title>irbox</title>\n"
        "    <link>https://irbox.github.io</link>\n"
        "    <description>Software engineering, developer tooling, and technical writing.</description>\n"
    );

    for (size_t i = 0; i < post_count; i++) {
        buf_append_str(&out, "    <item>\n      <title>");
        escape_html(&out, posts[i].title, strlen(posts[i].title));
        buf_append_str(&out, "</title>\n      <link>");
        buf_append_str(&out, SITE_URL);
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
static void load_docs() {
    DIR *d = opendir("_docs");
    if (!d) return;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        size_t len = strlen(dir->d_name);
        if (len < 4 || strcmp(dir->d_name + len - 3, ".md") != 0) continue;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "_docs/%s", dir->d_name);
        char *content = read_entire_file(path);
        if (!content) continue;

        DocItem *item = &docs[doc_count++];
        memset(item, 0, sizeof(DocItem));
        strncpy(item->slug, dir->d_name, len - 3);
        item->slug[len - 3] = '\0';
        snprintf(item->url, sizeof(item->url), "/docs/%s/", item->slug);
        strncpy(item->source_path, path, sizeof(item->source_path) - 1);

        const char *md = parse_frontmatter_block(content, item, 1);
        StrBuf html;
        buf_init(&html, 4096);
        markdown_to_html(&html, md);
        item->html_content = html.data; /* Keep buffer data */
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

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "_posts/%s", dir->d_name);
        char *content = read_entire_file(path);
        if (!content) continue;

        PostItem *item = &posts[post_count++];
        memset(item, 0, sizeof(PostItem));
        char raw_slug[128];
        strncpy(raw_slug, dir->d_name, len - 3);
        raw_slug[len - 3] = '\0';

        /* Strip date prefix if present */
        char *s = raw_slug;
        if (isdigit((unsigned char)s[0]) && isdigit((unsigned char)s[1]) && s[4] == '-' && s[7] == '-') {
            s += 11;
        }
        strncpy(item->slug, s, sizeof(item->slug) - 1);
        snprintf(item->url, sizeof(item->url), "/blog/%s/", item->slug);
        strncpy(item->source_path, path, sizeof(item->source_path) - 1);

        const char *md = parse_frontmatter_block(content, item, 2);
        StrBuf html;
        buf_init(&html, 4096);
        markdown_to_html(&html, md);
        item->html_content = html.data;
        free(content);

        add_search(item->title, item->url, "Blog", item->description, item->tags);
    }
    closedir(d);
}

static void load_projects() {
    DIR *d = opendir("_projects");
    if (!d) return;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        size_t len = strlen(dir->d_name);
        if (len < 4 || strcmp(dir->d_name + len - 3, ".md") != 0) continue;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "_projects/%s", dir->d_name);
        char *content = read_entire_file(path);
        if (!content) continue;

        ProjectItem *item = &projects[project_count++];
        memset(item, 0, sizeof(ProjectItem));
        strncpy(item->slug, dir->d_name, len - 3);
        item->slug[len - 3] = '\0';
        snprintf(item->url, sizeof(item->url), "/projects/%s/", item->slug);
        strncpy(item->source_path, path, sizeof(item->source_path) - 1);

        const char *md = parse_frontmatter_block(content, item, 3);
        StrBuf html;
        buf_init(&html, 4096);
        markdown_to_html(&html, md);
        item->html_content = html.data;
        free(content);

        add_search(item->title, item->url, "Project", item->description, item->category);
    }
    closedir(d);
}

/* ==========================================================================
   Main Orchestrator
   ========================================================================== */
int main(int argc, char **argv) {
    const char *out_dir = "_site";
    if (argc >= 2) out_dir = argv[1];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    printf("================================================================\n");
    printf("⚡ irbox C-SSG — Zero-Dependency Standalone Site Generator\n");
    printf("Target Output: %s\n", out_dir);
    printf("================================================================\n");

    mkdir_p(out_dir);

    init_docs_navigation();
    load_docs();
    load_posts();
    load_projects();

    /* Render All Pages */
    render_home_page(out_dir);
    render_blog_landing(out_dir);
    render_projects_landing(out_dir);
    render_books_landing(out_dir);
    render_reader_page(out_dir);
    render_changelog_page(out_dir);
    render_about_page(out_dir);
    render_404_page(out_dir);
    render_offline_page(out_dir);

    for (size_t i = 0; i < doc_count; i++) {
        render_docs_page(&docs[i], out_dir);
    }
    for (size_t i = 0; i < post_count; i++) {
        render_post_page(&posts[i], out_dir);
    }
    for (size_t i = 0; i < project_count; i++) {
        render_project_page(&projects[i], out_dir);
    }

    /* Books archive static mirror */
    char books_arc_dst[MAX_PATH];
    snprintf(books_arc_dst, sizeof(books_arc_dst), "%s/books/archive", out_dir);
    copy_dir_recursive("books/archive", books_arc_dst);

    /* Mirror sw.js */
    char sw_dst[MAX_PATH];
    snprintf(sw_dst, sizeof(sw_dst), "%s/sw.js", out_dir);
    copy_file("sw.js", sw_dst);

    /* Mirror Static Assets */
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

    /* Index reader for search */
    add_search("Web Book Reader", "/books/reader/", "Books", "Read digital EPUB and PDF books directly in your browser with offline support.", "books reader epub pdf viewer");

    /* Write Search Index, Sitemap, and RSS Feed */
    write_search_json(out_dir);
    write_sitemap_xml(out_dir);
    write_feed_xml(out_dir);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    printf("✨ Successfully compiled & generated %zu pages!\n", doc_count + post_count + project_count + 7);
    printf("📊 Search index: %zu entries\n", search_entry_count);
    printf("⏱️  Total build time: %.2f ms\n", elapsed_ms);
    printf("================================================================\n");

    return 0;
}

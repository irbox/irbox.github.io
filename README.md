# Folio — Jekyll portfolio, docs, projects & blog

A Jekyll port of the original Next.js docsite, rebuilt as a single showcase site.

| Section | Source | URL |
| --- | --- | --- |
| Home | `index.html` | `/` |
| Docs | `_docs/` + `_data/docs_nav.yml` | `/docs/…` |
| Projects (was *Components*) | `_projects/` | `/projects/`, `/projects/<name>/` |
| Blog | `_posts/` | `/blog/`, `/blog/<slug>/` |
| Changelog | `_data/changelog.yml` | `/changelog/` |
| About | `about.md` | `/about/` |

Removed from the original: Templates, Themes, Playground, Storybook, Sandbox, the MCP/llms routes and all Next/StyleX/Astryx tooling.

## Run it

```bash
bundle install
bundle exec jekyll serve --livereload   # http://localhost:4000
```

## Announcement banner (replaces the Canary warning)

In `_config.yml`:

```yaml
announcement:
  enabled: true      # false = not rendered at all
  id: "2026-10-welcome"   # change to re-show after visitors dismissed it
  status: warning    # info | warning | success | danger
  title: "…"
  message: "…"
  dismissible: true
  link: { label: "…", url: /changelog/ }
```

Per-page opt-out: `announcement: false` in front matter.

## Adding content

- **Doc:** add `_docs/foo.md`, then list `foo` in `_data/docs_nav.yml`.
- **Project:** add `_projects/foo.md` (see `_docs/writing-projects.md`). Its `category` must match `project_categories` in `_config.yml`.
- **Post:** add `_posts/YYYY-MM-DD-title.md`.

## Placeholders to replace

- `assets/images/placeholders/*.svg`: used automatically wherever a `thumbnail` / `cover` is omitted.
- `assets/images/brand-icon.svg`, `favicon.svg`, `og-image.svg` (use a 1200×630 PNG/JPG for social cards).
- All names, URLs and text in `_config.yml`, `_data/*`, `about.md` and the sample content.

## Layout map

```
_config.yml            site settings, banner, collections
_data/                 navigation, docs sidebar, social, authors, changelog
_layouts/              default, page, docs, project, post
_includes/             header, footer, announcement, sidebar, cards, icons, search
_docs/ _projects/ _posts/
assets/css/main.css    tokens + all styles (light/dark via light-dark())
assets/js/main.js      theme, menu, outline, tabs, filters, search (no dependencies)
```

## Notes

- Built to work on GitHub Pages' plugin allow-list (`jekyll-feed`, `-seo-tag`, `-sitemap`, `-paginate`).
- Dark mode uses CSS `light-dark()` (Chrome/Edge 123+, Safari 17.5+, Firefox 120+).
- Search is client-side over `/search.json`; open it with Ctrl/⌘ + K.

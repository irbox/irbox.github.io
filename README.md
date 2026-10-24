# Folio — Personal Engineering Portfolio, Documentation & Writing

A high-performance, accessible, and privacy-respecting personal showcase site built with Jekyll and modern native web standards.

| Section   | Source                          | URL                               |
| --------- | ------------------------------- | --------------------------------- |
| Home      | `index.html`                    | `/`                               |
| Docs      | `_docs/` + `_data/docs_nav.yml` | `/docs/…`                         |
| Projects  | `_projects/`                    | `/projects/`, `/projects/<name>/` |
| Blog      | `_posts/`                       | `/blog/`, `/blog/<slug>/`         |
| Changelog | `_data/changelog.yml`           | `/changelog/`                     |
| About     | `about.md`                      | `/about/`                         |

## Features & Native Web Architecture

- **Zero Heavy Frameworks & Zero Trackers**: Dependency-free vanilla JavaScript and native modern CSS.
- **Self-Hosted Typography**: High-legibility serif reading typeface (Libron by Nico Verbruggen, SIL OFL 1.1) preloaded in WOFF2 with zero external tracking calls.
- **Native Color Schemes**: Light and dark themes implemented via native CSS `light-dark()` with OS preference matching and instant local persistence.
- **Fast Client-Side Search**: `<dialog>` powered search palette with keyboard navigation (`⌘K` / `Ctrl+K`), live scoring, and fuzzy matches.
- **Keyboard Navigation & Quick Actions**: Press `?` to open the shortcuts cheat sheet; `T` to toggle theme; `H`, `D`, `P`, `B` to jump directly to sections.
- **Progressive Enhancement**: Native CSS Scroll-Driven reading progress, GPU-accelerated View Transitions, safe area insets, and high-fidelity print formatting.
- **Strict Content Security Policy (CSP)**: Hardened headers, zero external third-party cookies or telemetry.

## Local Development

```bash
bundle install
bundle exec jekyll serve --livereload   # http://localhost:4000
```

## Adding Content

- **Documentation**: Add `_docs/your-page.md`, then register the slug in `_data/docs_nav.yml`.
- **Projects**: Add `_projects/your-project.md`. Specify `category`, `tags`, `features`, and `stack`.
- **Blog Posts**: Add `_posts/YYYY-MM-DD-title.md`.

## License

- **Code & Content**: Available under the [MIT License](./LICENSE).
- **Typography**: [Libron](./assets/fonts/libron/OFL.txt) is licensed under the [SIL Open Font License 1.1](http://scripts.sil.org/OFL) by Nico Verbruggen.
- **Icons**: Standard SVG vectors based on the permissive MIT-licensed Feather and Lucide icons.

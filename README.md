# Irbox — Personal Engineering Showcase, Books Archive & Writing

A high-performance, accessible, and privacy-respecting personal showcase site powered by a standalone zero-dependency C99 static site generator and modern native web standards.

| Section         | Source                      | URL                               |
| --------------- | --------------------------- | --------------------------------- |
| Home            | `index.html`                | `/`                               |
| Docs            | `_docs/`                    | `/docs/…`                         |
| Projects        | `_projects/`                | `/projects/`, `/projects/<name>/` |
| Blog            | `_posts/`                   | `/blog/`, `/blog/<slug>/`         |
| Books & Archive | `books/` + `books/archive/` | `/books/`, `/books/reader/`       |
| Changelog       | `changelog/`                | `/changelog/`                     |
| About           | `about.md`                  | `/about/`                         |

## Features & Native Web Architecture

- **Zero-Dependency C99 SSG**: Standalone C compiler pipeline (`tools/ssg.c`), compiling in ~150ms and rendering the entire site in < 20ms. Completely replaces Jekyll, Ruby, and Bundler.
- **Zero Heavy Frameworks & Zero Trackers**: Dependency-free vanilla JavaScript and native modern CSS.
- **Personal Book Archive & In-Browser Reader**: Universal EPUB & PDF reading engine with themes, typography scaling, and offline IndexedDB support.
- **Self-Hosted Typography**: High-legibility serif reading typeface (Libron by Nico Verbruggen, SIL OFL 1.1) preloaded in WOFF2 with zero external tracking calls.
- **Native Color Schemes**: Light and dark themes implemented via native CSS `light-dark()` with OS preference matching and instant local persistence.
- **Fast Client-Side Search**: `<dialog>` powered search palette with keyboard navigation (`⌘K` / `Ctrl+K`), live scoring, and fuzzy matches.
- **Keyboard Navigation & Quick Actions**: Press `?` to open the shortcuts cheat sheet; `T` to toggle theme; `H`, `D`, `P`, `B`, `L` to jump directly to sections.
- **Progressive Enhancement**: Native CSS Scroll-Driven reading progress, GPU-accelerated View Transitions, safe area insets, and high-fidelity print formatting.
- **Strict Content Security Policy (CSP)**: Hardened headers, zero external third-party cookies or telemetry.

## Local Development

```bash
make build              # Compiles tools/ssg.c and renders _site/ in < 20ms
```

## Production Build & Optimization

The site features an optimization and pre-compression pipeline for maximum wire-transfer efficiency:

```bash
npm run build           # Full production build: C SSG + Minification + Brotli/Gzip pre-compression
npm run build:clean     # Clean rebuild from scratch
npm run format          # Prettier code formatting
npm run check           # Formatting style validation
```

### Optimization Pipeline Features

- **Extreme HTML Minification**: Full whitespace collapse, comment stripping, attribute quote removal, boolean reduction, and inline CSS & JS compression.
- **Level 2 CSS Minification**: AST-level restructuring, rule merging, duplicate declaration elimination, and color/selector compaction via CleanCSS.
- **SVG Vector Optimization**: Multi-pass SVGO optimization preserving responsive `viewBox` definitions.
- **Data & Feed Compaction**: Zero-whitespace minification for `search.json`, `manifest.webmanifest`, `feed.xml`, and `sitemap.xml`.
- **Pre-Compression for Maximum Efficiency**: Generates both Brotli (`.br`, quality 11) and Gzip (`.gz`, level 9) pre-compressed assets alongside original files for instant, zero-CPU wire delivery on modern web servers and CDNs (~78% wire transfer reduction).

## Adding Content

- **Documentation**: Add `_docs/your-page.md`.
- **Projects**: Add `_projects/your-project.md`. Specify `category`, `tags`, `features`, and `stack`.
- **Blog Posts**: Add `_posts/YYYY-MM-DD-title.md`.
- **Books**: Add `.epub` or `.pdf` to `books/archive/` and run `npm run index:books`.

## License

- **Code & Content**: Available under the [MIT License](./LICENSE).
- **Typography**: [Libron](./assets/fonts/libron/OFL.txt) is licensed under the [SIL Open Font License 1.1](http://scripts.sil.org/OFL) by Nico Verbruggen.
- **Icons**: Standard SVG vectors based on the permissive MIT-licensed Feather and Lucide icons.

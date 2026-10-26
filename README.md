# Irbox — Personal Engineering Showcase, Books Archive & Writing

A high-performance, accessible, and privacy-respecting personal showcase site built with Jekyll and modern native web standards.

| Section         | Source                          | URL                               |
| --------------- | ------------------------------- | --------------------------------- |
| Home            | `index.html`                    | `/`                               |
| Docs            | `_docs/` + `_data/docs_nav.yml` | `/docs/…`                         |
| Projects        | `_projects/`                    | `/projects/`, `/projects/<name>/` |
| Blog            | `_posts/`                       | `/blog/`, `/blog/<slug>/`         |
| Books & Archive | `books/` + `books/archive/`     | `/books/`, `/books/reader/`       |
| Changelog       | `_data/changelog.yml`           | `/changelog/`                     |
| About           | `about.md`                      | `/about/`                         |

## Features & Native Web Architecture

- **Zero Heavy Frameworks & Zero Trackers**: Dependency-free vanilla JavaScript and native modern CSS.
- **Personal Book Archive & In-Browser Reader**: Universal EPUB & PDF reading engine with themes, typography scaling, and offline support.
- **Self-Hosted Typography**: High-legibility serif reading typeface (Libron by Nico Verbruggen, SIL OFL 1.1) preloaded in WOFF2 with zero external tracking calls.
- **Native Color Schemes**: Light and dark themes implemented via native CSS `light-dark()` with OS preference matching and instant local persistence.
- **Fast Client-Side Search**: `<dialog>` powered search palette with keyboard navigation (`⌘K` / `Ctrl+K`), live scoring, and fuzzy matches.
- **Keyboard Navigation & Quick Actions**: Press `?` to open the shortcuts cheat sheet; `T` to toggle theme; `H`, `D`, `P`, `B`, `L` to jump directly to sections.
- **Progressive Enhancement**: Native CSS Scroll-Driven reading progress, GPU-accelerated View Transitions, safe area insets, and high-fidelity print formatting.
- **Strict Content Security Policy (CSP)**: Hardened headers, zero external third-party cookies or telemetry.

## Local Development

```bash
bundle install
npm install
bundle exec jekyll serve --livereload   # http://localhost:4000
```

## Production Build & Extreme Optimization

The site features an extreme obfuscation and minification pipeline designed for maximum runtime security and wire-transfer efficiency:

```bash
# Full production build: Jekyll compile + Extreme Obfuscation + Extreme Minification + Brotli/Gzip pre-compression
npm run build:prod

# Or run Jekyll build followed by the optimizer directly:
npm run build

# Standalone optimization runs:
npm run optimize      # Optimize and obfuscate all generated assets in _site/
npm run obfuscate     # Obfuscate all JavaScript files with cryptographic Base64 string arrays and control flow flattening
npm run minify        # Minify HTML, CSS, JavaScript, SVG, and data manifests
```

### Optimization Pipeline Features

- **Extreme JavaScript Obfuscation**: Cryptographic Base64 string array encoding, runtime rotation, control-flow flattening via switch-dispatch state machines, hexadecimal identifier mangling, split strings, and number-to-expression transforms. Fully compliant with strict Content Security Policy (`CSP`).
- **Extreme HTML Minification**: Full whitespace collapse, comment stripping, attribute quote removal, boolean reduction, and inline CSS & JS compression. Email addresses (`mailto:`) are entity-encoded against scrapers.
- **Level 2 CSS Minification**: AST-level restructuring, rule merging, duplicate declaration elimination, and color/selector compaction via CleanCSS.
- **SVG Vector Optimization**: Multi-pass SVGO optimization preserving responsive `viewBox` definitions.
- **Data & Feed Compaction**: Zero-whitespace minification for `search.json`, `manifest.webmanifest`, `feed.xml`, and `sitemap.xml`.
- **Pre-Compression for Maximum Efficiency**: Generates both Brotli (`.br`, quality 11) and Gzip (`.gz`, level 9) pre-compressed assets alongside original files for instant, zero-CPU wire delivery on modern web servers and CDNs (~78% wire transfer reduction).

## Adding Content

- **Documentation**: Add `_docs/your-page.md`, then register the slug in `_data/docs_nav.yml`.
- **Projects**: Add `_projects/your-project.md`. Specify `category`, `tags`, `features`, and `stack`.
- **Blog Posts**: Add `_posts/YYYY-MM-DD-title.md`.

## License

- **Code & Content**: Available under the [MIT License](./LICENSE).
- **Typography**: [Libron](./assets/fonts/libron/OFL.txt) is licensed under the [SIL Open Font License 1.1](http://scripts.sil.org/OFL) by Nico Verbruggen.
- **Icons**: Standard SVG vectors based on the permissive MIT-licensed Feather and Lucide icons.

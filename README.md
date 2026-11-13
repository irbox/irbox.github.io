# Irbox — Personal Engineering Showcase, Books Archive & Writing

A high-performance, accessible, and privacy-respecting personal showcase site powered by a standalone zero-dependency ISO C23 static site generator and modern native web standards.

| Section         | Source Path                         | URL Route                         |
| --------------- | ----------------------------------- | --------------------------------- |
| Home            | [`tools/ssg.c`](./tools/ssg.c)      | `/`                               |
| Documentation   | [`_docs/`](./_docs/)                | `/docs/…`                         |
| Projects        | [`_projects/`](./_projects/)        | `/projects/`, `/projects/<slug>/` |
| Blog            | [`_posts/`](./_posts/)              | `/blog/`, `/blog/<slug>/`         |
| Books & Reader  | [`books/`](./books/)                | `/books/`, `/books/reader/`       |
| Changelog       | [`tools/ssg.c`](./tools/ssg.c)      | `/changelog/`                     |
| About           | [`about.md`](./about.md)            | `/about/`                         |

## Features & Architecture

- **eNature Ergonomics & Positive Contrast**: Calibrated to retinal neuroscience standards (16.48:1 contrast baseline protecting against blur-induced axial elongation). Applies out-of-the-box with **eNature Dark** as default and a clean 2-way toggle for **eNature Light**.
- **Zero-Dependency ISO C23 Engine**: Standalone C generator (`tools/ssg.c`) compiling and rendering 35+ pages with full-text search in < 400ms without Node, Ruby, or external package runtimes.
- **In-Browser Book Archive & Reader**: Client-side EPUB and PDF reading environment with offline IndexedDB storage, font controls, and reading progress.
- **Self-Hosted Typography**: High-legibility serif typeface (Libron, SIL OFL 1.1) preloaded via local WOFF2 assets with zero third-party telemetry.
- **Instant Search & Command Palette**: Native `<dialog>` modal with fuzzy indexing, keyboard navigation (`⌘K` / `Ctrl+K`), and live search scoring.
- **Accessible Hotkeys**: Press `?` for keyboard shortcuts cheat sheet, `T` to toggle eNature theme (dark / light), and `H`, `D`, `P`, `B` to jump directly to sections.
- **Strict Hardened Security**: Zero tracking cookies, zero external analytics, and strict Content Security Policy (CSP) headers.

## Building & Verification

Compile and run via POSIX `make`:

```bash
make check              # Syntax & pedantic checks under -std=c2x -Wall -Wextra -Wpedantic
make build              # Builds site binary and renders static output into _site/
make test               # Runs end-to-end site generation and verification tests
make clean              # Removes generated binary and _site/ directory
```

Direct compilation without make:

```bash
cc -O3 -Wall -Wextra -pedantic -std=c2x tools/ssg.c -o ssg
./ssg _site
```

## Adding Content

- **Documentation**: Add Markdown files into [`_docs/`](./_docs/).
- **Projects**: Add Markdown files into [`_projects/`](./_projects/) with `category`, `status`, `year`, `tags`, `features`, and `stack` frontmatter.
- **Blog Posts**: Add Markdown files named `YYYY-MM-DD-title.md` into [`_posts/`](./_posts/).
- **Books**: Place `.epub` or `.pdf` files in [`books/archive/`](./books/archive/) and rebuild.

## License

- **Source Code & Content**: [MIT License](./LICENSE)
- **Typography**: [Libron](./assets/fonts/libron/OFL.txt) under SIL Open Font License 1.1
- **Icons**: Permissive MIT-licensed Feather and Lucide SVG vectors

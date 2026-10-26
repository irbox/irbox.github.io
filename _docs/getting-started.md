---
title: Getting started
description: Run this site locally and make it yours in a few minutes.
category: Introduction
---

This site is a zero-dependency static project powered by a standalone modern C23 / C2Y site generator (`tools/ssg.c`). It combines a portfolio, project showcase, documentation, books archive, and blog in a single codebase.

## What's included

- **Docs** in `_docs/`, organized into structured sidebar groups.
- **Projects** in `_projects/`, shown on the [Projects](/projects/) landing page.
- **Blog posts** in `_posts/`.
- **Books & Web Reader** in `books/`, supporting native offline EPUB and PDF reading.
- A configurable **announcement banner**, a **changelog**, light/dark theming and **search**.

## Quick start

```bash
git clone https://github.com/irbox/irbox.github.io.git
cd irbox.github.io
make build
```

Open `_site/index.html` or serve with any local HTTP server. The C SSG compiles and generates the full 30-page site in sub-second speed with zero runtime dependencies.

## Next steps

1. Add your projects to `_projects/`.
2. Write notes and essays in `_posts/` and documentation guides in `_docs/`.
3. Add books to `books/archive/` and rebuild with `make build`.
4. Read [Installation](/docs/installation/) for deploy options.

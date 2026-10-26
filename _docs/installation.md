---
title: Installation
description: Requirements, local setup and deployment.
category: Introduction
---

## Requirements

- C99-compatible compiler (`gcc` or `clang`)
- `make`
- Node.js >= 18 (optional, for asset minification and pre-compression)

## Install and run

Build the entire static site instantly with zero external dependencies:

```bash
make build
```

The C SSG compiles in ~150ms and generates all static HTML pages, search index, sitemap, and RSS feed in under 20ms into `_site/`.

## Build for production (with optimization)

```bash
npm run build
```

This compiles the site using the C SSG, minifies CSS/HTML/JS, optimizes SVGs, and pre-compresses assets with Brotli (.br) and Gzip (.gz).

## Deploy

| Host                      | How                                                                       |
| ------------------------- | ------------------------------------------------------------------------- |
| GitHub Pages              | Automated with GitHub Actions via `.github/workflows/deploy.yml`.         |
| Cloudflare Pages / Vercel | Build command: `make build && node scripts/optimize.js`, output: `_site`. |
| Any static host           | Upload the contents of `_site/`.                                          |

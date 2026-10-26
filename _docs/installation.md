---
title: Installation
description: Requirements, local setup and deployment.
category: Introduction
---

## Requirements

- Modern C23-compatible compiler (`cc`, `gcc`, or `clang` with `-std=c23` support)
- `make` (GNU make or BSD make)

No package managers, runtime interpreters, or external libraries required.

## Install and run

Build the entire static site instantly with zero external dependencies:

```bash
make build
```

The C SSG compiles in ~150ms and generates all static HTML pages, search index, sitemap, and RSS feed in under 20ms into `_site/`.

## Deploy

| Host                      | How                                                               |
| ------------------------- | ----------------------------------------------------------------- |
| GitHub Pages              | Automated with GitHub Actions via `.github/workflows/deploy.yml`. |
| Cloudflare Pages / Vercel | Build command: `make build`, output: `_site`.                     |
| Any static host           | Upload the contents of `_site/`.                                  |

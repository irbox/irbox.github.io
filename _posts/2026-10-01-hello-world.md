---
title: 'Building Irbox: Why I wrote a standalone C23 site generator'
description: 'Moving away from heavy JavaScript toolchains and Ruby gems toward a fast, zero-dependency C23 static site generator.'
tags: [Engineering, Architecture, C23]
cover_alt: 'Irbox architectural overview'
related_docs:
  - { title: Getting started, href: /docs/getting-started/ }
  - { title: Configuration, href: /docs/configuration/ }
---

Every few years, personal websites seem to accumulate layers of dependency rot. A framework upgrade breaks your plugin system, an unmaintained npm package triggers audit warnings, or a Ruby version mismatch halts your deployment pipeline.

When building Irbox, I wanted to step off that treadmill completely.

## The problem with modern SSG toolchains

Modern static site generators are often heavier than the sites they generate. A simple 30-page personal showcase frequently pulls in hundreds of megabytes of `node_modules` or a Ruby gem environment. When you want to publish a quick update from a different machine or a lightweight container, you first spend ten minutes installing build dependencies.

I wanted something different:

1. **Zero external dependencies**: No Node.js, no Ruby, no Python runtime required.
2. **Sub-second compile and build**: The entire site should compile from scratch and emit all static HTML pages in less than a second.
3. **Standards compliance**: Strict HTML Living Standard, modern native CSS with `light-dark()`, and standard ECMAScript.

## Why C23?

C23 brings welcome modernizations to ISO C—native `bool`, `nullptr`, attributes like `[[nodiscard]]`, and safer string primitives.

The generator ([`tools/ssg.c`](/docs/getting-started/)) is a single C file under 3,500 lines. It includes a custom key = value parser for configuration, a streaming Markdown parser with GitHub Flavored Markdown table and inline HTML support, and a search index generator.

```bash
# Compile and build the entire site
cc -O3 -Wall -Wextra -std=c23 tools/ssg.c -o ssg
./ssg _site
```

The C binary compiles in ~150 milliseconds and generates all 33 pages, search index, sitemap, and RSS feeds in roughly 500 milliseconds.

## What to expect here

This site serves three purposes:

- **[Projects](/projects/)**: Selected software projects, libraries, and developer tools with source code and technical write-ups.
- **[Documentation](/docs/getting-started/)**: Guides, architecture notes, and configuration references.
- **[Books & Archive](/books/)**: A digital library shelf with an in-browser EPUB and PDF reader that works offline.

Thank you for visiting. All source code is openly hosted on [GitHub](https://github.com/irbox/irbox.github.io).

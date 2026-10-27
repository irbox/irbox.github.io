---
title: FAQ
description: Common questions about Irbox and the C23 static site generator.
category: Reference
---

## How does the C23 static generator work?

The site is built by [`tools/ssg.c`](/docs/getting-started/), a single-file C23 engine with zero external dependencies. Running `make build` compiles `ssg.c` and generates all HTML pages, search indexes, feeds, and sitemaps in under a second.

## Why isn't my page in the sidebar?

Documentation pages are grouped by their front matter `category`. Projects appear automatically when their `category` matches one of the labels in `_config.cfg`.

## Why are my links broken on GitHub Pages?

If your site is served from a project subdirectory (e.g. `username.github.io/my-repo`), set `baseurl = /my-repo` in `_config.cfg` and rebuild.

## How do I add books to the reader shelf?

Drop any `.epub` or `.pdf` file into `books/archive/` and run `make build`. The book is immediately indexed and readable in the browser at `/books/reader/`.

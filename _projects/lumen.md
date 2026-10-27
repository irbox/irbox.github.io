---
title: Lumen
description: A fast static-site generator with a friendly CLI.
category: Web Apps
status: Stable
year: 2025
role: Creator & maintainer
featured: true
order: 2
docs: project-lumen
links:
  repo: https://github.com/irbox/lumen
tags: [Python, CLI, Markdown]
features:
  - Incremental content-hash builds
  - Sub-second local development server
  - Extensible hook architecture
stack:
  - { name: 'Python', purpose: 'Core generator and pipeline' }
  - { name: 'Jinja2', purpose: 'Layout and component templating' }
  - { name: 'TOML', purpose: 'Configuration parsing' }
gallery:
  - { caption: 'Terminal build pipeline' }
  - { caption: 'Documentation output preview' }
  - { caption: 'Configuration file structure' }
---

Lumen is a fast, minimalist static site generator designed for technical documentation, personal blogs, and developer portfolios. It provides instantaneous build times through smart dependency hashing.

## The problem

Modern JavaScript-based static site frameworks frequently suffer from massive dependency trees (`node_modules`), prolonged cold start times, and multi-second rebuild latencies on simple Markdown edits.

## The approach

Lumen is structured as a single standalone CLI package with minimal dependencies.

- **Content Hashing**: Lumen tracks cryptographic hashes of source Markdown documents and Jinja templates, rebuilding only files whose content or parent layout has actually changed.
- **Single TOML Configuration**: All site options, navigation menus, and build targets are consolidated in a concise, easily readable `lumen.toml` file.
- **Zero-Config Development**: Built-in HTTP server with Server-Sent Events (SSE) live-reload automatically refreshes browsers upon file modification without requiring third-party devtools.

```bash
# Install Lumen and build your documentation
pip install lumen-ssg
lumen init my-docs
lumen build --watch
```

## Outcome

Builds completing in under 40 milliseconds for standard documentation projects. Lumen eliminates toolchain fatigue and gives developers a dependable, zero-friction writing environment.

---
title: Lumen
description: CLI commands, configuration options, and templating reference for Lumen.
category: My projects
---

[Lumen](/projects/lumen/) is a high-speed static site generator for technical documentation and developer blogs. This guide details CLI usage, configuration parameters, and template extensions.

## Installation

Lumen requires Python 3.10 or later:

```bash
pip install lumen-ssg
```

Verify your installation:

```bash
lumen --version
```

## CLI commands

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `lumen init` | `[directory]` | Scaffolds a new Lumen site with default templates and assets. |
| `lumen build` | `[--out dist]` | Performs a complete static compilation into the output directory. |
| `lumen dev` | `[--port 8000]` | Starts local HTTP development server with instant live-reload. |
| `lumen check` | — | Validates internal links, asset paths, and front-matter schemas. |

## Configuration reference (`lumen.toml`)

All project options are managed in a single `lumen.toml` file in your repository root:

```toml
[site]
title = "Engineering Handbook"
tagline = "System architecture and team notes"
url = "https://example.org"
output = "_site"

[markdown]
tables = true
code_highlighting = true
smart_punctuation = true

[build]
clean_urls = true
minify_html = false
cache_dir = ".lumen-cache"
```

## Custom template hooks

Lumen supports custom Python filter functions in a local `lumen_plugins.py` file:

```python
# lumen_plugins.py
def register_filters(env):
    @env.filter
    def read_time(content):
        words = len(content.split())
        return f"{max(1, words // 200)} min read"
```

These filters become immediately accessible in your Jinja templates:

```html
<span class="read-time">{{ post.content | read_time }}</span>
```

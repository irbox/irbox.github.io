---
title: Installation
description: Requirements, local setup and deployment.
category: Introduction
---
## Requirements

- Ruby 3.0 or newer
- Bundler (`gem install bundler`)

## Install and run

```bash
bundle install
bundle exec jekyll serve
```

## Build for production

```bash
JEKYLL_ENV=production bundle exec jekyll build
```

The generated site is written to `_site/`.

## Deploy

| Host | How |
| --- | --- |
| GitHub Pages | Push to GitHub, then enable Pages (GitHub Actions or branch deploy). Set `url` and `baseurl` in `_config.yml`. |
| Netlify / Cloudflare Pages | Build command `bundle exec jekyll build`, publish directory `_site`. |
| Any static host | Upload the contents of `_site/`. |

> **Project pages:** if the site lives at `username.github.io/repo`, set `baseurl: "/repo"`. All internal links use `relative_url`, so they keep working.

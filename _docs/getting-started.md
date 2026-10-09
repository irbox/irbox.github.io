---
title: Getting started
description: Run this site locally and make it yours in a few minutes.
category: Introduction
---
This site is a static [Jekyll](https://jekyllrb.com) project. It combines a portfolio, project showcase, documentation and blog in a single codebase.

## What's included

- **Docs** in `_docs/`, listed in the sidebar by `_data/docs_nav.yml`.
- **Projects** in `_projects/`, shown on the [Projects]({{ site.baseurl }}/projects/) landing page.
- **Blog posts** in `_posts/`.
- A configurable **announcement banner**, a **changelog**, light/dark theming and **search**.

## Quick start

```bash
git clone https://github.com/your-username/your-repo.git
cd your-repo
bundle install
bundle exec jekyll serve --livereload
```

Open <http://localhost:4000>. Edits to content, layouts and CSS reload automatically; changes to `_config.yml` need a restart.

## Next steps

1. Edit `_config.yml`: title, author, URL, social links.
2. Replace the sample [projects]({{ site.baseurl }}/projects/) and [posts]({{ site.baseurl }}/blog/) with your own.
3. Swap the placeholder images in `assets/images/`.
4. Read [Installation]({{ site.baseurl }}/docs/installation/) for deploy options.

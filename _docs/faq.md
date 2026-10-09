---
title: FAQ
description: Common questions.
category: Reference
---
## Why isn't my page in the sidebar?

Docs only appear if listed in `_data/docs_nav.yml`. Projects appear automatically if their `category` matches `project_categories`.

## Why are my links broken on GitHub Pages?

Set `baseurl` to your repository name (for example `"/my-repo"`) when the site is served from a project page.

## How do I replace the placeholder images?

Drop your images into `assets/images/` and point `thumbnail`, `cover` or `gallery.src` at them. Anything omitted falls back to a generated placeholder.

## How do I remove the search?

Delete `_includes/search-dialog.html` and its `include` in `_layouts/default.html`, and remove the search button from `_includes/header.html`.

---
title: Front matter reference
description: Every front matter field the layouts understand.
category: Reference
---
## Pages and docs

| Field | Used by | Purpose |
| --- | --- | --- |
| `title` | all | Page title and search entry. |
| `description` | all | Lead paragraph, SEO description, search. |
| `category` | docs | Small label above the title. |
| `announcement` | all | Set to `false` to hide the banner on this page. |

## Projects

| Field | Purpose |
| --- | --- |
| `category` | Must match an entry in `project_categories`. |
| `status`, `year`, `role` | Facts shown under the title. |
| `featured` | Show on the home page. |
| `order` | Sort order within a category. |
| `thumbnail` | Card and header image. Falls back to a placeholder. |
| `docs` | Slug of a doc page; adds a Documentation button. |
| `links.repo`, `links.demo` | Action buttons. |
| `features`, `stack`, `gallery`, `tags` | Overview list, tech stack tab, gallery tab, badges. |

## Posts

| Field | Purpose |
| --- | --- |
| `tags` | Badges on cards and the post header. |
| `cover`, `cover_alt` | Cover image. |
| `author` | Key from `_data/authors.yml`. |
| `related_docs` | List of `title` / `href` shown after the post. |

---
title: Writing projects
description: Add a project to the showcase.
category: Writing content
---

Each project is a Markdown file in `_projects/`. The front matter drives the card, the sidebar entry, and the detail page:

```yaml
---
title: Atlas
description: Short summary shown on cards.
category: Web Apps # matches project_categories in _config.cfg
status: Active
year: 2026
role: Design & engineering
featured: true # show on the home page
order: 1 # sort order inside the category
docs: project-atlas # optional; slug of a page in _docs/ to link to
links:
  repo: https://github.com/irbox/atlas
tags: [React, TypeScript]
features:
  - Offline-first sync
stack:
  - { name: React, purpose: UI }
gallery:
  - { caption: Dashboard }
---
```

The Markdown body becomes the project overview. The `stack` and `gallery` fields automatically generate dedicated tabs; omitting either cleanly hides that section.

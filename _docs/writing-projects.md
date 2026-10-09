---
title: Writing projects
description: Add a project to the showcase.
category: Writing content
---
Each project is a Markdown file in `_projects/`. The front matter drives the card, the sidebar entry and the detail page.

```yaml
---
title: Atlas
description: Short summary shown on cards.
category: Web Apps        # must match project_categories in _config.yml
status: Active
year: 2026
role: Design & engineering
featured: true           # show on the home page
order: 1                 # sort order inside the category
thumbnail: /assets/images/atlas.png   # optional; a placeholder is used if omitted
docs: project-atlas      # optional; slug of a page in _docs/ to link to
links:
  repo: https://github.com/you/atlas
  demo: https://atlas.example.com
tags: [React, TypeScript]
features:
  - Offline-first sync
stack:
  - { name: React, purpose: UI }
gallery:
  - { src: /assets/images/atlas-1.png, caption: Dashboard }
---
```

The Markdown body becomes the **Overview** tab. `stack` fills the **Tech stack** tab and `gallery` the **Gallery** tab; omit either and the tab disappears.

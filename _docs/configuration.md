---
title: Configuration
description: The settings you will touch most in _config.yml.
category: Introduction
---
Everything site-wide lives in `_config.yml`.

## Identity

```yaml
title: Folio
tagline: Portfolio, docs, projects and writing in one place.
url: "https://example.github.io"
author:
  name: Your Name
  role: Software Engineer
```

## Navigation and social links

Edit the data files instead of templates:

- `_data/navigation.yml` for the top bar, mobile drawer and footer
- `_data/social.yml` for footer icons
- `_data/docs_nav.yml` for the docs sidebar and prev/next links

## Project categories

```yaml
project_categories:
  - Web Apps
  - Libraries
  - Tools & CLI
  - Experiments
```

The order here is the order of sections on the Projects page and in the sidebar. A project's `category` must match one of these names.

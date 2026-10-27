---
title: Configuration
description: The settings you will touch most in _config.cfg.
category: Introduction
---

Everything site-wide lives in `_config.cfg` using a clean, zero-dependency C23 `key = value` configuration format.

## Site Identity & Author

```ini
# Site Identity
title = Irbox
tagline = Portfolio, docs, projects and writing in one place.
description = A personal showcase site: selected projects, documentation for the things I build, and a blog about how I build them.
url = https://irbox.github.io
baseurl = 
repo = https://github.com/irbox/irbox.github.io
repo_branch = main

# Author Profile
author_name = Irbox
author_role = Software Engineer
author_email = 189630825+irbox@users.noreply.github.com
author_avatar = /assets/images/placeholders/avatar.svg
author_bio = Software engineer and open-source developer.
```

## Project categories

```ini
# Project Showcase Categories
project_categories = Web Apps, Libraries, Tools & CLI, Experiments
```

The order here dictates the display order of categories on `/projects/` and within the project navigation sidebar. A project's `category` in front matter matches one of these labels.

## Announcement banner

```ini
# Announcement Banner
announcement_enabled = false
announcement_id = 2026-10-welcome
announcement_status = warning
announcement_title = You're viewing a work-in-progress site
announcement_message = Some pages are still being written and may change.
announcement_link_label = Read the changelog
announcement_link_url = /changelog/
```

After modifying `_config.cfg`, run `make build` to regenerate the site instantly.

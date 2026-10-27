---
title: Announcement banner
description: Show or hide a site-wide notice from _config.cfg.
category: Customizing
---

The banner is controlled entirely from `_config.cfg`:

```ini
# Announcement Banner
announcement_enabled = true
announcement_id = 2026-10-welcome
announcement_status = warning
announcement_title = You're viewing a work-in-progress site
announcement_message = Some pages are still being written.
announcement_link_label = Read the changelog
announcement_link_url = /changelog/
```

| Key | Effect |
| :--- | :--- |
| `announcement_enabled` | `true` renders the banner on every page, `false` disables it. |
| `announcement_id` | Dismissals are stored in `localStorage` per `id`. Change it to prompt visitors on new announcements. |
| `announcement_status` | Visual styling variant (`info`, `warning`, `success`, `danger`). |
| `announcement_link_url` | Optional action button URL. |

To hide the banner on a specific page, add `announcement: false` to that document's front matter. Run `make build` after changing `_config.cfg`.

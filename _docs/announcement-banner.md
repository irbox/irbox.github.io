---
title: Announcement banner
description: Show or hide a site-wide notice from _config.yml.
category: Customizing
---
The banner is controlled entirely from `_config.yml`:

```yaml
announcement:
  enabled: true
  id: "2026-10-welcome"
  status: warning        # info | warning | success | danger
  title: "You're viewing a work-in-progress site"
  message: "Some pages are still being written."
  dismissible: true
  link:
    label: "Read the changelog"
    url: /changelog/
```

| Key | Effect |
| --- | --- |
| `enabled` | `true` shows the banner on every page, `false` removes it from the HTML entirely. |
| `id` | Dismissals are remembered per `id`. Change it to show the banner again to people who closed an earlier one. |
| `dismissible` | Adds a close button. |
| `link` | Optional button. Delete the block to hide it. |

To hide the banner on one page, add `announcement: false` to that page's front matter. Rebuild after changing `_config.yml`.

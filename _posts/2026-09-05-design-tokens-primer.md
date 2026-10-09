---
title: "A short primer on design tokens"
description: "What design tokens are and why light/dark pairs make theming simple."
tags: [Design systems, CSS]
cover_alt: Placeholder cover image
related_docs:
  - { title: Getting started, href: /docs/getting-started/ }
---

Design tokens are named values for color, spacing and type.

## Light/dark pairs

Modern CSS lets a single token carry both themes:

```css
:root {
  color-scheme: light dark;
  --bg: light-dark(#ffffff, #111112);
}
```

No JavaScript is needed to switch values; the browser resolves them from `color-scheme`.

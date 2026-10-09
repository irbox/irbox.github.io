---
title: Theming
description: Colors, fonts and light/dark mode.
category: Customizing
---
All design tokens are CSS custom properties at the top of `assets/css/main.css`.

```css
:root {
  --bg-body: light-dark(#F8F4ED, #111112);
  --ink: light-dark(#15110C, #DFE2E5);
  --brand: light-dark(#225BFF, #3D87FF);
  --radius-container: 16px;
}
```

Each token is a `light-dark()` pair, so dark mode is handled by the value itself. The toggle in the top bar sets `data-theme` on `<html>` and remembers the choice; with no saved choice the site follows the operating system.

## Changing the font

Replace the Google Fonts link in `_includes/head.html` and update `--font-body`. To self-host, drop the link and add `@font-face` rules.

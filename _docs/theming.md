---
title: Theming
description: Colors, fonts and light/dark mode.
category: Customizing
---

All design tokens are CSS custom properties at the top of `assets/css/main.css`.

```css
:root {
  --bg-body: light-dark(#faf8f5, #0a0d12);
  --bg-surface: light-dark(#ffffff, #121620);
  --bg-muted: light-dark(#f3ede4, #0f1218);
  --ink: light-dark(#151413, #f1f3f7);
  --ink-2: light-dark(#565149, #9da4b0);
  --ink-3: light-dark(#706a61, #7e8794);
  --brand: light-dark(#1a56db, #4887f4);
  --radius-container: 16px;
}
```

Each color token is a `light-dark()` pair, so dark mode is handled by the value itself. The toggle in the top bar sets `data-theme` on `<html>` and remembers the choice; with no saved choice the site follows the operating system color scheme.

## Typography

The site uses a dual-typeface typographic system:

- **Headings & Reading Body**: Self-hosted [Libron](https://github.com/nicoverbruggen/libron) (SIL OFL 1.1) in WOFF2 format (`--font-reading`, `--font-heading`).
- **UI Controls & Navigation**: Crisp sans-serif font stack (`--font-sans`).
- **Code & Technical Specs**: Monospace font stack (`--font-mono`).

Fonts are preloaded in `_includes/head.html` with zero external tracking dependencies for maximum speed, privacy, and zero layout shift.

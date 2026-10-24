---
title: Theming
description: Colors, fonts and light/dark mode.
category: Customizing
---

All design tokens are CSS custom properties at the top of `assets/css/main.css`.

```css
:root {
  --bg-body: light-dark(#faf8f5, #0c0e12);
  --bg-surface: light-dark(#ffffff, #14171d);
  --bg-muted: light-dark(#f3eee6, #1c2028);
  --ink: light-dark(#161412, #f0f2f5);
  --brand: light-dark(#1a56db, #4582ec);
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

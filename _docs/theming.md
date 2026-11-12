---
title: Theming
description: Colors, fonts and light/dark mode.
category: Customizing
---

All design tokens are CSS custom properties at the top of `assets/css/main.css`.

```css
:root, [data-theme='light'] {
  --bg-body: #ffffff;
  --bg-surface: #fafafa;
  --bg-muted: #f1f2f4;
  --ink: #111827;
  --ink-2: #4b5563;
  --brand: #2563eb;
  --radius-container: 16px;
}
@media (prefers-color-scheme: dark) {
  :root:not([data-theme='light']) {
    --bg-body: #0b0d11;
    --bg-surface: #12151b;
    --bg-muted: #1c212b;
    --ink: #f9fafb;
    --ink-2: #9ca3af;
    --brand: #3b82f6;
  }
}
```

The toggle in the top bar sets `data-theme` on `<html>` and remembers the choice; with no saved choice the site follows the operating system color scheme.

## Typography

The site uses a dual-typeface typographic system:

- **Headings & Reading Body**: Self-hosted [Libron](https://github.com/nicoverbruggen/libron) (SIL OFL 1.1) in WOFF2 format (`--font-serif`, `--font-heading`).
- **UI Controls & Navigation**: Crisp sans-serif font stack (`--font-sans`).
- **Code & Technical Specs**: Monospace font stack (`--font-mono`).

Fonts are self-hosted with zero external tracking dependencies for maximum speed, privacy, and zero layout shift.

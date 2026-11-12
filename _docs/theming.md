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

The toggle in the top bar cycles through four color modes (`Light` -> `Dark` -> `Nature` -> `Nature Light`) and persists the choice in `localStorage`. With no saved choice the site follows the operating system color scheme. You can also press **`T`** anywhere on the site to cycle themes.

## The Nature Theme System

In addition to standard Light and Dark modes, the site includes **Nature**—a color scheme engineered from retinal neurobiology and myopia prevention research (*Aleman, Wang, & Schaeffel, 2018, Nature Scientific Reports 8:10840*):

- **Nature (`data-theme='nature'`)**: Deep loam substrate (`#111613`) activating the retinal ON pathway (positive contrast polarity) to promote choroidal thickening (+10 µm) and inhibit myopia, with syntax highlighting constrained to a 515–590 nm spectral corridor to eliminate longitudinal chromatic aberration (<0.35 D).
- **Nature Light (`data-theme='nature-light'`)**: Sun-bleached botanical parchment (`#f0f2eb`) designed for workstations next to windows, providing high diffuse reflectance to overpower window specular glare without the blinding choroidal shock (-16 µm) of 100% white.

Downloadable configuration files for editors and terminals are available:
- [VS Code Theme (`assets/themes/nature-vscode.json`)](/assets/themes/nature-vscode.json)
- [Neovim Lua Colorscheme (`assets/themes/nature.nvim`)](/assets/themes/nature.nvim)
- [Alacritty Terminal Theme (`assets/themes/nature-alacritty.toml`)](/assets/themes/nature-alacritty.toml)
- [Web CSS Tokens (`assets/themes/nature.css`)](/assets/themes/nature.css)
- [Machine-Readable Palette Specification (`assets/themes/nature.json`)](/assets/themes/nature.json)

## Typography

The site uses a dual-typeface typographic system:

- **Headings & Reading Body**: Self-hosted [Libron](https://github.com/nicoverbruggen/libron) (SIL OFL 1.1) in WOFF2 format (`--font-serif`, `--font-heading`).
- **UI Controls & Navigation**: Crisp sans-serif font stack (`--font-sans`).
- **Code & Technical Specs**: Monospace font stack (`--font-mono`).

Fonts are self-hosted with zero external tracking dependencies for maximum speed, privacy, and zero layout shift.

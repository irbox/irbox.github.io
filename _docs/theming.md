---
title: Theming
description: Nature color system, design tokens, and light/dark modes.
category: Customizing
---

**Nature** is the default and original theme system for this site. Grounded in retinal neurobiology and myopia prevention research (*Aleman, Wang, & Schaeffel, 2018, Scientific Reports 8:10840*), the design architecture features two scientifically calibrated modes:

- **Nature Dark (`data-theme='dark'`)**: The primary biological baseline. Deep ocular substrate (`#0a0f14`) activating the retinal ON-pathway through positive contrast polarity, promoting choroidal thickening (+10 µm) and inhibiting axial elongation. Syntax highlighting is anchored in Claude Sonnet 5.5 High's low-LCA Choroid palette (`#4fd1c5` teal, `#8fd694` chlorophyll green, `#f29e5c` amber orange, `#f2cc6b` ochre yellow).
- **Nature Light (`data-theme='light'`)**: The daylight and high-ambient reading mode. Sun-bleached botanical parchment (`#f0f2eb`) engineered for workstations next to windows, providing high diffuse reflectance to overpower specular glare without triggering the severe choroidal thinning (-16 µm) of 100% blinding white.

## Design Tokens

All tokens are standard CSS custom properties declared at the root of `assets/css/main.css`:

```css
/* Nature Dark (Universal Biological Baseline) */
:root, [data-theme='dark'] {
  --bg-body: #0a0f14;
  --bg-surface: #101820;
  --bg-surface-raised: #17222c;
  --bg-surface-elevated: #22313d;
  --ink: #e9eef3;
  --ink-2: #b4c0cb;
  --ink-muted: #74889a;
  --border: #2c3c49;
  --brand: #4fd1c5;
}

/* Nature Light (Daylight Glare-Resistant Reading) */
[data-theme='light'] {
  --bg-body: #f0f2eb;
  --bg-surface: #e5e9e0;
  --bg-surface-raised: #dbe0d5;
  --bg-surface-elevated: #ffffff;
  --ink: #19241c;
  --ink-2: #354538;
  --ink-muted: #5a6d5f;
  --border: rgba(25, 36, 28, 0.12);
  --brand: #2d6139;
}
```

The topbar button and the **`T`** keyboard shortcut toggle cleanly between Nature Dark and Nature Light, persisting preference in `localStorage`. Default theme behavior is also configurable in `_config.cfg` via `default_theme = dark`.

## Editor & Terminal Palette Exports

Standalone configuration files are available for download:
- [VS Code Theme (`assets/themes/nature-vscode.json`)](/assets/themes/nature-vscode.json)
- [Neovim Lua Colorscheme (`assets/themes/nature.nvim`)](/assets/themes/nature.nvim)
- [Alacritty Terminal Theme (`assets/themes/nature-alacritty.toml`)](/assets/themes/nature-alacritty.toml)
- [Web CSS Tokens (`assets/themes/nature.css`)](/assets/themes/nature.css)
- [Machine-Readable Palette Specification (`assets/themes/nature.json`)](/assets/themes/nature.json)

## Typography

The site uses an engineered typographic hierarchy:

- **Headings & Reading Body**: Self-hosted [Libron](https://github.com/nicoverbruggen/libron) (SIL OFL 1.1) in WOFF2 format (`--font-serif`, `--font-heading`).
- **UI Controls & Navigation**: Engineered sans-serif font stack (`--font-sans`).
- **Code & Technical Specs**: High-legibility monospace font stack (`--font-mono`).

Pure POSIX design rules are strictly enforced: zero rounded corners (`border-radius: 0 !important;`) and zero blur filters (`backdrop-filter: none !important;`).

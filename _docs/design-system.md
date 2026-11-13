---
title: Design System & UI Architecture
description: Comprehensive implementation guide for the minimalist, zero-dependency design system, typography tokens, vector iconography, and UI components.
category: Customizing
---

A modern, minimalist design system engineered for high-performance static websites. Built entirely with clean semantic HTML, responsive modern CSS, and zero external trackers or proprietary dependencies.

## 1. Typography Suite

The typography architecture uses native high-fidelity system fonts combined with self-hosted open-source editorial serifs. This delivers instantaneous zero-latency rendering with zero external network requests:

| Typeface Stack | Purpose & Scope | Font Stack | License |
| :--- | :--- | :--- | :--- |
| **System UI Sans** | UI labels, navigation, buttons, cards, headings | `system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif` | Native OS / Open |
| **Libron Serif** | Editorial longform, documentation prose | `'Libron', Charter, Georgia, serif` | SIL Open Font License |
| **Code Monospace** | Code blocks, technical specs, terminal logs | `ui-monospace, "Cascadia Code", "Fira Code", Menlo, monospace` | Native OS / Open |

```css
/* Typography tokens in assets/css/main.css */
:root {
  --font-sans: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, -apple-system, sans-serif;
  --font-heading: var(--font-sans);
  --font-serif: 'Libron', Charter, 'Bitstream Charter', 'Sitka Text', Cambria, Georgia, serif;
  --font-mono: ui-monospace, 'SF Mono', 'Cascadia Code', 'Fira Code', Menlo, Monaco, Consolas, monospace;
}
```

## 2. Minimalist Vector Iconography

All icons across the platform use lightweight, inline SVG geometry. With consistent 1.8px to 2px stroke weight, round caps, and responsive sizing, icons adapt seamlessly to color modes and density:

- **Theme Mode**: Ambient sun and moon glyphs for light and dark appearance
- **Search**: Crisp magnifying glass for instant command palette search
- **Navigation**: Clean chevrons for hierarchy and drill-down lists
- **Connectivity**: Resilient offline status badge
- **E-Reader**: Clean book and chapter navigation markers
- **Actions**: Native web share, print layout, and clipboard copy triggers

## 3. UI Component Kit

### Segmented Controls

Versatile, tactile switcher for views, intervals, and filter states:

<div class="segmented-control" role="tablist">
  <button class="segment-btn is-active" type="button" role="tab" aria-selected="true">Daily</button>
  <button class="segment-btn" type="button" role="tab" aria-selected="false">Weekly</button>
  <button class="segment-btn" type="button" role="tab" aria-selected="false">Monthly</button>
  <button class="segment-btn" type="button" role="tab" aria-selected="false">Yearly</button>
</div>

```html
<div class="segmented-control" role="tablist">
  <button class="segment-btn is-active" type="button" role="tab" aria-selected="true">Daily</button>
  <button class="segment-btn" type="button" role="tab" aria-selected="false">Weekly</button>
  <button class="segment-btn" type="button" role="tab" aria-selected="false">Monthly</button>
</div>
```

### Inset Grouped List

Clean, rounded container for key-value data, metadata, and configuration options:

<div class="grouped-list">
  <div class="grouped-item">
    <div class="grouped-item__left">
      <strong>Theme Mode</strong>
    </div>
    <div class="grouped-item__right">
      <span>System Automatic</span>
      <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m9 18 6-6-6-6"/></svg>
    </div>
  </div>
  <div class="grouped-item">
    <div class="grouped-item__left">
      <strong>Typography</strong>
    </div>
    <div class="grouped-item__right">
      <span>System UI + Libron</span>
      <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m9 18 6-6-6-6"/></svg>
    </div>
  </div>
  <div class="grouped-item">
    <div class="grouped-item__left">
      <strong>Generator Pipeline</strong>
    </div>
    <div class="grouped-item__right">
      <span>ISO C23 Standalone</span>
      <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m9 18 6-6-6-6"/></svg>
    </div>
  </div>
</div>

```html
<div class="grouped-list">
  <div class="grouped-item">
    <div class="grouped-item__left"><strong>Label</strong></div>
    <div class="grouped-item__right"><span>Value</span></div>
  </div>
</div>
```

### Semantic Callouts

Structured callouts for notices, tips, and important technical context:

> [!NOTE]
> **System Architecture**: All fonts are self-hosted or native to the operating system, guaranteeing zero external latency, zero third-party tracking, and 100% offline availability.

> [!TIP]
> **Accessibility Standards**: Meets WCAG AA contrast criteria across both Light and Dark modes with automatic OS appearance synchronization.

## 4. Modern Button & Badge System

<div class="project-actions">
  <button class="btn btn--primary" type="button">Primary Button</button>
  <button class="btn btn--secondary" type="button">Secondary Button</button>
  <button class="btn btn--ghost" type="button">Ghost Button</button>
  <span class="badge badge--added">Added</span>
  <span class="badge badge--changed">Changed</span>
  <span class="badge badge--fixed">Fixed</span>
  <span class="badge badge--security">Security</span>
</div>

```html
<button class="btn btn--primary" type="button">Primary</button>
<button class="btn btn--secondary" type="button">Secondary</button>
<button class="btn btn--ghost" type="button">Ghost</button>
<span class="badge badge--added">Added</span>
<span class="badge badge--security">Security</span>
```

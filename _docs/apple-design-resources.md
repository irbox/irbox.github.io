---
title: Apple Design Resources
description: Implementation guide and interactive showcase for Apple Design Resources, typography, SF Symbols, device frames, and HIG components.
category: Customizing
---

[Apple Design Resources](https://developer.apple.com/design/resources/) provides the foundational building blocks for Apple platforms—macOS, iOS, iPadOS, watchOS, and visionOS. This site implements a complete, self-hosted web translation of Apple's Human Interface Guidelines (HIG), typography, iconography, and device bezels.

## 1. Apple Typography Suite

The design system incorporates four distinct Apple type families, hosted locally with zero external network tracking:

| Typeface | Optical Size / Purpose | Weights Bundled |
| :--- | :--- | :--- |
| **SF Pro Display** | Headlines, display banners (`≥ 20pt`), hero metrics | Regular (400), Medium (500), Bold (700) |
| **SF Pro Text** | Body text, microcopy, form controls (`< 20pt`) | Regular (400), Medium (500), Bold (700) |
| **SF Mono** | Terminal blocks, C23 code, technical specifications | Regular (400), Medium (500), Bold (700) |
| **New York** | Editorial serif, long-form reading, book reader | Regular (400), Italic (400), Bold (700) |

```css
/* Typography tokens in assets/css/main.css */
:root {
  --font-sans: 'SF Pro Text', -apple-system, BlinkMacSystemFont, 'SF Pro', system-ui, sans-serif;
  --font-heading: 'SF Pro Display', -apple-system, BlinkMacSystemFont, 'SF Pro', system-ui, sans-serif;
  --font-serif: 'New York', 'Libron', Georgia, 'Times New Roman', serif;
  --font-mono: 'SF Mono', SFMono-Regular, ui-monospace, Menlo, Monaco, Consolas, monospace;
}
```

## 2. SF Symbols Iconography

All site icons have been mapped to Apple's official SF Symbols vector geometry:

- **Share Sheet**: Apple's `square.and.arrow.up`
- **Navigation & Drill-down**: Apple's `chevron.right`
- **Search**: Apple's `magnifyingglass`
- **Appearance Modes**: Apple's `sun.max.fill` and `moon.fill`
- **External Links**: Apple's `arrow.up.right`
- **Connectivity**: Apple's `wifi.slash`
- **Books & Library**: Apple's `book.closed`
- **Hardware & Keyboard**: Apple's `command` / `keyboard`

## 3. Apple Device Mockup Bezels

Authentic device frames allow project screenshots, code output, and book previews to be displayed inside genuine hardware bezels.

### macOS Window & Studio Display Frame

<div class="apple-device-mac">
  <div class="apple-device-mac__titlebar">
    <div class="apple-device-mac__traffic-lights">
      <span class="apple-device-mac__dot apple-device-mac__dot--close"></span>
      <span class="apple-device-mac__dot apple-device-mac__dot--minimize"></span>
      <span class="apple-device-mac__dot apple-device-mac__dot--fullscreen"></span>
    </div>
    <span class="apple-device-mac__title">Terminal — irbox-ssg (C23) — 80×24</span>
  </div>
  <div class="apple-device-mac__body" style="padding: 1.5rem; font-family: var(--font-mono); font-size: 0.875rem;">
    <p style="margin: 0; color: var(--success);">$ make build FAST=1</p>
    <p style="margin: 0.5rem 0 0; color: var(--ink-2);">⚡ irbox C23-SSG — Modern Standalone Site Generator</p>
    <p style="margin: 0.25rem 0 0; color: var(--ink);">⏱️  Total Build Time: 4.43 ms | 32 pages generated</p>
    <p style="margin: 0.25rem 0 0; color: var(--brand);">✨ Status: 0 warnings, ISO C23 verified.</p>
  </div>
</div>

```html
<!-- HTML Structure for macOS Window -->
<div class="apple-device-mac">
  <div class="apple-device-mac__titlebar">
    <div class="apple-device-mac__traffic-lights">
      <span class="apple-device-mac__dot apple-device-mac__dot--close"></span>
      <span class="apple-device-mac__dot apple-device-mac__dot--minimize"></span>
      <span class="apple-device-mac__dot apple-device-mac__dot--fullscreen"></span>
    </div>
    <span class="apple-device-mac__title">App Title</span>
  </div>
  <div class="apple-device-mac__body">
    <!-- Viewport content -->
  </div>
</div>
```

### iPhone Pro with Dynamic Island

<div class="apple-device-iphone">
  <div class="apple-device-iphone__screen">
    <div class="apple-device-iphone__dynamic-island">
      <span class="apple-device-iphone__camera"></span>
    </div>
    <div style="padding: 4rem 1.25rem 2rem; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100%; text-align: center;">
      <div class="apple-icon-squircle" style="margin-bottom: 1rem;">
        <svg xmlns="http://www.w3.org/2000/svg" width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 19.5v-15A2.5 2.5 0 0 1 6.5 2H20v20H6.5a2.5 2.5 0 0 1-2.5-2.5Z"/><path d="M6 6h10M6 10h10"/></svg>
      </div>
      <h3 style="font-size: 1.125rem; font-weight: 600; margin: 0 0 0.25rem;">Apple Books Reader</h3>
      <p style="font-size: 0.8125rem; color: var(--ink-2); margin: 0 0 1.25rem;">Digital typography calibrated with New York &amp; Libron.</p>
      <div class="apple-segmented-control" style="width: 100%; justify-content: center;">
        <span class="apple-segment-item is-active">Library</span>
        <span class="apple-segment-item">Reading</span>
      </div>
    </div>
    <span class="apple-device-iphone__home-bar"></span>
  </div>
</div>

## 4. HIG UI Component Kit

### Segmented Controls

<div style="margin: 1.5rem 0;">
  <div class="apple-segmented-control" role="tablist">
    <button class="apple-segment-item is-active" type="button" role="tab" aria-selected="true">Daily</button>
    <button class="apple-segment-item" type="button" role="tab" aria-selected="false">Weekly</button>
    <button class="apple-segment-item" type="button" role="tab" aria-selected="false">Monthly</button>
    <button class="apple-segment-item" type="button" role="tab" aria-selected="false">Yearly</button>
  </div>
</div>

```html
<div class="apple-segmented-control" role="tablist">
  <button class="apple-segment-item is-active" type="button">Tab 1</button>
  <button class="apple-segment-item" type="button">Tab 2</button>
  <button class="apple-segment-item" type="button">Tab 3</button>
</div>
```

### Inset Grouped Table (Settings Style)

<div class="apple-grouped-list">
  <div class="apple-grouped-item">
    <div class="apple-grouped-item__left">
      <strong>Theme Mode</strong>
    </div>
    <div class="apple-grouped-item__right">
      <span>System Automatic</span>
      <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"><path d="m9 18 6-6-6-6"/></svg>
    </div>
  </div>
  <div class="apple-grouped-item">
    <div class="apple-grouped-item__left">
      <strong>Typography</strong>
    </div>
    <div class="apple-grouped-item__right">
      <span>San Francisco Pro</span>
      <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"><path d="m9 18 6-6-6-6"/></svg>
    </div>
  </div>
  <div class="apple-grouped-item">
    <div class="apple-grouped-item__left">
      <strong>Pipeline Engine</strong>
    </div>
    <div class="apple-grouped-item__right">
      <span>C23 (Sub-second)</span>
      <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"><path d="m9 18 6-6-6-6"/></svg>
    </div>
  </div>
</div>

### Semantic Callout Banners

<div class="apple-callout apple-callout--info">
  <div>
    <strong>System Tip</strong>: All typography files are loaded using native <code>@font-face</code> definitions with <code>font-display: swap</code> for zero layout shift (CLS).
  </div>
</div>

<div class="apple-callout apple-callout--success">
  <div>
    <strong>Verified Standard</strong>: Fully compliant with Apple Human Interface Guidelines and WCAG AA contrast criteria across Light and Dark appearances.
  </div>
</div>

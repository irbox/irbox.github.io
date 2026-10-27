---
title: Apple Design Resources
description: Complete implementation guide and interactive showcase for Apple Design Resources, official typography, SF Symbols, hardware bezels, identity badges, and Apple Intelligence integration.
category: Customizing
---

[Apple Design Resources](https://developer.apple.com/design/resources/) provides the foundational building blocks for Apple platforms—macOS, iOS, iPadOS, watchOS, and visionOS. This site implements a complete, self-hosted web translation of Apple's Human Interface Guidelines (HIG), typography, iconography, official identity marks, hardware bezels, and App Intents integration architecture.

## 1. Apple Typography Suite

The design system incorporates four distinct Apple type families, hosted locally with zero external network tracking:

| Typeface | Optical Size / Purpose | Weights Bundled | Format |
| :--- | :--- | :--- | :--- |
| **SF Pro Display** | Headlines, display banners (`≥ 20pt`), hero metrics | Regular (400), Medium (500), Bold (700) | WOFF / OTF |
| **SF Pro Text** | Body text, microcopy, form controls (`< 20pt`) | Regular (400), Medium (500), Bold (700) | WOFF / OTF |
| **SF Mono** | Terminal blocks, C23 code, technical specifications | Regular (400), Medium (500), Bold (700) | WOFF2 / OTF |
| **New York** | Editorial serif, long-form reading, book reader | Regular (400), Italic (400), Bold (700) | OTF |

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

## 3. Official Identity Badges & Action Buttons

Directly integrated from Apple's official artwork packages:

### Sign in with Apple

Compliant with Apple Human Interface Guidelines button specifications:

<div style="display: flex; gap: 1rem; flex-wrap: wrap; margin: 1.5rem 0;">
  <a class="apple-btn-siwa apple-btn-siwa--black" href="#siwa">
    <img src="/assets/images/apple/siwa-btn-white.svg" alt="Apple logo" width="17" height="20">
    <span>Sign in with Apple</span>
  </a>
  <a class="apple-btn-siwa apple-btn-siwa--white" href="#siwa">
    <img src="/assets/images/apple/siwa-btn-black.svg" alt="Apple logo" width="17" height="20">
    <span>Sign in with Apple</span>
  </a>
  <a class="apple-btn-siwa apple-btn-siwa--black apple-btn-siwa--pill" href="#siwa">
    <img src="/assets/images/apple/siwa-btn-white.svg" alt="Apple logo" width="17" height="20">
    <span>Sign in with Apple</span>
  </a>
</div>

```html
<!-- Sign in with Apple Buttons -->
<a class="apple-btn-siwa apple-btn-siwa--black" href="#">
  <img src="/assets/images/apple/siwa-btn-white.svg" alt="Apple logo">
  <span>Sign in with Apple</span>
</a>
<a class="apple-btn-siwa apple-btn-siwa--white" href="#">
  <img src="/assets/images/apple/siwa-btn-black.svg" alt="Apple logo">
  <span>Sign in with Apple</span>
</a>
```

### Apple Ecosystem Badges

<div style="display: flex; gap: 0.75rem; flex-wrap: wrap; margin: 1.5rem 0;">
  <span class="apple-identity-badge">
    <img src="/assets/images/apple/watch-face-dark.svg" alt="Apple Watch Face">
    <span>Add to Apple Watch</span>
  </span>
  <span class="apple-identity-badge">
    <img src="/assets/images/apple/game-center-black.svg" alt="Game Center">
    <span>Apple Game Center</span>
  </span>
  <span class="apple-identity-badge">
    <img src="/assets/images/apple/siri-orb.png" alt="Siri" style="border-radius: 50%;">
    <span>Apple Siri &amp; Intelligence</span>
  </span>
  <span class="apple-identity-badge">
    <img src="/assets/images/apple/apple-health.png" alt="HealthKit">
    <span>Apple HealthKit</span>
  </span>
  <span class="apple-identity-badge">
    <img src="/assets/images/apple/home-app.png" alt="HomeKit">
    <span>Apple HomeKit</span>
  </span>
</div>

## 4. Official Hardware Device Bezels

Authentic device frames allow project screenshots, code output, and book previews to be displayed inside genuine hardware bezels.

### MacBook Pro M5 Frame

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
    <p style="margin: 0.25rem 0 0; color: var(--ink);">⏱️  Total Build Time: 4.43 ms | 33 pages generated</p>
    <p style="margin: 0.25rem 0 0; color: var(--brand);">✨ Status: 0 warnings, ISO C23 verified.</p>
  </div>
</div>

### iPhone 16 Pro with Dynamic Island

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

### Official Hardware Bezel Images

The repository bundles official transparent PNG bezels extracted from Apple disk images:

- `assets/images/apple/bezels/iphone-16-pro.png` (iPhone 16 Pro Black Titanium)
- `assets/images/apple/bezels/macbook-pro-m5.png` (MacBook Pro M5 Space Black)
- `assets/images/apple/bezels/ipad-pro-m5.png` (iPad Pro M5 Space Black)

```html
<div class="apple-bezel-frame apple-bezel-frame--iphone">
  <img class="apple-bezel-img" src="/assets/images/apple/bezels/iphone-16-pro.png" alt="iPhone 16 Pro Bezel" loading="lazy">
</div>
```

## 5. HIG UI Component Kit

### Segmented Controls

<div style="margin: 1.5rem 0;">
  <div class="apple-segmented-control" role="tablist">
    <button class="apple-segment-item is-active" type="button" role="tab" aria-selected="true">Daily</button>
    <button class="apple-segment-item" type="button" role="tab" aria-selected="false">Weekly</button>
    <button class="apple-segment-item" type="button" role="tab" aria-selected="false">Monthly</button>
    <button class="apple-segment-item" type="button" role="tab" aria-selected="false">Yearly</button>
  </div>
</div>

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

## 6. Apple Intelligence & App Intents Integration Architecture

Based on official Apple Developer reference implementations (`UnicornChat` and the Intelligence Sample Library):

```swift
import AppIntents

// App Intent for Apple Intelligence System Actions
struct SendMessageIntent: AppIntent {
    static var title: LocalizedStringResource = "Send Message"
    static var description = IntentDescription("Sends a message to an existing conversation.")

    @Parameter(title: "Recipient")
    var recipient: ContactEntity

    @Parameter(title: "Message Text")
    var message: String

    static var parameterSummary: some ParameterSummary {
        Summary("Send \(\.$message) to \(\.$recipient)")
    }

    @MainActor
    func perform() async throws -> some IntentResult & ReturnsValue<MessageEntity> {
        let sentMessage = try await MessageManager.shared.sendMessage(message, to: recipient)
        return .result(value: sentMessage)
    }
}
```

### Semantic Callouts

<div class="apple-callout apple-callout--info">
  <div>
    <strong>System Architecture</strong>: All typography files are loaded using native <code>@font-face</code> definitions with <code>font-display: swap</code> for zero cumulative layout shift (CLS).
  </div>
</div>

<div class="apple-callout apple-callout--success">
  <div>
    <strong>Verified Standard</strong>: Fully compliant with Apple Human Interface Guidelines and WCAG AA contrast criteria across Light and Dark appearances.
  </div>
</div>

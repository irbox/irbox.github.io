---
title: Kit
description: A small set of accessible UI primitives.
category: Libraries
status: Stable
year: 2025
role: Author
featured: true
order: 1
links:
  repo: https://github.com/irbox/kit
tags: [TypeScript, A11y, CSS]
features:
  - Zero runtime dependencies
  - WAI-ARIA compliant patterns
  - Tree-shakeable modular imports
stack:
  - { name: 'TypeScript', purpose: 'Strict typed component API' }
  - { name: 'CSS Custom Properties', purpose: 'Native light-dark theming' }
  - { name: 'HTML5 Dialog API', purpose: 'Accessible native modal primitives' }
gallery:
  - { caption: 'Dialog and drawer primitives' }
  - { caption: 'Popover and tooltip positioning' }
  - { caption: 'Keyboard focus management' }
---

Kit is an unstyled, headless collection of accessible UI primitives designed for modern web applications. It provides robust keyboard navigation, focus trapping, and ARIA state management with zero external dependencies.

## The problem

Mainstream UI component libraries often ship with hundreds of kilobytes of runtime code, prescribe rigid CSS styling systems, and fail basic accessibility audits for complex widgets like dialogs, comboboxes, and disclosure panels.

## The approach

Kit leverages native browser primitives rather than re-implementing standard behaviors from scratch.

- **Native Dialog & Popover**: Uses the standard `HTMLDialogElement` and modern Popover API where available, polyfilling only missing behaviors.
- **Focus Management**: Implements keyboard roving-tabindex and circular focus containment using lightweight DOM traversal algorithms.
- **Styling Agnostic**: Ships without bundled CSS rules; components accept plain class names and data attributes that integrate with any styling system or CSS variables.

```typescript
import { createDialog, createTabs } from '@irbox/kit';

const dialog = createDialog('#settings-modal', {
  restoreFocus: true,
  closeOnBackdrop: true,
});

dialog.open();
```

## Outcome

The entire primitive suite weighs under 3.2 KB gzipped. Components score 100% on automated Axe and Lighthouse accessibility audits and provide seamless keyboard navigation across screen readers.

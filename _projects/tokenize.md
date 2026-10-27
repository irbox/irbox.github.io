---
title: Tokenize
description: Design-token tooling that outputs CSS, JSON and Swift.
category: Libraries
status: Experimental
year: 2026
role: Author
featured: false
order: 2
links:
  repo: https://github.com/irbox/tokenize
tags: [Design systems, Node.js, Swift]
features:
  - W3C Design Tokens Community Group (DTCG) specification support
  - Multi-platform outputs for Web (CSS), React (TypeScript), and Apple (Swift)
  - Color contrast validation and mathematical token transforms
stack:
  - { name: 'Node.js', purpose: 'Compiler engine and token resolver' }
  - { name: 'TypeScript', purpose: 'Type generation and schema validation' }
  - { name: 'Style Dictionary', purpose: 'Underlying token transform engine' }
gallery:
  - { caption: 'Token JSON definition format' }
  - { caption: 'Generated CSS variables output' }
  - { caption: 'Swift enum code generation' }
---

Tokenize is a multi-platform design token pipeline that converts centralized design specifications into native variables for CSS, TypeScript, and Swift.

## The problem

Modern cross-platform development requires maintaining identical design variables—colors, spacing, shadows, and typography—across web stylesheets, TypeScript component libraries, and native iOS applications. Manual updates inevitably lead to visual divergence and styling bugs.

## The approach

Tokenize establishes a single source of truth using standard JSON token structures:

- **W3C Format Compliance**: Follows the official Design Tokens Community Group specification with support for `$value`, `$type`, and nested token references.
- **Light/Dark Pair Synthesis**: Automatically generates CSS `light-dark()` functional notations for web targets and semantic `ColorAsset` structures for Xcode / SwiftUI.
- **Automated Validation**: Runs WCAG contrast checks during build time, alerting developers if text and background pairings fail AA or AAA criteria.

```bash
# Compile design tokens across all targets
npx tokenize build --tokens ./tokens.json --out ./dist
```

## Outcome

Eliminates manual synchronization overhead between designers and developers, generating type-safe, zero-runtime assets in milliseconds.

---
title: Shipit
description: A one-command release helper for monorepos.
category: Tools & CLI
status: Active
year: 2024
role: Creator
featured: true
order: 1
links:
  repo: https://github.com/irbox/shipit
tags: [Go, CLI, Git]
features:
  - Semantic versioning calculation from Conventional Commits
  - Automated Markdown changelog generation
  - Non-destructive dry-run mode and GPG tag signing
stack:
  - { name: 'Go', purpose: 'Static single-binary executable' }
  - { name: 'Git Porcelain', purpose: 'Deterministic repository inspection' }
  - { name: 'GitHub API', purpose: 'Automated release asset uploads' }
gallery:
  - { caption: 'Terminal release prompt' }
  - { caption: 'Changelog diff preview' }
  - { caption: 'Multi-package monorepo workflow' }
---

Shipit is a single-binary CLI tool written in Go that streamlines software releases by parsing Git history, calculating semantic version bumps, and generating human-readable release notes.

## The problem

Multi-package repositories and monorepos often rely on complicated Node.js release scripts, fragile bash wrappers, or slow CI jobs that fail late in the publication process due to transient credential or tag conflicts.

## The approach

Shipit is distributed as a single, zero-dependency static binary:

- **Conventional Commit Parsing**: Automatically scans commits since the latest Git tag, identifying breaking changes (`major`), features (`minor`), and bug fixes (`patch`).
- **Interactive Review**: Displays a colorized terminal preview of the proposed version increment and changelog entries before touching remote repositories.
- **Cryptographic Signing**: Integrates directly with local GPG and SSH keys to cryptographically sign Git release tags during execution.

```bash
# Preview next release version
shipit --dry-run

# Tag and publish release
shipit --publish
```

## Outcome

Executes in under 150 milliseconds locally and in CI/CD pipelines, replacing bulky multi-dependency npm tooling with a dependable native binary.

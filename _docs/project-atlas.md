---
title: Atlas
description: Documentation for the Atlas project (sample).
category: My projects
---
This page is a template for documenting one of your projects. Replace the content below.

## Overview

Atlas is a sample project used to demonstrate how project documentation lives next to the project showcase. See the [project page]({{ site.baseurl }}/projects/atlas/).

## Installation

```bash
npm install atlas-sample
```

## Usage

```js
import { createAtlas } from "atlas-sample";

const atlas = createAtlas({ container: "#app" });
atlas.start();
```

## Options

| Option | Type | Default | Description |
| --- | --- | --- | --- |
| `container` | `string` | `"#app"` | CSS selector to mount into. |
| `theme` | `"light" \| "dark"` | `"light"` | Color scheme. |

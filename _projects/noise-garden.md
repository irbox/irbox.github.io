---
title: Noise Garden
description: Generative art made from layered noise.
category: Experiments
status: Archived
year: 2023
role: Solo
featured: true
order: 1
links:
  repo: https://github.com/irbox/noise-garden
tags: [Canvas, Generative, Math]
features:
  - Deterministic pseudo-random seeding
  - Multi-octave Simplex noise fields
  - High-resolution PNG and SVG canvas export
stack:
  - { name: 'HTML5 Canvas', purpose: 'Hardware-accelerated 2D rendering' }
  - { name: 'Simplex Noise', purpose: 'Continuous mathematical field generation' }
  - { name: 'JavaScript', purpose: 'Particle simulation and vector math' }
gallery:
  - { caption: 'Multi-octave curl noise rendering' }
  - { caption: 'Color palette parameter space' }
  - { caption: 'Vector export detail' }
---

Noise Garden is an algorithmic generative art experiment exploring emergent organic patterns through layered 2D Simplex noise and velocity vector fields.

## The problem

Exploring generative visual complexity often leans on heavyweight 3D engines or complex WebGL setups, creating substantial overhead for simple geometric and flow-based simulations.

## The approach

Noise Garden uses pure mathematical vector fields rendered directly to the HTML5 Canvas 2D API:

- **Flow Field Dynamics**: Thousands of autonomous particles traverse a grid of velocity vectors derived from multi-octave 2D noise, leaving subtle gradient trails that accumulate into fluid textures.
- **Deterministic URL Seeding**: Every artwork state is encoded into a short alphanumeric seed string stored in the URL hash, allowing exact visual reproductions across devices.
- **Resolution-Independent Export**: Renders output up to 8K resolution in off-screen canvases for crisp physical printing and vector drafting.

```bash
# Clone and explore Noise Garden
git clone https://github.com/irbox/noise-garden.git
cd noise-garden && python -m http.server 8000
```

## Outcome

Achieves smooth 60 FPS particle simulations across modern desktop and mobile browsers with zero external runtime libraries.

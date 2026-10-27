---
title: Atlas
description: A map-based workspace for planning and sharing trips.
category: Web Apps
status: Active
year: 2026
role: Design & engineering
featured: true
order: 1
docs: project-atlas
links:
  repo: https://github.com/irbox/atlas
tags: [React, TypeScript, MapLibre]
features:
  - Offline-first vector tile caching
  - Shareable read-only itinerary links
  - Keyboard-accessible waypoint controls
stack:
  - { name: 'React', purpose: 'UI and state' }
  - { name: 'TypeScript', purpose: 'Type-safe application code' }
  - { name: 'MapLibre GL', purpose: 'Interactive vector maps' }
  - { name: 'Vite', purpose: 'Dev server and bundling' }
gallery:
  - { caption: 'Main itinerary view' }
  - { caption: 'Waypoint elevation profile' }
  - { caption: 'Mobile responsive layout' }
---

Atlas is a spatial itinerary planner engineered for multi-leg expeditions, backcountry routing, and travel navigation. It combines client-side vector tile rendering with local-first data persistence.

## The problem

Most route planning tools require continuous internet connectivity and lock itinerary data into proprietary cloud silos. When planning remote travel or navigating areas with poor cellular reception, web-based map interfaces often fail completely or lose unsaved edits.

## The approach

Atlas is built as an offline-first single page application. Vector map tiles and coordinate waypoints are cached in the browser using IndexedDB and CacheStorage APIs.

- **Client-Side Spatial Indexing**: Route segments and elevation curves are calculated locally in Web Workers, keeping the main UI thread responsive during pan and zoom.
- **Accessible Navigation**: Every waypoint and route control can be selected, reordered, and edited entirely using keyboard shortcuts.
- **Open Standards**: Trip data exports to standard GeoJSON and GPX formats for seamless interoperability with handheld GPS units and mobile mapping apps.

```bash
# Clone and run Atlas locally
git clone https://github.com/irbox/atlas.git
cd atlas && npm install
npm run dev
```

## Outcome

Atlas loads and operates smoothly with zero network latency once regional map tiles are cached. Users can plan complex routes, inspect elevation profiles, and export route data without relying on third-party servers.

---
title: Atlas
description: Architecture, coordinate pipeline, and offline storage reference for Atlas.
category: My projects
---

[Atlas](/projects/atlas/) is an offline-capable spatial itinerary planner. This guide covers its client-side architecture, tile caching mechanisms, and data exchange formats.

## Architecture overview

Atlas is structured around a decoupled spatial pipeline where coordinate calculation, route optimization, and vector tile rendering operate independently:

1. **Map Engine**: Powered by MapLibre GL for rendering vector tile packages (`.mbtiles` and vector PBF tiles).
2. **Local Tile Store**: Uses `CacheStorage` for caching tile coordinates with LRU eviction.
3. **Trip State**: Backed by `IndexedDB` with immutable revision histories for undo/redo support.
4. **Elevation Worker**: Background Web Worker that calculates elevation gain and slope profiles without locking user interactions.

## Quick start

To integrate Atlas components into an existing web project:

```bash
npm install @irbox/atlas
```

```typescript
import { createAtlasMap, WaypointManager } from '@irbox/atlas';

const map = createAtlasMap({
  container: '#map-container',
  center: [-122.4194, 37.7749], // [longitude, latitude]
  zoom: 12,
  offlineStorage: true,
});

const waypoints = new WaypointManager(map);
waypoints.add({
  id: 'point-1',
  name: 'Trailhead',
  coordinates: [-122.4201, 37.7755],
  elevationMeters: 450,
});
```

## Data interchange formats

Atlas exports and imports standard spatial data formats:

### GeoJSON Schema

All itineraries export to standard RFC 7946 GeoJSON FeatureCollections:

```json
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "geometry": {
        "type": "Point",
        "coordinates": [-122.4194, 37.7749]
      },
      "properties": {
        "name": "Basecamp",
        "timestamp": "2026-10-27T10:00:00Z"
      }
    }
  ]
}
```

## Configuration options

| Option | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `container` | `string \| HTMLElement` | `"#map"` | Target container element or CSS selector. |
| `offlineStorage` | `boolean` | `true` | Enables CacheStorage vector tile persistence. |
| `maxTileCacheMB` | `number` | `250` | Maximum disk storage budget for offline map regions. |
| `elevationSmoothing` | `number` | `3` | Smoothing window radius for elevation profile graphing. |
| `theme` | `"light" \| "dark" \| "auto"` | `"auto"` | Map basemap theme mode matching system appearance. |

---
layout: null
sitemap: false
---
/* ==========================================================================
   Irbox PWA Service Worker (v1.1.0)
   Resilient offline-first caching for static assets & intelligent navigation
   ========================================================================== */

const CACHE_NAME = 'irbox-v1.2.0';
const OFFLINE_URL = '{{ "/offline.html" | relative_url }}';

const PRECACHE_ASSETS = [
  '{{ "/" | relative_url }}',
  OFFLINE_URL,
  '{{ "/assets/css/main.css" | relative_url }}',
  '{{ "/assets/js/main.js" | relative_url }}',
  '{{ "/assets/js/reader.js" | relative_url }}',
  '{{ "/assets/images/brand-icon.svg" | relative_url }}',
  '{{ "/assets/images/favicon.svg" | relative_url }}',
  '{{ "/assets/images/icon-192.png" | relative_url }}',
  '{{ "/assets/images/icon-512.png" | relative_url }}',
  '{{ "/assets/fonts/libron/Libron-Regular.woff2" | relative_url }}',
  '{{ "/assets/fonts/libron/Libron-Bold.woff2" | relative_url }}',
  '{{ "/manifest.webmanifest" | relative_url }}',
  '{{ "/books/archive/irbox-docs.epub" | relative_url }}',
  '{{ "/search.json" | relative_url }}'
];

/* Pre-cache core application shell on installation */
self.addEventListener('install', (event) => {
  event.waitUntil(
    caches
      .open(CACHE_NAME)
      .then((cache) => cache.addAll(PRECACHE_ASSETS))
      .then(() => self.skipWaiting())
  );
});

/* Prune outdated caches on activation */
self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches
      .keys()
      .then((keys) =>
        Promise.all(
          keys.map((key) => {
            if (key !== CACHE_NAME) {
              return caches.delete(key);
            }
          })
        )
      )
      .then(() => self.clients.claim())
  );
});

/* Network interceptor with Stale-While-Revalidate and Offline Fallback */
self.addEventListener('fetch', (event) => {
  const req = event.request;
  if (req.method !== 'GET') return;

  const url = new URL(req.url);

  // Ignore cross-origin non-asset requests
  if (url.origin !== self.location.origin && !url.hostname.includes('gstatic.com') && !url.hostname.includes('googleapis.com')) {
    return;
  }

  // 1. Navigation requests (HTML pages) -> Network first with cache & offline fallback
  if (req.mode === 'navigate') {
    event.respondWith(
      fetch(req)
        .then((networkRes) => {
          if (networkRes && networkRes.status === 200) {
            const resClone = networkRes.clone();
            caches.open(CACHE_NAME).then((cache) => cache.put(req, resClone));
          }
          return networkRes;
        })
        .catch(async () => {
          const cachedRes = await caches.match(req);
          if (cachedRes) return cachedRes;
          return caches.match(OFFLINE_URL);
        })
    );
    return;
  }

  // 2. Static assets (CSS, JS, WebFonts, SVGs, Images, E-Books, Manifests) -> Stale-While-Revalidate
  const isStatic =
    url.pathname.endsWith('.css') ||
    url.pathname.endsWith('.js') ||
    url.pathname.endsWith('.woff2') ||
    url.pathname.endsWith('.svg') ||
    url.pathname.endsWith('.png') ||
    url.pathname.endsWith('.jpg') ||
    url.pathname.endsWith('.webp') ||
    url.pathname.endsWith('.ico') ||
    url.pathname.endsWith('.epub') ||
    url.pathname.endsWith('.pdf') ||
    url.pathname.endsWith('.webmanifest') ||
    url.pathname.endsWith('.json');

  if (isStatic) {
    event.respondWith(
      caches.match(req).then((cachedRes) => {
        const fetchPromise = fetch(req)
          .then((networkRes) => {
            if (networkRes && networkRes.status === 200) {
              const resClone = networkRes.clone();
              caches.open(CACHE_NAME).then((cache) => cache.put(req, resClone));
            }
            return networkRes;
          })
          .catch(() => cachedRes);

        return cachedRes || fetchPromise;
      })
    );
  }
});

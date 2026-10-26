/* ==========================================================================
   Irbox PWA Service Worker (v1.1.0)
   Resilient offline-first caching for static assets & intelligent navigation
   ========================================================================== */

const CACHE_NAME = 'irbox-v1.3.0';
const OFFLINE_URL = '/offline.html';

const PRECACHE_ASSETS = [
  '/',
  OFFLINE_URL,
  '/assets/css/main.css',
  '/assets/js/main.js',
  '/assets/js/reader.js',
  '/assets/images/brand-icon.svg',
  '/assets/images/favicon.svg',
  '/assets/images/icon-192.png',
  '/assets/images/icon-512.png',
  '/assets/fonts/libron/Libron-Regular.woff2',
  '/assets/fonts/libron/Libron-Bold.woff2',
  '/manifest.webmanifest',
  '/assets/book.epub',
  '/books/archive/irbox-docs.epub',
  '/search.json',
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
  if (
    url.origin !== self.location.origin &&
    !url.hostname.includes('gstatic.com') &&
    !url.hostname.includes('googleapis.com')
  ) {
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
    url.hostname.includes('gstatic.com') ||
    url.hostname.includes('googleapis.com') ||
    url.pathname.endsWith('.css') ||
    url.pathname.endsWith('.js') ||
    url.pathname.endsWith('.woff2') ||
    url.pathname.endsWith('.woff') ||
    url.pathname.endsWith('.ttf') ||
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

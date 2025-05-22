// sw.js
const CACHE_NAME = 'offline-anki-chat-v1.2'; // Increment version to force update
const urlsToCache = [
    '/', // Caches index.html
    'index.html',
    'style.css',
    'app.js',
    'db.js',
    'anki-parser.js',
    // TensorFlow.js and Universal Sentence Encoder models are loaded from CDN.
    // The service worker needs to cache these too.
    // These URLs might change with new versions, so keep an eye on them.
    'https://cdn.jsdelivr.net/npm/@tensorflow/tfjs@4.x/dist/tf.min.js',
    'https://cdn.jsdelivr.net/npm/@tensorflow-models/universal-sentence-encoder@1.x/dist/universal-sentence-encoder.min.js',
    'https://cdnjs.cloudflare.com/ajax/libs/jszip/3.10.1/jszip.min.js',
    'https://cdnjs.cloudflare.com/ajax/libs/sql.js/1.10.3/sql-wasm.js',
    'https://cdnjs.cloudflare.com/ajax/libs/sql.js/1.10.3/sql-wasm.wasm', // sql.js requires this WASM file
    // Universal Sentence Encoder model files (these are typically fetched by TF.js when model.load() is called)
    // You might need to inspect network requests during first load to get exact URLs,
    // but the TF.js loader usually handles internal caching for these.
    // For robustness, explicitly caching TF.js model URLs is recommended if they are static.
    // Example: (These can be dynamic based on TF.js model versions, verify in network tab)
    'https://storage.googleapis.com/tfjs-models/tfjs/universal-sentence-encoder/model.json',
    'https://storage.googleapis.com/tfjs-models/tfjs/universal-sentence-encoder/group1-shard1of1.bin',
];

self.addEventListener('install', event => {
    console.log('Service Worker: Install event');
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then(cache => {
                console.log('Service Worker: Caching app shell');
                return cache.addAll(urlsToCache);
            })
            .catch(error => {
                console.error('Service Worker: Failed to cache during install:', error);
            })
    );
});

self.addEventListener('activate', event => {
    console.log('Service Worker: Activate event');
    event.waitUntil(
        caches.keys().then(cacheNames => {
            return Promise.all(
                cacheNames.map(cacheName => {
                    if (cacheName !== CACHE_NAME) {
                        console.log('Service Worker: Deleting old cache', cacheName);
                        return caches.delete(cacheName);
                    }
                })
            );
        })
    );
});

self.addEventListener('fetch', event => {
    // console.log('Service Worker: Fetching', event.request.url);
    event.respondWith(
        caches.match(event.request)
            .then(response => {
                // Cache hit - return response
                if (response) {
                    return response;
                }
                // No cache hit - fetch from network
                return fetch(event.request)
                    .then(fetchResponse => {
                        // If we fetched successfully, clone response and add to cache
                        // This is for dynamic caching of TF.js model shards etc.
                        if (fetchResponse && fetchResponse.status === 200 && fetchResponse.type === 'basic') {
                            const responseToCache = fetchResponse.clone();
                            caches.open(CACHE_NAME)
                                .then(cache => {
                                    cache.put(event.request, responseToCache);
                                });
                        }
                        return fetchResponse;
                    })
                    .catch(error => {
                        // This catch block handles network errors.
                        // You might want to return a fallback page for navigations.
                        console.error('Service Worker: Fetch failed for', event.request.url, error);
                        // For example, if it's a navigation request and no cache, return an offline page
                        if (event.request.mode === 'navigate') {
                             // return caches.match('/offline.html'); // Optional: serve a custom offline page
                        }
                        throw error; // Re-throw to propagate the error
                    });
            })
            .catch(error => {
                console.error('Service Worker: Caching/Fetch failed:', error);
                // Fallback for when both cache and network fail
            })
    );
});

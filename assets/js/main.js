/**
 * Irbox — High-Performance Zero-Bloat Client Architecture
 * Strictly POSIX standards compliant, dependency-free ECMAScript.
 */
(() => {
  'use strict';

  const doc = document;
  const root = doc.documentElement;
  root.classList.add('js');

  const $ = (s, c = doc) => c.querySelector(s);
  const $$ = (s, c = doc) => Array.from(c.querySelectorAll(s));
  const store = {
    get: (k) => { try { return localStorage.getItem(k); } catch { return null; } },
    set: (k, v) => { try { localStorage.setItem(k, v); } catch {} }
  };

  /* ---- 1. Page Progress Indicator ---- */
  const progress = $('#page-progress');
  if (progress) {
    const done = () => {
      progress.classList.remove('is-loading');
      progress.classList.add('is-done');
      setTimeout(() => progress.classList.remove('is-done'), 350);
    };
    window.addEventListener('pageshow', done);
    window.addEventListener('popstate', done);
    doc.addEventListener('click', (e) => {
      const a = e.target.closest('a');
      if (!a || !a.href || a.target === '_blank' || a.hasAttribute('download')) return;
      if (e.metaKey || e.ctrlKey || e.shiftKey || e.altKey || e.defaultPrevented) return;
      const href = a.getAttribute('href');
      if (!href || href.startsWith('#') || href.startsWith('mailto:') || href.startsWith('tel:')) return;
      if (/\.(epub|pdf|zip|tar|gz|xml|json|rss|txt)$/i.test(a.pathname)) return;
      try {
        const u = new URL(a.href, location.href);
        if (u.origin === location.origin && (u.pathname !== location.pathname || u.search !== location.search)) {
          progress.classList.remove('is-done');
          progress.classList.add('is-loading');
        }
      } catch {}
    });
  }

  /* ---- 2. Image Loading Transitions ---- */
  $$('img.thumb, img.avatar').forEach((img) => {
    if (img.complete && img.naturalWidth > 0) img.classList.add('is-loaded');
    else {
      const ok = () => img.classList.add('is-loaded');
      img.addEventListener('load', ok, { once: true });
      img.addEventListener('error', ok, { once: true });
    }
  });

  /* ---- 3. Theme Controller (Choroid v4.0 / Nature System: Deep, Soft, Lumen) ---- */
  const themeBtns = $$('[data-theme-toggle]');
  const metas = $$('meta[name="theme-color"]');
  const getTheme = () => {
    const t = root.getAttribute('data-theme') || root.getAttribute('data-choroid') || store.get('theme');
    if (t === 'soft') return 'soft';
    if (t === 'light' || t === 'lumen' || t === 'nature-light') return 'light';
    return 'dark'; // Deep baseline
  };
  const syncTheme = (t) => {
    const choroidVariant = (t === 'light' || t === 'lumen') ? 'lumen' : (t === 'soft' ? 'soft' : 'deep');
    root.setAttribute('data-theme', t);
    root.setAttribute('data-choroid', choroidVariant);
    const colors = { dark: '#0a0f14', soft: '#121a22', light: '#f5f7f9' };
    const labels = {
      dark: 'Current: Deep (Max contrast 16.5:1, myopia-informed). Click to switch to Soft (or press T)',
      soft: 'Current: Soft (Reduced halation 13.6:1, astigmatism-friendly). Click to switch to Lumen (or press T)',
      light: 'Current: Lumen (Daylight fallback 16.3:1). Click to switch to Deep (or press T)'
    };
    metas.forEach((m) => m.setAttribute('content', colors[t] || '#0a0f14'));
    themeBtns.forEach((b) => {
      b.setAttribute('aria-label', labels[t] || labels.dark);
      b.setAttribute('title', labels[t] || labels.dark);
    });
  };
  const cycleTheme = () => {
    const cur = getTheme();
    const next = (cur === 'dark') ? 'soft' : ((cur === 'soft') ? 'light' : 'dark');
    root.setAttribute('data-theme', next);
    store.set('theme', next);
    syncTheme(next);
  };
  syncTheme(getTheme());
  themeBtns.forEach((b) => b.addEventListener('click', cycleTheme));
  window.addEventListener('storage', (e) => {
    if (e.key === 'theme') {
      if (e.newValue) root.setAttribute('data-theme', e.newValue);
      else root.removeAttribute('data-theme');
      syncTheme(getTheme());
    }
  });
  try {
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', (m) => {
      if (!store.get('theme')) syncTheme(m.matches ? 'dark' : 'light');
    });
  } catch {}

  /* ---- 3b. Reading Distance & Font Size Calibration (~34.4 arcmin rule) ---- */
  const distanceBtns = $$('[data-distance]');
  const metricBtns = $$('[data-metric-toggle]');
  const syncDistance = (dist) => {
    const valid = ['30', '35', '40', '50', '60', '70'].includes(dist) ? dist : '40';
    root.setAttribute('data-reading-distance', valid + 'cm');
    root.setAttribute('data-choroid-distance', valid + 'cm');
    distanceBtns.forEach((b) => {
      const isCur = b.getAttribute('data-distance') === valid;
      b.classList.toggle('is-active', isCur);
      b.setAttribute('aria-pressed', String(isCur));
    });
  };
  const syncMetric = (m) => {
    const metric = (m === 'x-height') ? 'x-height' : 'cap-height';
    if (metric === 'x-height') {
      root.setAttribute('data-choroid-metric', 'x-height');
      root.setAttribute('data-reading-metric', 'x-height');
    } else {
      root.removeAttribute('data-choroid-metric');
      root.removeAttribute('data-reading-metric');
    }
    metricBtns.forEach((b) => {
      b.setAttribute('data-current-metric', metric);
      b.textContent = (metric === 'x-height') ? 'Metric: x-height (0.52em)' : 'Metric: Cap (0.70em)';
      b.setAttribute('aria-pressed', String(metric === 'x-height'));
    });
  };
  const initialDistance = store.get('reading-distance') || '40';
  syncDistance(initialDistance);
  const initialMetric = store.get('reading-metric') || 'cap-height';
  syncMetric(initialMetric);
  distanceBtns.forEach((b) => {
    b.addEventListener('click', () => {
      const dist = b.getAttribute('data-distance');
      if (dist) {
        store.set('reading-distance', dist);
        syncDistance(dist);
      }
    });
  });
  metricBtns.forEach((b) => {
    b.addEventListener('click', () => {
      const cur = root.getAttribute('data-choroid-metric') === 'x-height' ? 'x-height' : 'cap-height';
      const next = (cur === 'x-height') ? 'cap-height' : 'x-height';
      store.set('reading-metric', next);
      syncMetric(next);
    });
  });

  /* ---- 4. Mobile Navigation & Sidebar ---- */
  const drawerBtn = $('[data-drawer-toggle]');
  const drawer = $('#drawer');
  if (drawerBtn && drawer) {
    const setDrawer = (open) => {
      drawer.classList.toggle('is-open', open);
      if (open) drawer.removeAttribute('hidden');
      else setTimeout(() => { if (!drawer.classList.contains('is-open')) drawer.setAttribute('hidden', ''); }, 200);
      drawerBtn.setAttribute('aria-expanded', String(open));
      drawerBtn.setAttribute('aria-label', open ? 'Close menu' : 'Open menu');
    };
    drawerBtn.addEventListener('click', () => setDrawer(!drawer.classList.contains('is-open')));
    $$('a', drawer).forEach((a) => a.addEventListener('click', () => setDrawer(false)));
    doc.addEventListener('keydown', (e) => {
      if (e.key === 'Escape' && drawer.classList.contains('is-open')) setDrawer(false);
    });
    doc.addEventListener('click', (e) => {
      if (drawer.classList.contains('is-open') && !e.target.closest('#topnav') && !e.target.closest('#drawer')) setDrawer(false);
    });
  }

  const sidebar = $('#sidebar');
  const sideBtns = $$('[data-sidebar-toggle]');
  if (sidebar && sideBtns.length) {
    const setSidebar = (open) => {
      sidebar.classList.toggle('is-open', open);
      sideBtns.forEach((b) => b.setAttribute('aria-expanded', String(open)));
    };
    sideBtns.forEach((b) => b.addEventListener('click', (e) => {
      e.stopPropagation();
      setSidebar(!sidebar.classList.contains('is-open'));
    }));
    $$('a', sidebar).forEach((a) => a.addEventListener('click', () => { if (window.innerWidth <= 767) setSidebar(false); }));
    doc.addEventListener('click', (e) => {
      if (sidebar.classList.contains('is-open') && !sidebar.contains(e.target) && !e.target.closest('[data-sidebar-toggle]')) setSidebar(false);
    });
    doc.addEventListener('keydown', (e) => {
      if (e.key === 'Escape' && sidebar.classList.contains('is-open')) setSidebar(false);
    });
  }

  /* ---- 5. Announcement Banner ---- */
  const banner = $('[data-announcement]');
  const bannerDismiss = $('[data-announcement-dismiss]');
  if (banner && bannerDismiss) {
    bannerDismiss.addEventListener('click', () => {
      store.set('announcement-dismissed', banner.dataset.announcement);
      root.setAttribute('data-announcement-hidden', '');
    });
  }

  /* ---- 6. Code Copy & Outline Spy ---- */
  const copyText = async (text) => {
    if (navigator.clipboard?.writeText) {
      try { await navigator.clipboard.writeText(text); return true; } catch {}
    }
    const ta = doc.createElement('textarea');
    ta.value = text;
    ta.style.cssText = 'position:fixed;opacity:0';
    doc.body.appendChild(ta);
    ta.select();
    try { return doc.execCommand('copy'); } finally { ta.remove(); }
  };

  doc.addEventListener('click', async (e) => {
    const b = e.target.closest('.code-copy-btn, .copy-btn');
    if (!b) return;
    const pre = b.closest('pre');
    const code = pre?.querySelector('code')?.innerText || pre?.innerText || '';
    if (await copyText(code.replace(/\n$/, ''))) {
      const orig = b.textContent;
      b.classList.add('is-copied');
      b.textContent = 'Copied!';
      setTimeout(() => {
        b.classList.remove('is-copied');
        b.textContent = orig;
      }, 1800);
    }
  });

  const outlineLinks = $$('.outline a');
  if (outlineLinks.length && 'IntersectionObserver' in window) {
    const headingMap = new Map(outlineLinks.map((a) => [a.getAttribute('href').slice(1), a]));
    const obs = new IntersectionObserver((entries) => {
      entries.forEach((entry) => {
        if (entry.isIntersecting) {
          const a = headingMap.get(entry.target.id);
          if (a) {
            outlineLinks.forEach((l) => l.classList.remove('is-active'));
            a.classList.add('is-active');
          }
        }
      });
    }, { rootMargin: '0px 0px -70% 0px' });
    headingMap.forEach((_, id) => {
      const el = doc.getElementById(id);
      if (el) obs.observe(el);
    });
  }

  /* ---- 7. Project Tabs ---- */
  const tabs = $$('[data-tabs] [data-tab]');
  if (tabs.length) {
    const selectTab = (name, updateUrl) => {
      tabs.forEach((t) => {
        const on = t.dataset.tab === name;
        t.classList.toggle('is-active', on);
        t.setAttribute('aria-selected', String(on));
      });
      $$('.tab-panel').forEach((p) => {
        const on = p.dataset.panel === name;
        p.toggleAttribute('hidden', !on);
        p.classList.toggle('is-active', on);
      });
      if (updateUrl && history.replaceState) history.replaceState(null, '', '#' + name);
    };
    tabs.forEach((t) => t.addEventListener('click', () => selectTab(t.dataset.tab, true)));
    const hash = location.hash.replace('#', '');
    if (hash && tabs.some((t) => t.dataset.tab === hash)) selectTab(hash, false);
  }

  /* ---- 8. Project Filter Chips ---- */
  const chips = $$('[data-filter-chips] [data-filter]');
  if (chips.length) {
    chips.forEach((c) => {
      c.addEventListener('click', () => {
        const f = c.dataset.filter;
        chips.forEach((b) => {
          const on = b === c;
          b.classList.toggle('is-active', on);
          b.setAttribute('aria-pressed', String(on));
        });
        $$('.category[data-category-section]').forEach((sec) => {
          sec.hidden = f !== 'all' && sec.dataset.categorySection !== f;
        });
        $$('.project-card[data-category]').forEach((card) => {
          card.hidden = f !== 'all' && card.dataset.category !== f;
        });
      });
    });
  }

  /* ---- 9. Search Dialog Controller ---- */
  const searchDlg = $('#search-dialog');
  const searchInput = $('#search-input');
  const searchResults = $('#search-results');
  let searchIndex = null;
  let searchSelected = -1;
  let searchDebounce = null;

  const loadSearch = () => {
    if (searchIndex) return Promise.resolve(searchIndex);
    const path = searchDlg?.getAttribute('data-index') || '/search.json';
    return fetch(path)
      .then((r) => { if (!r.ok) throw new Error('HTTP ' + r.status); return r.json(); })
      .then((data) => { searchIndex = data; return data; })
      .catch((err) => {
        searchIndex = null;
        if (searchResults) {
          const li = doc.createElement('li');
          li.className = 'search__empty search__empty--error';
          li.textContent = `Search index unavailable (${err.message}).`;
          searchResults.replaceChildren(li);
        }
        throw err;
      });
  };

  const renderSearch = (q) => {
    if (!searchResults) return;
    q = q.trim().toLowerCase();
    const words = q.split(/\s+/).filter(Boolean);
    if (!words.length) {
      searchResults.replaceChildren();
      return;
    }
    const hits = (searchIndex || []).filter((item) => {
      const text = `${item.title || ''} ${item.description || ''} ${item.url || ''}`.toLowerCase();
      return words.every((w) => text.includes(w));
    });

    if (!hits.length) {
      const li = doc.createElement('li');
      li.className = 'search__empty';
      li.textContent = 'No matching entries found.';
      searchResults.replaceChildren(li);
      return;
    }

    const frag = doc.createDocumentFragment();
    hits.slice(0, 10).forEach((item, idx) => {
      const li = doc.createElement('li');
      const a = doc.createElement('a');
      a.href = item.url;
      if (idx === 0) a.className = 'is-selected';
      a.innerHTML = `<span class="meta">${item.type || 'Page'}</span><strong>${item.title || ''}</strong>` +
        (item.description ? `<span class="desc">${item.description}</span>` : '');
      li.appendChild(a);
      frag.appendChild(li);
    });
    searchResults.replaceChildren(frag);
    searchSelected = 0;
  };

  const openSearch = () => {
    if (!searchDlg?.showModal) return;
    if (!searchDlg.open) searchDlg.showModal();
    if (searchInput) {
      searchInput.value = '';
      searchInput.focus();
    }
    loadSearch().then(() => renderSearch(''));
  };

  const closeSearch = () => {
    if (searchDlg?.open) searchDlg.close();
  };

  $$('[data-search-open]').forEach((b) => b.addEventListener('click', openSearch));
  $$('[data-search-close]').forEach((b) => b.addEventListener('click', closeSearch));
  searchDlg?.addEventListener('click', (e) => { if (e.target === searchDlg) closeSearch(); });

  searchInput?.addEventListener('input', () => {
    clearTimeout(searchDebounce);
    searchDebounce = setTimeout(() => {
      loadSearch().then(() => renderSearch(searchInput.value)).catch(() => {});
    }, 40);
  });

  searchInput?.addEventListener('keydown', (e) => {
    const items = $$('a', searchResults);
    if (!items.length) return;
    if (e.key === 'ArrowDown' || e.key === 'ArrowUp') {
      e.preventDefault();
      items.forEach((a) => a.classList.remove('is-selected'));
      searchSelected = (searchSelected + (e.key === 'ArrowDown' ? 1 : -1) + items.length) % items.length;
      items[searchSelected].classList.add('is-selected');
      items[searchSelected].scrollIntoView({ block: 'nearest' });
    } else if (e.key === 'Enter' && items[searchSelected]) {
      e.preventDefault();
      items[searchSelected].click();
    }
  });

  /* ---- 10. Shortcuts Dialog & Hotkeys ---- */
  const shortcutsDlg = $('#shortcuts-dialog');
  const openShortcuts = () => { if (shortcutsDlg?.showModal && !shortcutsDlg.open) shortcutsDlg.showModal(); };
  $$('[data-shortcuts-open]').forEach((b) => b.addEventListener('click', openShortcuts));
  $$('[data-shortcuts-close]').forEach((b) => b.addEventListener('click', () => shortcutsDlg?.close()));
  shortcutsDlg?.addEventListener('click', (e) => { if (e.target === shortcutsDlg) shortcutsDlg.close(); });

  doc.addEventListener('keydown', (e) => {
    const tag = e.target.tagName;
    if (tag === 'INPUT' || tag === 'TEXTAREA' || e.target.isContentEditable) return;
    if ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === 'k') {
      e.preventDefault();
      openSearch();
      return;
    }
    if (!e.metaKey && !e.ctrlKey && !e.altKey) {
      const k = e.key.toLowerCase();
      if (k === '/') { e.preventDefault(); openSearch(); }
      else if (k === '?') { e.preventDefault(); openShortcuts(); }
      else if (k === 't') cycleTheme();
      else if (k === 'h') location.href = '/';
      else if (k === 'd') location.href = '/docs/getting-started/';
      else if (k === 'p') location.href = '/projects/';
      else if (k === 'b') location.href = '/blog/';
    }
  });

  /* ---- 11. Reading Progress Bar (Fallback) ---- */
  const readingBar = $('.reading-progress');
  if (readingBar && !(window.CSS?.supports?.('animation-timeline', 'scroll()'))) {
    let ticking = false;
    const updateReading = () => {
      const max = root.scrollHeight - window.innerHeight;
      readingBar.style.transform = `scaleX(${max > 0 ? Math.min(1, Math.max(0, window.scrollY / max)) : 0})`;
      ticking = false;
    };
    window.addEventListener('scroll', () => {
      if (!ticking) { requestAnimationFrame(updateReading); ticking = true; }
    }, { passive: true });
    updateReading();
  }

  /* ---- 12. Back To Top ---- */
  $('.footer__back-to-top')?.addEventListener('click', (e) => {
    e.preventDefault();
    window.scrollTo({ top: 0, behavior: 'smooth' });
  });

  /* ---- 13. Web Share ---- */
  $$('[data-share-btn]').forEach((b) => {
    b.addEventListener('click', async () => {
      if (navigator.share) {
        try { await navigator.share({ title: doc.title, url: location.href }); return; } catch {}
      }
      copyText(location.href);
      b.classList.add('is-copied');
      setTimeout(() => b.classList.remove('is-copied'), 2000);
    });
  });

  /* ---- 14. Service Worker & Print ---- */
  if ('serviceWorker' in navigator && (location.protocol === 'https:' || location.hostname === 'localhost' || location.hostname === '127.0.0.1')) {
    window.addEventListener('load', () => navigator.serviceWorker.register('/sw.js').catch(() => {}));
  }
  $$('[data-print-btn]').forEach((b) => b.addEventListener('click', () => window.print()));

  requestAnimationFrame(() => root.classList.remove('is-loading'));
})();

/**
 * Irbox — Modern Web Standards Client
 * Powered directly by Chrome, Safari & Firefox native platform capabilities:
 * Popover API, <dialog>, View Transitions, OKLCH light-dark(), and Scroll-Driven Animations.
 */
(() => {
  'use strict';

  const doc = document;
  const root = doc.documentElement;
  root.classList.add('js');

  /* ---- 1. Native Color Scheme & View Transitions ---- */
  const getTheme = () =>
    root.getAttribute('data-theme') ||
    (window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light');

  const applyTheme = (theme, withTransition = true) => {
    const update = () => {
      root.setAttribute('data-theme', theme);
      root.style.colorScheme = theme;
      try {
        localStorage.setItem('theme', theme);
      } catch (_) {}
      const next = theme === 'dark' ? 'light' : 'dark';
      doc.querySelectorAll('[data-theme-toggle]').forEach((b) => {
        b.setAttribute('aria-label', `Switch to ${next} theme (or press T)`);
        b.setAttribute('title', `Switch to ${next} theme (T)`);
      });
    };

    if (
      withTransition &&
      doc.startViewTransition &&
      !window.matchMedia('(prefers-reduced-motion: reduce)').matches
    ) {
      doc.startViewTransition(update);
    } else {
      update();
    }
  };

  const initTheme = () => {
    const cur = getTheme();
    applyTheme(cur, false);

    doc.querySelectorAll('[data-theme-toggle]').forEach((btn) => {
      btn.addEventListener('click', () => {
        applyTheme(getTheme() === 'dark' ? 'light' : 'dark', true);
      });
    });

    window.addEventListener('storage', (e) => {
      if (e.key === 'theme' && (e.newValue === 'light' || e.newValue === 'dark')) {
        applyTheme(e.newValue, false);
      }
    });

    try {
      window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', (e) => {
        if (!localStorage.getItem('theme')) {
          applyTheme(e.matches ? 'dark' : 'light', true);
        }
      });
    } catch (_) {}
  };

  /* ---- 2. Page Loading Indicator ---- */
  const initPageProgress = () => {
    const bar = doc.getElementById('page-progress');
    if (!bar) return;
    const finish = () => {
      bar.classList.remove('is-loading');
      bar.classList.add('is-done');
      setTimeout(() => bar.classList.remove('is-done'), 350);
    };
    window.addEventListener('pageshow', finish);
    doc.addEventListener('click', (e) => {
      const a = e.target.closest('a');
      if (!a || !a.href || a.target === '_blank' || a.hasAttribute('download')) return;
      if (e.metaKey || e.ctrlKey || e.shiftKey || e.altKey || e.defaultPrevented) return;
      const href = a.getAttribute('href');
      if (!href || href.startsWith('#') || href.startsWith('mailto:') || href.startsWith('tel:')) return;
      if (/\.(epub|pdf|zip|gz|xml|json|rss|txt)$/i.test(a.pathname)) return;
      if (a.origin !== location.origin || (a.pathname === location.pathname && a.search === location.search)) return;
      bar.classList.remove('is-done');
      bar.classList.add('is-loading');
    });
  };

  /* ---- 3. Native Popover Drawers (Mobile Nav & Sidebar) ---- */
  const initDrawers = () => {
    const drawer = doc.getElementById('drawer');
    if (drawer) {
      drawer.querySelectorAll('a').forEach((link) => {
        link.addEventListener('click', () => {
          try {
            drawer.hidePopover?.();
          } catch (_) {}
        });
      });
    }

    const sidebar = doc.getElementById('sidebar');
    const sideBtns = doc.querySelectorAll('[data-sidebar-toggle]');
    if (sidebar) {
      const setSidebar = (open) => {
        sidebar.classList.toggle('is-open', open);
        sideBtns.forEach((b) => b.setAttribute('aria-expanded', String(open)));
      };
      sideBtns.forEach((b) => {
        b.addEventListener('click', (e) => {
          e.stopPropagation();
          setSidebar(!sidebar.classList.contains('is-open'));
        });
      });
      sidebar.querySelectorAll('a').forEach((link) => {
        link.addEventListener('click', () => {
          if (window.innerWidth <= 767) setSidebar(false);
        });
      });
      doc.addEventListener('click', (e) => {
        if (sidebar.classList.contains('is-open') && !sidebar.contains(e.target) && !e.target.closest('[data-sidebar-toggle]')) {
          setSidebar(false);
        }
      });
      doc.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && sidebar.classList.contains('is-open')) setSidebar(false);
      });
    }
  };

  /* ---- 4. Modern Clipboard & Code Blocks ---- */
  const copyText = async (text) => {
    if (navigator.clipboard) {
      try {
        await navigator.clipboard.writeText(text);
        return true;
      } catch (_) {}
    }
    return false;
  };

  const initProse = () => {
    // Scrollable table wrappers
    doc.querySelectorAll('.prose table, [data-prose] table').forEach((table) => {
      if (!table.parentElement.classList.contains('table-wrap')) {
        const wrap = doc.createElement('div');
        wrap.className = 'table-wrap';
        wrap.tabIndex = 0;
        wrap.setAttribute('role', 'region');
        wrap.setAttribute('aria-label', 'Scrollable table');
        table.parentNode.insertBefore(wrap, table);
        wrap.appendChild(table);
      }
    });

    // Heading anchors
    doc.querySelectorAll('.prose :is(h2,h3,h4)[id], [data-prose] :is(h2,h3,h4)[id]').forEach((h) => {
      if (!h.querySelector('.anchor')) {
        const a = doc.createElement('a');
        a.className = 'anchor';
        a.href = '#' + h.id;
        a.setAttribute('aria-label', 'Link to this section');
        a.textContent = '#';
        h.appendChild(a);
      }
    });

    // Copy buttons
    doc.querySelectorAll('.prose pre, [data-prose] pre').forEach((pre) => {
      if (pre.dataset.hasCopyBtn) return;
      pre.dataset.hasCopyBtn = 'true';

      let container = pre.parentElement;
      if (!container.classList.contains('code-block-wrap')) {
        const wrap = doc.createElement('div');
        wrap.className = 'code-block-wrap';
        pre.parentNode.insertBefore(wrap, pre);
        wrap.appendChild(pre);
        container = wrap;
      }

      const btn = doc.createElement('button');
      btn.type = 'button';
      btn.className = 'copy-btn';
      btn.setAttribute('aria-label', 'Copy code to clipboard');
      btn.innerHTML = `
        <svg class="copy-btn__icon copy-btn__icon--copy" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="9" y="9" width="13" height="13" rx="2"/><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/></svg>
        <svg class="copy-btn__icon copy-btn__icon--check" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="20 6 9 17 4 12"/></svg>
        <span class="copy-btn__text">Copy</span>
      `;
      btn.addEventListener('click', async (e) => {
        e.preventDefault();
        const code = pre.querySelector('code') || pre;
        const text = code.textContent.replace(/\r\n/g, '\n').replace(/\n$/, '');
        const ok = await copyText(text);
        btn.classList.add(ok ? 'is-copied' : 'is-error');
        const txt = btn.querySelector('.copy-btn__text');
        if (txt) txt.textContent = ok ? 'Copied' : 'Failed';
        setTimeout(() => {
          btn.classList.remove('is-copied', 'is-error');
          if (txt) txt.textContent = 'Copy';
        }, 1800);
      });
      container.appendChild(btn);
    });

    // Syntax highlighting
    const escapeHtml = (s) => s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    const tokenRegex =
      /(\/\*[\s\S]*?\*\/|\/\/[^\n]*|#[^\n]*)|("(?:\\[\s\S]|[^"\\])*"|'(?:\\[\s\S]|[^'\\])*'|`(?:\\[\s\S]|[^`\\])*`)|(\b(?:0x[0-9a-fA-F]+|0b[01]+|\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\b)|(\b(?:const|constexpr|auto|static|struct|typedef|void|int|char|size_t|bool|true|false|nullptr|if|else|for|while|return|switch|case|break|continue|default|import|export|from|function|class|async|await|try|catch|new|this|let|var|package|echo|cd|make|git|mkdir|rm)\b)|(\b[a-zA-Z_]\w*(?=\s*\())|(==|!=|<=|>=|=>|->|\+\+|--|&&|\|\||[+\-*\/%&|^!<>]=?)/g;

    doc.querySelectorAll('.prose pre code, [data-prose] pre code').forEach((code) => {
      if (code.dataset.highlighted) return;
      code.dataset.highlighted = 'true';
      code.classList.add('highlight');
      const raw = code.textContent;
      if (!raw || raw.length > 50000) return;

      let html = '';
      let pos = 0;
      let match;
      while ((match = tokenRegex.exec(raw)) !== null) {
        if (match.index > pos) html += escapeHtml(raw.slice(pos, match.index));
        const token = match[0];
        let cls = 'o';
        if (match[1]) cls = 'c';
        else if (match[2]) cls = 's';
        else if (match[3]) cls = 'm';
        else if (match[4]) cls = 'k';
        else if (match[5]) cls = 'nf';
        html += `<span class="${cls}">${escapeHtml(token)}</span>`;
        pos = tokenRegex.lastIndex;
      }
      if (pos < raw.length) html += escapeHtml(raw.slice(pos));
      code.innerHTML = html;
    });
  };

  /* ---- 5. View Transitions for Tabs & Filter Chips ---- */
  const initProjectTabs = () => {
    const tabs = Array.from(doc.querySelectorAll('[data-tabs] [data-tab]'));
    if (!tabs.length) return;

    const setTab = (name) => {
      const apply = () => {
        tabs.forEach((t) => {
          const on = t.getAttribute('data-tab') === name;
          t.classList.toggle('is-active', on);
          t.setAttribute('aria-selected', String(on));
        });
        doc.querySelectorAll('.tab-panel').forEach((p) => {
          p.toggleAttribute('hidden', p.getAttribute('data-panel') !== name);
        });
      };
      if (doc.startViewTransition && !window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
        doc.startViewTransition(apply);
      } else {
        apply();
      }
    };

    tabs.forEach((t) => {
      t.addEventListener('click', () => {
        const name = t.getAttribute('data-tab');
        setTab(name);
        history.replaceState(null, '', '#' + name);
      });
    });

    const hash = location.hash.replace('#', '');
    if (hash && doc.querySelector(`[data-tab="${hash}"]`)) setTab(hash);
  };

  const initFilterChips = () => {
    const chips = Array.from(doc.querySelectorAll('[data-filter]'));
    if (!chips.length) return;

    const setFilter = (filter) => {
      chips.forEach((c) => {
        const on = c.getAttribute('data-filter') === filter;
        c.classList.toggle('is-active', on);
        c.setAttribute('aria-pressed', String(on));
      });
      doc.querySelectorAll('[data-category-section]').forEach((sec) => {
        sec.classList.toggle('is-hidden', filter !== 'all' && sec.getAttribute('data-category-section') !== filter);
      });
    };

    chips.forEach((chip) => {
      chip.addEventListener('click', () => {
        const f = chip.getAttribute('data-filter');
        setFilter(f);
        history.replaceState(null, '', f === 'all' ? location.pathname : '#' + f);
      });
    });

    const hash = location.hash.replace('#', '');
    if (hash && doc.querySelector(`[data-filter="${hash}"]`)) setFilter(hash);
  };

  /* ---- 6. Native <dialog> Powered Search ---- */
  const initSearch = () => {
    const dlg = doc.getElementById('search-dialog');
    const input = doc.getElementById('search-input');
    const results = doc.getElementById('search-results');
    const status = doc.getElementById('search-status');
    if (!dlg || !input || !results) return;

    let indexData = null;
    let selectedIdx = -1;

    const loadIndex = async () => {
      if (indexData) return indexData;
      try {
        const r = await fetch(dlg.getAttribute('data-index') || '/search.json');
        indexData = await r.json();
      } catch (_) {
        indexData = [];
      }
      return indexData;
    };

    const escapeHtml = (s) => String(s || '').replace(/[&<>"']/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' })[c]);

    const render = (query) => {
      const q = query.trim().toLowerCase();
      const words = q.split(/\s+/).filter(Boolean);
      const hits = (indexData || [])
        .map((item) => {
          const t = String(item.title || '').toLowerCase();
          const d = String(item.description || '').toLowerCase();
          const tags = String(item.tags || '').toLowerCase();
          let score = 0;
          if (!words.length) return { item, score: 1 };
          for (const w of words) {
            if (t.startsWith(w)) score += 8;
            else if (t.includes(w)) score += 5;
            else if (tags.includes(w)) score += 3;
            else if (d.includes(w)) score += 1;
            else return { item, score: 0 };
          }
          return { item, score };
        })
        .filter((h) => h.score > 0)
        .sort((a, b) => b.score - a.score)
        .slice(0, 8);

      selectedIdx = hits.length ? 0 : -1;
      if (status) status.textContent = hits.length ? `${hits.length} result${hits.length === 1 ? '' : 's'}` : 'No results';

      results.innerHTML = hits.length
        ? hits
            .map(
              (h, i) =>
                `<li><a href="${escapeHtml(h.item.url)}"${i === 0 ? ' class="is-selected"' : ''}>` +
                `<span class="meta">${escapeHtml(h.item.type)}</span><strong>${escapeHtml(h.item.title)}</strong>` +
                (h.item.description ? `<span class="desc">${escapeHtml(h.item.description)}</span>` : '') +
                '</a></li>'
            )
            .join('')
        : '<li class="search__empty">No results found</li>';
    };

    const openSearch = async () => {
      if (!dlg.open) dlg.showModal();
      input.value = '';
      await loadIndex();
      render('');
      input.focus();
    };

    doc.querySelectorAll('[data-search-open]').forEach((btn) => btn.addEventListener('click', openSearch));

    // Native backdrop click closing
    dlg.addEventListener('click', (e) => {
      if (e.target === dlg) dlg.close();
    });

    input.addEventListener('input', () => render(input.value));
    input.addEventListener('keydown', (e) => {
      const items = Array.from(results.querySelectorAll('a'));
      if (e.key === 'ArrowDown' || e.key === 'ArrowUp') {
        e.preventDefault();
        if (!items.length) return;
        items.forEach((a) => a.classList.remove('is-selected'));
        const delta = e.key === 'ArrowDown' ? 1 : -1;
        selectedIdx = (selectedIdx + delta + items.length) % items.length;
        items[selectedIdx].classList.add('is-selected');
        items[selectedIdx].scrollIntoView({ block: 'nearest' });
      } else if (e.key === 'Enter') {
        e.preventDefault();
        if (items[selectedIdx]) {
          dlg.close();
          location.href = items[selectedIdx].href;
        }
      }
    });

    return openSearch;
  };

  /* ---- 7. Global Shortcuts & Dialog ---- */
  const initShortcuts = (openSearch) => {
    const shortcutsDlg = doc.getElementById('shortcuts-dialog');
    if (shortcutsDlg) {
      shortcutsDlg.addEventListener('click', (e) => {
        if (e.target === shortcutsDlg) shortcutsDlg.close();
      });
      doc.querySelectorAll('[data-shortcuts-open]').forEach((btn) => {
        btn.addEventListener('click', () => {
          if (!shortcutsDlg.open) shortcutsDlg.showModal();
        });
      });
    }

    doc.addEventListener('keydown', (e) => {
      const isInput = Boolean(e.target?.closest?.('input, textarea, select, [contenteditable="true"]'));
      const isSearchOpen = doc.getElementById('search-dialog')?.open;
      const isShortcutsOpen = shortcutsDlg?.open;

      // Cmd+K / Ctrl+K
      if ((e.metaKey || e.ctrlKey) && !e.shiftKey && !e.altKey && e.key.toLowerCase() === 'k') {
        e.preventDefault();
        const dlg = doc.getElementById('search-dialog');
        if (dlg) dlg.open ? dlg.close() : openSearch();
        return;
      }

      if (isInput || isSearchOpen || isShortcutsOpen) return;

      if (e.key === '/' && !e.metaKey && !e.ctrlKey) {
        e.preventDefault();
        openSearch();
      } else if (e.key === '?' || (e.shiftKey && e.key === '/')) {
        e.preventDefault();
        if (shortcutsDlg) shortcutsDlg.open ? shortcutsDlg.close() : shortcutsDlg.showModal();
      } else if ((e.key === 't' || e.key === 'T') && !e.metaKey && !e.ctrlKey) {
        applyTheme(getTheme() === 'dark' ? 'light' : 'dark', true);
      } else if (!e.metaKey && !e.ctrlKey && !e.altKey) {
        const k = e.key.toLowerCase();
        if (k === 'h') location.href = '/';
        else if (k === 'd') location.href = '/docs/getting-started/';
        else if (k === 'p') location.href = '/projects/';
        else if (k === 'b') location.href = '/blog/';
        else if (k === 'l') location.href = '/books/';
      }
    });
  };

  /* ---- 8. Tactile Bento Card Spotlight Physics ---- */
  const initSpotlight = () => {
    if (!window.matchMedia('(hover: hover) and (pointer: fine)').matches) return;
    doc.querySelectorAll('.bento__card, .card, .book-card').forEach((card) => {
      card.addEventListener('pointermove', (e) => {
        const rect = card.getBoundingClientRect();
        card.style.setProperty('--mouse-x', `${e.clientX - rect.left}px`);
        card.style.setProperty('--mouse-y', `${e.clientY - rect.top}px`);
      });
    });
  };

  /* ---- 9. Scroll Progress Fallback (for engines without CSS scroll()) ---- */
  const initProgressFallback = () => {
    if (window.CSS && CSS.supports && CSS.supports('animation-timeline', 'scroll()')) return;
    const bar = doc.querySelector('.reading-progress');
    if (!bar) return;
    window.addEventListener('scroll', () => {
      const h = root.scrollHeight - window.innerHeight;
      bar.style.transform = `scaleX(${h > 0 ? window.scrollY / h : 0})`;
    }, { passive: true });
  };

  /* ---- 10. Native Web Share & Back To Top ---- */
  const initActions = () => {
    doc.querySelectorAll('[data-share-btn]').forEach((btn) => {
      btn.addEventListener('click', async () => {
        if (navigator.share) {
          try {
            await navigator.share({ title: doc.title, url: location.href });
            return;
          } catch (_) {}
        }
        await copyText(location.href);
        btn.classList.add('is-copied');
        setTimeout(() => btn.classList.remove('is-copied'), 2000);
      });
    });

    doc.querySelectorAll('.footer__back-to-top').forEach((btn) => {
      btn.addEventListener('click', (e) => {
        e.preventDefault();
        window.scrollTo({ top: 0, behavior: 'smooth' });
      });
    });

    doc.querySelectorAll('[data-print-btn]').forEach((btn) => {
      btn.addEventListener('click', () => window.print());
    });

    const banner = doc.querySelector('[data-announcement]');
    const dismiss = doc.querySelector('[data-announcement-dismiss]');
    if (banner && dismiss) {
      dismiss.addEventListener('click', () => {
        try {
          localStorage.setItem('announcement-dismissed', banner.getAttribute('data-announcement'));
        } catch (_) {}
        banner.remove();
      });
    }
  };

  /* ---- 11. Service Worker Registration ---- */
  const initSW = () => {
    if ('serviceWorker' in navigator && (location.protocol === 'https:' || location.hostname === 'localhost' || location.hostname === '127.0.0.1')) {
      window.addEventListener('load', () => navigator.serviceWorker.register('/sw.js').catch(() => {}));
    }
  };

  /* ---- Launch All Systems ---- */
  initTheme();
  initPageProgress();
  initDrawers();
  initProse();
  initProjectTabs();
  initFilterChips();
  const openSearch = initSearch() || (() => {});
  initShortcuts(openSearch);
  initSpotlight();
  initProgressFallback();
  initActions();
  initSW();
})();

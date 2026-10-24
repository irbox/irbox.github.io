/* ==========================================================================
   Folio / Irbox — Core Client Architecture
   Modular, dependency-free enhancements adhering to modern ECMAScript (ES2026).
   ========================================================================== */
(() => {
  'use strict';

  const doc = document;
  const root = doc.documentElement;
  root.classList.add('js');

  /* ---- Module 1: DOM & Storage Helpers ---- */
  const $ = (selector, context = doc) => context.querySelector(selector);
  const $$ = (selector, context = doc) => Array.from(context.querySelectorAll(selector));
  const store = {
    get: (key) => {
      try {
        return localStorage.getItem(key);
      } catch {
        return null;
      }
    },
    set: (key, value) => {
      try {
        localStorage.setItem(key, value);
      } catch {}
    },
  };

  const onReady = () => {
    requestAnimationFrame(() => {
      root.classList.remove('is-loading');
    });
  };
  if (doc.readyState === 'loading') {
    doc.addEventListener('DOMContentLoaded', onReady);
  } else {
    onReady();
  }

  /* ---- Module 2: Page Navigation Progress Indicator ---- */
  const initPageProgress = () => {
    const progress = $('#page-progress');
    if (!progress) return;

    const resetProgress = () => {
      progress.classList.remove('is-loading');
      progress.classList.add('is-done');
      setTimeout(() => {
        progress.classList.remove('is-done');
      }, 400);
    };

    window.addEventListener('pageshow', resetProgress);
    window.addEventListener('popstate', resetProgress);

    doc.addEventListener('click', (e) => {
      const link = e.target.closest('a');
      if (!link || !link.href || link.target === '_blank' || link.hasAttribute('download')) return;
      if (e.metaKey || e.ctrlKey || e.shiftKey || e.altKey || e.defaultPrevented) return;

      let url;
      try {
        url = new URL(link.href, location.href);
      } catch {
        return;
      }
      if (url.origin !== location.origin) return;
      if (url.pathname === location.pathname && url.search === location.search) return;

      progress.classList.remove('is-done');
      progress.classList.add('is-loading');
    });
  };

  /* ---- Module 3: Image Load Transitions ---- */
  const initImageTransitions = () => {
    $$('img.thumb, img.avatar').forEach((img) => {
      if (img.complete && img.naturalWidth > 0) {
        img.classList.add('is-loaded');
      } else {
        const markLoaded = () => img.classList.add('is-loaded');
        img.addEventListener('load', markLoaded, { once: true });
        img.addEventListener('error', markLoaded, { once: true });
      }
    });
  };

  /* ---- Module 4: Theme Toggle Controller ---- */
  const initThemeToggle = () => {
    const themeBtns = $$('[data-theme-toggle]');
    themeBtns.forEach((btn) => {
      btn.addEventListener('click', () => {
        const current =
          root.getAttribute('data-theme') ||
          (matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light');
        const next = current === 'dark' ? 'light' : 'dark';
        root.setAttribute('data-theme', next);
        store.set('theme', next);
      });
    });

    // Synchronize if system OS color scheme changes and user has not set an explicit override
    try {
      const colorMedia = window.matchMedia('(prefers-color-scheme: dark)');
      colorMedia.addEventListener('change', () => {
        if (!store.get('theme')) {
          root.removeAttribute('data-theme');
        }
      });
    } catch {}

    // Multi-tab synchronization
    window.addEventListener('storage', (e) => {
      if (e.key === 'theme') {
        if (e.newValue === 'light' || e.newValue === 'dark') {
          root.setAttribute('data-theme', e.newValue);
        } else {
          root.removeAttribute('data-theme');
        }
      }
    });
  };

  /* ---- Module 5: Mobile Navigation (Drawer & Sidebar) ---- */
  const initMobileNav = () => {
    // Drawer
    const drawerBtn = $('[data-drawer-toggle]');
    const drawer = $('#drawer');
    if (drawerBtn && drawer) {
      drawer.removeAttribute('hidden');
      const setDrawer = (open) => {
        drawer.classList.toggle('is-open', open);
        drawerBtn.setAttribute('aria-expanded', String(open));
        drawerBtn.setAttribute('aria-label', open ? 'Close menu' : 'Open menu');
      };
      drawerBtn.addEventListener('click', () => {
        setDrawer(!drawer.classList.contains('is-open'));
      });
      $$('a', drawer).forEach((link) => {
        link.addEventListener('click', () => setDrawer(false));
      });
      doc.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && drawer.classList.contains('is-open')) {
          setDrawer(false);
          drawerBtn.focus();
        }
      });
      doc.addEventListener('click', (e) => {
        if (drawer.classList.contains('is-open') && !e.target.closest('#topnav')) {
          setDrawer(false);
        }
      });
      window.addEventListener(
        'resize',
        () => {
          if (window.innerWidth > 767 && drawer.classList.contains('is-open')) {
            setDrawer(false);
          }
        },
        { passive: true }
      );
    }

    // Sidebar
    const sidebar = $('#sidebar');
    const sideBtns = $$('[data-sidebar-toggle]');
    if (sidebar) {
      const setSidebar = (open) => {
        sidebar.classList.toggle('is-open', open);
        sideBtns.forEach((btn) => {
          btn.setAttribute('aria-expanded', String(open));
          btn.setAttribute('aria-label', open ? 'Close sidebar' : 'Open sidebar');
        });
      };
      sideBtns.forEach((btn) => {
        btn.addEventListener('click', (e) => {
          e.stopPropagation();
          setSidebar(!sidebar.classList.contains('is-open'));
        });
      });
      $$('a', sidebar).forEach((link) => {
        link.addEventListener('click', () => {
          if (window.innerWidth <= 767) {
            setSidebar(false);
          }
        });
      });
      doc.addEventListener('click', (e) => {
        if (sidebar.classList.contains('is-open')) {
          if (!sidebar.contains(e.target) && !e.target.closest('[data-sidebar-toggle]')) {
            setSidebar(false);
          }
        }
      });
      doc.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && sidebar.classList.contains('is-open')) {
          setSidebar(false);
          if (sideBtns[0]) sideBtns[0].focus();
        }
      });

      // Restore and save scroll position
      try {
        const savedScroll = sessionStorage.getItem('sidebar-scroll');
        if (savedScroll) sidebar.scrollTop = +savedScroll;
        const activeItem = $('a.is-active', sidebar);
        if (activeItem && !savedScroll) activeItem.scrollIntoView({ block: 'center' });
        window.addEventListener('pagehide', () => {
          sessionStorage.setItem('sidebar-scroll', sidebar.scrollTop);
        });
      } catch {}
    }
  };

  /* ---- Module 6: Announcement Banner Controller ---- */
  const initAnnouncement = () => {
    const banner = $('[data-announcement]');
    const dismiss = $('[data-announcement-dismiss]');
    if (!banner || !dismiss) return;

    dismiss.addEventListener('click', () => {
      store.set('announcement-dismissed', banner.getAttribute('data-announcement'));
      banner.classList.add('is-dismissing');
      let finalized = false;
      const finalize = () => {
        if (finalized) return;
        finalized = true;
        root.setAttribute('data-announcement-hidden', '');
      };
      banner.addEventListener('transitionend', finalize, { once: true });
      setTimeout(finalize, 280);
    });
  };

  /* ---- Module 7: Prose (Heading Anchors & Copy Buttons) ---- */
  const copyText = async (text) => {
    if (navigator.clipboard && window.isSecureContext) {
      return navigator.clipboard.writeText(text);
    }
    const ta = doc.createElement('textarea');
    ta.value = text;
    ta.style.position = 'fixed';
    ta.style.opacity = '0';
    doc.body.appendChild(ta);
    ta.focus();
    ta.select();
    try {
      const ok = doc.execCommand('copy');
      if (!ok) throw new Error('Copy failed');
    } finally {
      doc.body.removeChild(ta);
    }
  };

  const initProse = () => {
    // Responsive table wrappers
    $$('[data-prose] table').forEach((table) => {
      if (!table.parentElement.classList.contains('table-wrap')) {
        const wrap = doc.createElement('div');
        wrap.className = 'table-wrap';
        table.parentNode.insertBefore(wrap, table);
        wrap.appendChild(table);
      }
    });

    // Heading anchor links
    $$('[data-prose] :is(h2,h3,h4)[id]').forEach((h) => {
      const a = doc.createElement('a');
      a.className = 'anchor';
      a.href = '#' + h.id;
      a.setAttribute('aria-label', 'Link to this section');
      a.textContent = '#';
      h.appendChild(a);
    });

    // Code copy buttons
    $$('[data-prose] pre').forEach((pre) => {
      const b = doc.createElement('button');
      b.type = 'button';
      b.className = 'copy-btn';
      b.textContent = 'Copy';
      b.setAttribute('aria-label', 'Copy code to clipboard');
      b.addEventListener('click', async () => {
        const code = $('code', pre);
        const text = code
          ? code.innerText
          : pre.innerText.replace(/\bCopy\b|\bCopied\b/g, '').trim();
        try {
          await copyText(text);
          b.textContent = 'Copied';
          b.classList.add('is-copied');
          b.setAttribute('aria-label', 'Code copied to clipboard');
          setTimeout(() => {
            b.textContent = 'Copy';
            b.classList.remove('is-copied');
            b.setAttribute('aria-label', 'Copy code to clipboard');
          }, 1800);
        } catch {
          b.textContent = 'Failed';
          setTimeout(() => {
            b.textContent = 'Copy';
          }, 1800);
        }
      });
      pre.appendChild(b);
    });
  };

  /* ---- Module 8: Project Tabs & View Transitions ---- */
  let outlineObserver;
  const updateOutline = () => {
    const aside = $('[data-outline]');
    const mobile = $('[data-outline-mobile]');
    if (!aside) return;

    const heads = $$('[data-prose] :is(h2,h3)[id]').filter((h) => !h.closest('[hidden]'));
    const list = $('ul', aside);
    list.innerHTML = '';

    if (heads.length < 2) {
      aside.setAttribute('hidden', '');
      if (mobile) mobile.setAttribute('hidden', '');
      return;
    }

    aside.removeAttribute('hidden');
    const getCleanTitle = (heading) => {
      const clone = heading.cloneNode(true);
      const anchor = clone.querySelector('.anchor');
      if (anchor) anchor.remove();
      return clone.textContent.trim();
    };

    const links = heads.map((h) => {
      const li = doc.createElement('li');
      const a = doc.createElement('a');
      a.href = '#' + h.id;
      a.textContent = getCleanTitle(h);
      if (h.tagName === 'H3') a.className = 'is-sub';
      a.addEventListener('click', (e) => {
        e.preventDefault();
        const target = doc.getElementById(h.id);
        if (target) {
          target.scrollIntoView({ behavior: 'smooth', block: 'start' });
          if (history.replaceState) {
            history.replaceState(null, '', '#' + h.id);
          }
        }
      });
      li.appendChild(a);
      list.appendChild(li);
      return a;
    });

    if (mobile) {
      mobile.removeAttribute('hidden');
      mobile.innerHTML = '';
      const select = doc.createElement('select');
      select.setAttribute('aria-label', 'On this page');

      const defaultOpt = doc.createElement('option');
      defaultOpt.value = '';
      defaultOpt.textContent = 'On this page…';
      select.appendChild(defaultOpt);

      heads.forEach((h) => {
        const opt = doc.createElement('option');
        opt.value = h.id;
        opt.textContent = (h.tagName === 'H3' ? '— ' : '') + getCleanTitle(h);
        select.appendChild(opt);
      });

      select.addEventListener('change', (e) => {
        if (e.target.value) {
          const target = doc.getElementById(e.target.value);
          if (target) {
            target.scrollIntoView({ behavior: 'smooth', block: 'start' });
            if (history.replaceState) {
              history.replaceState(null, '', '#' + e.target.value);
            }
          }
        }
      });
      mobile.appendChild(select);
    }

    if (outlineObserver) outlineObserver.disconnect();
    if ('IntersectionObserver' in window) {
      outlineObserver = new IntersectionObserver(
        (entries) => {
          entries.forEach((entry) => {
            if (entry.isIntersecting) {
              links.forEach((link) => {
                link.classList.toggle(
                  'is-active',
                  link.getAttribute('href') === '#' + entry.target.id
                );
              });
              if (mobile) {
                const sel = $('select', mobile);
                if (sel) sel.value = entry.target.id;
              }
            }
          });
        },
        { rootMargin: '-80px 0px -70% 0px' }
      );
      heads.forEach((h) => outlineObserver.observe(h));
    }
  };

  const initProjectTabs = () => {
    const tabs = $$('[data-tabs] [data-tab]');
    if (!tabs.length) return;

    const applyTab = (name) => {
      tabs.forEach((t) => {
        const on = t.getAttribute('data-tab') === name;
        t.classList.toggle('is-active', on);
        t.setAttribute('aria-selected', String(on));
      });
      $$('.tab-panel').forEach((p) => {
        const on = p.getAttribute('data-panel') === name;
        p.toggleAttribute('hidden', !on);
        if (on) {
          p.classList.remove('is-animating');
          void p.offsetWidth;
          p.classList.add('is-animating');
        }
      });
      updateOutline();
    };

    const showTab = (name, updateUrl) => {
      if (
        doc.startViewTransition &&
        !window.matchMedia('(prefers-reduced-motion: reduce)').matches
      ) {
        doc.startViewTransition(() => applyTab(name));
      } else {
        applyTab(name);
      }
      if (updateUrl && history.replaceState) {
        history.replaceState(null, '', '#' + name);
      }
    };

    tabs.forEach((t) => {
      t.addEventListener('click', () => showTab(t.getAttribute('data-tab'), true));
    });

    window.addEventListener('hashchange', () => {
      const hashTab = location.hash.replace('#', '');
      if ($(`[data-tab="${hashTab}"]`)) {
        showTab(hashTab, false);
      }
    });

    let initialTab = 'overview';
    if (location.hash) {
      const hashTab = location.hash.replace('#', '');
      if ($(`[data-tab="${hashTab}"]`)) initialTab = hashTab;
    }
    showTab(initialTab, false);
  };

  /* ---- Module 9: Project Filter Chips Controller ---- */
  const initFilterChips = () => {
    const chips = $$('[data-filter]');
    if (!chips.length) return;

    chips.forEach((chip) => {
      chip.addEventListener('click', (e) => {
        const filter = chip.getAttribute('data-filter');
        chips.forEach((x) => {
          const on = x === chip;
          x.classList.toggle('is-active', on);
          x.setAttribute('aria-pressed', String(on));
        });
        $$('[data-category-section]').forEach((section) => {
          const match =
            filter === 'all' || section.getAttribute('data-category-section') === filter;
          section.classList.toggle('is-hidden', !match);
          if (match) {
            section.classList.remove('is-animating');
            void section.offsetWidth;
            section.classList.add('is-animating');
          }
        });
        if (e?.isTrusted && history.replaceState) {
          history.replaceState(null, '', filter === 'all' ? location.pathname : '#' + filter);
        }
      });
    });

    window.addEventListener('hashchange', () => {
      const targetFilter = location.hash.replace('#', '') || 'all';
      const match = $(`[data-filter="${targetFilter}"]`);
      if (match && !match.classList.contains('is-active')) {
        match.click();
      }
    });

    if (location.hash) {
      const targetFilter = location.hash.replace('#', '');
      const match = $(`[data-filter="${targetFilter}"]`);
      if (match) match.click();
    }
  };

  /* ---- Module 10: Search Dialog Controller ---- */
  const searchDlg = $('#search-dialog');
  const searchInput = $('#search-input');
  const searchResults = $('#search-results');
  let searchIndex = null;
  let searchSelected = -1;
  let searchLastFocus = null;
  let searchDebounceTimer = null;

  const loadSearchIndex = () => {
    if (searchIndex) return Promise.resolve(searchIndex);
    return fetch(searchDlg.getAttribute('data-index'))
      .then((r) => r.json())
      .then(
        (data) => {
          searchIndex = data;
          return data;
        },
        () => {
          searchIndex = [];
          return searchIndex;
        }
      );
  };

  const renderSearchResults = (query) => {
    query = query.trim().toLowerCase();
    const words = query.split(/\s+/).filter(Boolean);
    const hits = (searchIndex || [])
      .map((item) => {
        const title = String(item.title || '').toLowerCase();
        const desc = String(item.description || '').toLowerCase();
        const tags = (
          Array.isArray(item.tags) ? item.tags.join(' ') : String(item.tags || '')
        ).toLowerCase();
        let score = 0;
        if (!words.length) return { item, score: 1 };
        for (let i = 0; i < words.length; i++) {
          const w = words[i];
          if (title.indexOf(w) === 0) score += 8;
          else if (title.indexOf(w) > -1) score += 5;
          else if (tags.indexOf(w) > -1) score += 3;
          else if (desc.indexOf(w) > -1) score += 1;
          else return { item, score: 0 };
        }
        return { item, score };
      })
      .filter((h) => h.score > 0)
      .sort((a, b) => b.score - a.score)
      .slice(0, 8);

    searchSelected = hits.length ? 0 : -1;
    const status = $('#search-status');
    if (status) {
      status.textContent = hits.length
        ? `${hits.length} result${hits.length === 1 ? '' : 's'} found`
        : 'No results found';
    }

    const escapeHtml = (str) =>
      String(str || '').replace(
        /[&<>"'`]/g,
        (c) =>
          ({
            '&': '&amp;',
            '<': '&lt;',
            '>': '&gt;',
            '"': '&quot;',
            "'": '&#39;',
            '`': '&#96;',
          })[c]
      );

    const sanitizeUrl = (u) => {
      const s = String(u || '').trim();
      return /^(javascript|data|vbscript):/i.test(s) ? '#' : escapeHtml(s);
    };

    const highlight = (text, queryWords) => {
      const str = String(text || '');
      if (!queryWords || !queryWords.length) return escapeHtml(str);
      const validWords = queryWords
        .filter(Boolean)
        .map((w) => w.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'))
        .sort((a, b) => b.length - a.length);
      if (!validWords.length) return escapeHtml(str);

      const combinedRegex = new RegExp(`(${validWords.join('|')})`, 'gi');
      let result = '';
      let lastIndex = 0;
      let match;

      while ((match = combinedRegex.exec(str)) !== null) {
        if (match.index > lastIndex) {
          result += escapeHtml(str.slice(lastIndex, match.index));
        }
        result += `<mark>${escapeHtml(match[0])}</mark>`;
        lastIndex = combinedRegex.lastIndex;
        if (match.index === combinedRegex.lastIndex) {
          combinedRegex.lastIndex++;
        }
      }
      if (lastIndex < str.length) {
        result += escapeHtml(str.slice(lastIndex));
      }
      return result;
    };

    searchResults.innerHTML = hits.length
      ? hits
          .map((h, idx) => {
            const e = h.item;
            return (
              `<li><a href="${sanitizeUrl(e.url)}"${idx === 0 ? ' class="is-selected"' : ''}>` +
              `<span class="meta">${escapeHtml(e.type)}</span><strong>${highlight(e.title, words)}</strong>` +
              (e.description
                ? `<span class="desc">${highlight(e.description, words)}</span>`
                : '') +
              '</a></li>'
            );
          })
          .join('')
      : '<li class="search__empty">No results found</li>';
  };

  const openSearch = () => {
    if (!searchDlg?.showModal) return;
    searchLastFocus = doc.activeElement;
    if (!searchDlg.open) searchDlg.showModal();
    searchInput.value = '';
    if (!searchIndex) {
      searchResults.innerHTML =
        '<li class="search__loading"><span class="spinner" aria-hidden="true"></span><span>Loading search index…</span></li>';
    }
    loadSearchIndex().then(() => renderSearchResults(''));
    searchInput.focus();
  };

  const closeSearch = () => {
    if (searchDlg?.open) {
      searchDlg.close();
      if (searchLastFocus && typeof searchLastFocus.focus === 'function') {
        searchLastFocus.focus();
        searchLastFocus = null;
      }
    }
  };

  const moveSearchSelection = (delta) => {
    const items = $$('a', searchResults);
    if (!items.length) return;
    items.forEach((a) => a.classList.remove('is-selected'));
    searchSelected = (searchSelected + delta + items.length) % items.length;
    items[searchSelected].classList.add('is-selected');
    items[searchSelected].scrollIntoView({ block: 'nearest' });
  };

  const initSearch = () => {
    if (!searchDlg) return;

    const isMac = /(Mac|iPhone|iPod|iPad)/i.test(navigator.platform || navigator.userAgent);
    const kbd = $('.search-trigger kbd');
    if (kbd && isMac) kbd.textContent = '⌘K';

    let isBackdrop = false;
    $$('[data-search-open]').forEach((btn) => btn.addEventListener('click', openSearch));
    $$('[data-search-close]').forEach((btn) => btn.addEventListener('click', closeSearch));
    searchDlg.addEventListener('pointerdown', (e) => {
      isBackdrop = e.target === searchDlg;
    });
    searchDlg.addEventListener('click', (e) => {
      if (e.target === searchDlg && isBackdrop) closeSearch();
    });
    searchDlg.addEventListener('close', () => {
      if (searchLastFocus && typeof searchLastFocus.focus === 'function') {
        searchLastFocus.focus();
        searchLastFocus = null;
      }
    });

    searchInput.addEventListener('input', () => {
      clearTimeout(searchDebounceTimer);
      searchDebounceTimer = setTimeout(() => {
        loadSearchIndex().then(() => renderSearchResults(searchInput.value));
      }, 40);
    });

    searchInput.addEventListener('keydown', (e) => {
      if (e.key === 'ArrowDown') {
        e.preventDefault();
        moveSearchSelection(1);
      } else if (e.key === 'ArrowUp') {
        e.preventDefault();
        moveSearchSelection(-1);
      } else if (e.key === 'Home') {
        e.preventDefault();
        const items = $$('a', searchResults);
        if (items.length) {
          items.forEach((a) => a.classList.remove('is-selected'));
          searchSelected = 0;
          items[0].classList.add('is-selected');
          items[0].scrollIntoView({ block: 'nearest' });
        }
      } else if (e.key === 'End') {
        e.preventDefault();
        const items = $$('a', searchResults);
        if (items.length) {
          items.forEach((a) => a.classList.remove('is-selected'));
          searchSelected = items.length - 1;
          items[searchSelected].classList.add('is-selected');
          items[searchSelected].scrollIntoView({ block: 'nearest' });
        }
      } else if (e.key === 'Enter') {
        e.preventDefault();
        const selectedItem = $$('a', searchResults)[searchSelected];
        if (selectedItem) {
          closeSearch();
          location.href = selectedItem.href;
        }
      }
    });

    searchResults.addEventListener('mousemove', (e) => {
      const targetLink = e.target.closest('a');
      if (!targetLink) return;
      const items = $$('a', searchResults);
      const idx = items.indexOf(targetLink);
      if (idx !== -1 && idx !== searchSelected) {
        items.forEach((el) => el.classList.remove('is-selected'));
        searchSelected = idx;
        targetLink.classList.add('is-selected');
      }
    });

    doc.addEventListener('keydown', (e) => {
      if ((e.metaKey || e.ctrlKey) && !e.shiftKey && !e.altKey && e.key.toLowerCase() === 'k') {
        e.preventDefault();
        searchDlg.open ? closeSearch() : openSearch();
      }
    });
  };

  /* ---- Module 11: Shortcuts Dialog & Global Quick Keys ---- */
  const shortcutsDlg = $('#shortcuts-dialog');
  let shortcutsLastFocus = null;
  const openShortcuts = () => {
    if (!shortcutsDlg?.showModal) return;
    shortcutsLastFocus = doc.activeElement;
    if (!shortcutsDlg.open) shortcutsDlg.showModal();
  };
  const closeShortcuts = () => {
    if (shortcutsDlg?.open) {
      shortcutsDlg.close();
      if (shortcutsLastFocus && typeof shortcutsLastFocus.focus === 'function') {
        shortcutsLastFocus.focus();
        shortcutsLastFocus = null;
      }
    }
  };

  const initShortcuts = () => {
    if (shortcutsDlg) {
      let isBackdrop = false;
      $$('[data-shortcuts-open]').forEach((b) => b.addEventListener('click', openShortcuts));
      $$('[data-shortcuts-close]').forEach((b) => b.addEventListener('click', closeShortcuts));
      shortcutsDlg.addEventListener('pointerdown', (e) => {
        isBackdrop = e.target === shortcutsDlg;
      });
      shortcutsDlg.addEventListener('click', (e) => {
        if (e.target === shortcutsDlg && isBackdrop) closeShortcuts();
      });
      shortcutsDlg.addEventListener('close', () => {
        if (shortcutsLastFocus && typeof shortcutsLastFocus.focus === 'function') {
          shortcutsLastFocus.focus();
          shortcutsLastFocus = null;
        }
      });
      const isMac = /(Mac|iPhone|iPod|iPad)/i.test(navigator.platform || navigator.userAgent);
      if (isMac) {
        $$('kbd', shortcutsDlg).forEach((kbd) => {
          if (kbd.textContent.trim() === 'Ctrl') kbd.textContent = '⌘';
        });
      }
    }

    doc.addEventListener('keydown', (e) => {
      const isInput = Boolean(
        e.target?.closest?.('input, textarea, select, [contenteditable="true"]') ||
        e.target?.isContentEditable
      );
      const isModalOpen = Boolean(searchDlg?.open || shortcutsDlg?.open);

      // Search slash key '/'
      if (!isInput && !isModalOpen && e.key === '/' && !e.metaKey && !e.ctrlKey && !e.altKey) {
        e.preventDefault();
        openSearch();
        return;
      }

      // Question mark '?' for shortcuts sheet
      if (!isInput && (e.key === '?' || (e.shiftKey && e.key === '/'))) {
        e.preventDefault();
        if (shortcutsDlg) {
          shortcutsDlg.open ? closeShortcuts() : openShortcuts();
        }
        return;
      }

      // If any modal is open, don't execute single-character navigation hotkeys
      if (isModalOpen || isInput) return;

      // 't' or 'T' for theme toggle
      if ((e.key === 't' || e.key === 'T') && !e.metaKey && !e.ctrlKey && !e.altKey) {
        const firstThemeBtn = $('[data-theme-toggle]');
        if (firstThemeBtn) firstThemeBtn.click();
        return;
      }

      // Quick navigation: H (Home), D (Docs), P (Projects), B (Blog)
      if (!e.metaKey && !e.ctrlKey && !e.altKey) {
        const k = e.key.toLowerCase();
        const base = root.getAttribute('data-baseurl') || '';
        if (k === 'h') {
          location.href = base || '/';
        } else if (k === 'd') {
          location.href = base + '/docs/getting-started/';
        } else if (k === 'p') {
          location.href = base + '/projects/';
        } else if (k === 'b') {
          location.href = base + '/blog/';
        }
      }
    });
  };

  /* ---- Module 12: Bento Card Tactile Spotlight Physics ---- */
  const initBentoSpotlight = () => {
    if (!window.matchMedia('(hover: hover) and (pointer: fine)').matches) return;

    $$('.bento__card').forEach((card) => {
      let rect = null;
      card.addEventListener('pointerenter', () => {
        rect = card.getBoundingClientRect();
      });
      card.addEventListener('pointermove', (e) => {
        if (!rect) rect = card.getBoundingClientRect();
        card.style.setProperty('--mouse-x', `${e.clientX - rect.left}px`);
        card.style.setProperty('--mouse-y', `${e.clientY - rect.top}px`);
      });
      card.addEventListener('pointerleave', () => {
        rect = null;
        card.style.removeProperty('--mouse-x');
        card.style.removeProperty('--mouse-y');
      });
    });
  };

  /* ---- Module 13: Universal Reading Progress Fallback ---- */
  const initReadingProgressFallback = () => {
    const bar = $('.reading-progress');
    if (!bar) return;
    if (window.CSS && CSS.supports && CSS.supports('animation-timeline', 'scroll()')) return;

    let ticking = false;
    const updateProgress = () => {
      const docHeight = root.scrollHeight - window.innerHeight;
      const progress = docHeight > 0 ? window.scrollY / docHeight : 0;
      bar.style.transform = `scaleX(${Math.min(1, Math.max(0, progress))})`;
      ticking = false;
    };

    const requestTick = () => {
      if (!ticking) {
        requestAnimationFrame(updateProgress);
        ticking = true;
      }
    };

    window.addEventListener('scroll', requestTick, { passive: true });
    window.addEventListener('resize', requestTick, { passive: true });
    updateProgress();
  };

  /* ---- Module 14: Back To Top Navigation ---- */
  const initBackToTop = () => {
    const topLink = $('.footer__back-to-top');
    const mainEl = $('#main');
    if (!topLink) return;

    topLink.addEventListener('click', (e) => {
      e.preventDefault();
      window.scrollTo({ top: 0, behavior: 'smooth' });
      if (mainEl) {
        mainEl.setAttribute('tabindex', '-1');
        mainEl.focus({ preventScroll: true });
      }
      if (history.replaceState) {
        history.replaceState(null, '', location.pathname + location.search);
      }
    });
  };

  /* ---- Module 15: Native Web Share & Fallback Clipboard Sharing ---- */
  const initShare = () => {
    $$('[data-share-btn]').forEach((btn) => {
      btn.addEventListener('click', async () => {
        const title = doc.title;
        const url = location.href;
        if (navigator.share) {
          try {
            await navigator.share({ title, url });
            return;
          } catch {}
        }
        try {
          await copyText(url);
          btn.classList.add('is-copied');
          setTimeout(() => {
            btn.classList.remove('is-copied');
          }, 2000);
        } catch {}
      });
    });
  };

  /* ---- Module 16: Progressive Web App (PWA) Service Worker ---- */
  const initServiceWorker = () => {
    if (
      'serviceWorker' in navigator &&
      (location.protocol === 'https:' ||
        location.hostname === 'localhost' ||
        location.hostname === '127.0.0.1')
    ) {
      window.addEventListener('load', () => {
        const base = root.getAttribute('data-baseurl') || '';
        navigator.serviceWorker.register(base + '/sw.js').catch(() => {});
      });
    }
  };

  /* ---- Module 17: Publication Print Controller ---- */
  const initPrint = () => {
    $$('[data-print-btn]').forEach((btn) => {
      btn.addEventListener('click', () => {
        window.print();
      });
    });
  };

  /* ---- Initialize All Controllers ---- */
  initPageProgress();
  initImageTransitions();
  initThemeToggle();
  initMobileNav();
  initAnnouncement();
  initProse();
  initProjectTabs();
  updateOutline();
  initFilterChips();
  initSearch();
  initShortcuts();
  initBentoSpotlight();
  initReadingProgressFallback();
  initBackToTop();
  initShare();
  initServiceWorker();
  initPrint();
})();

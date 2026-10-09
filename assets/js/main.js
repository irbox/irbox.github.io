/* Folio — small, dependency-free enhancements. The site works without JS;
   this adds: theme toggle, mobile menu/sidebar, announcement dismiss,
   "On this page" outline, heading anchors, copy buttons, project tabs,
   project filter chips, and the Ctrl/⌘+K search palette. */
(function () {
  'use strict';
  var doc = document, root = doc.documentElement;
  root.classList.add('js');
  var $ = function (s, c) { return (c || doc).querySelector(s); };
  var $$ = function (s, c) { return Array.prototype.slice.call((c || doc).querySelectorAll(s)); };
  var store = {
    get: function (k) { try { return localStorage.getItem(k); } catch (e) { return null; } },
    set: function (k, v) { try { localStorage.setItem(k, v); } catch (e) {} }
  };

  /* ---- Theme toggle ---- */
  var themeBtn = $('[data-theme-toggle]');
  if (themeBtn) themeBtn.addEventListener('click', function () {
    var current = root.getAttribute('data-theme') ||
      (matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light');
    var next = current === 'dark' ? 'light' : 'dark';
    root.setAttribute('data-theme', next);
    store.set('theme', next);
  });

  /* ---- Mobile nav drawer ---- */
  var drawerBtn = $('[data-drawer-toggle]'), drawer = $('#drawer');
  if (drawerBtn && drawer) drawerBtn.addEventListener('click', function () {
    var open = drawer.hasAttribute('hidden');
    drawer.toggleAttribute('hidden', !open);
    drawerBtn.setAttribute('aria-expanded', String(open));
  });

  /* ---- Mobile sidebar ---- */
  var sideBtn = $('[data-sidebar-toggle]'), sidebar = $('#sidebar');
  if (sideBtn && sidebar) sideBtn.addEventListener('click', function () {
    var open = sidebar.classList.toggle('is-open');
    sideBtn.setAttribute('aria-expanded', String(open));
  });
  /* Keep the sidebar scroll position between page loads. */
  if (sidebar) {
    try {
      var y = sessionStorage.getItem('sidebar-scroll');
      if (y) sidebar.scrollTop = +y;
      var active = $('a.is-active', sidebar);
      if (active && !y) active.scrollIntoView({ block: 'center' });
      window.addEventListener('pagehide', function () { sessionStorage.setItem('sidebar-scroll', sidebar.scrollTop); });
    } catch (e) {}
  }

  /* ---- Announcement dismiss ---- */
  var banner = $('[data-announcement]');
  var dismiss = $('[data-announcement-dismiss]');
  if (banner && dismiss) dismiss.addEventListener('click', function () {
    store.set('announcement-dismissed', banner.getAttribute('data-announcement'));
    root.setAttribute('data-announcement-hidden', '');
  });

  /* ---- Prose: heading anchors + copy buttons ---- */
  $$('[data-prose] :is(h2,h3,h4)[id]').forEach(function (h) {
    var a = doc.createElement('a');
    a.className = 'anchor'; a.href = '#' + h.id; a.setAttribute('aria-label', 'Link to this section'); a.textContent = '#';
    h.appendChild(a);
  });
  $$('[data-prose] pre').forEach(function (pre) {
    var b = doc.createElement('button');
    b.type = 'button'; b.className = 'copy-btn'; b.textContent = 'Copy';
    b.addEventListener('click', function () {
      var code = $('code', pre) || pre;
      (navigator.clipboard ? navigator.clipboard.writeText(code.innerText) : Promise.reject()).then(function () {
        b.textContent = 'Copied'; setTimeout(function () { b.textContent = 'Copy'; }, 1500);
      }, function () { b.textContent = 'Press Ctrl+C'; });
    });
    pre.appendChild(b);
  });

  /* ---- Project tabs (all panels stay visible without JS) ---- */
  var tabs = $$('[data-tabs] [data-tab]');
  function showTab(name) {
    tabs.forEach(function (t) {
      var on = t.getAttribute('data-tab') === name;
      t.classList.toggle('is-active', on); t.setAttribute('aria-selected', String(on));
    });
    $$('.tab-panel').forEach(function (p) { p.toggleAttribute('hidden', p.getAttribute('data-panel') !== name); });
    buildOutline();
  }
  if (tabs.length) {
    tabs.forEach(function (t) { t.addEventListener('click', function () { showTab(t.getAttribute('data-tab')); }); });
    showTab('overview');
  }

  /* ---- "On this page" outline (desktop aside + mobile select) ---- */
  var observer;
  function ht(h) { var c = h.cloneNode(true); var x = c.querySelector('.anchor'); if (x) x.remove(); return c.textContent.trim(); }
  function buildOutline() {
    var aside = $('[data-outline]'), mobile = $('[data-outline-mobile]');
    if (!aside) return;
    var heads = $$('[data-prose] :is(h2,h3)[id]').filter(function (h) { return !h.closest('[hidden]'); });
    var list = $('ul', aside); list.innerHTML = '';
    if (heads.length < 2) { aside.setAttribute('hidden', ''); if (mobile) mobile.setAttribute('hidden', ''); return; }
    aside.removeAttribute('hidden');
    var links = heads.map(function (h) {
      var li = doc.createElement('li'), a = doc.createElement('a');
      a.href = '#' + h.id; a.textContent = ht(h);
      if (h.tagName === 'H3') a.className = 'is-sub';
      li.appendChild(a); list.appendChild(li); return a;
    });
    if (mobile) {
      mobile.removeAttribute('hidden');
      mobile.innerHTML = '<select aria-label="On this page"><option value="">On this page…</option>' +
        heads.map(function (h) { return '<option value="' + h.id + '">' + (h.tagName === 'H3' ? '— ' : '') + ht(h).replace(/</g, '&lt;') + '</option>'; }).join('') + '</select>';
      $('select', mobile).addEventListener('change', function (e) { if (e.target.value) location.hash = e.target.value; });
    }
    if (observer) observer.disconnect();
    if ('IntersectionObserver' in window) {
      observer = new IntersectionObserver(function (entries) {
        entries.forEach(function (en) {
          if (en.isIntersecting) {
            links.forEach(function (l) { l.classList.toggle('is-active', l.getAttribute('href') === '#' + en.target.id); });
          }
        });
      }, { rootMargin: '-80px 0px -70% 0px' });
      heads.forEach(function (h) { observer.observe(h); });
    }
  }
  buildOutline();

  /* ---- Project filter chips ---- */
  var chips = $$('[data-filter]');
  chips.forEach(function (c) {
    c.addEventListener('click', function () {
      var f = c.getAttribute('data-filter');
      chips.forEach(function (x) { x.classList.toggle('is-active', x === c); });
      $$('[data-category-section]').forEach(function (s) {
        s.classList.toggle('is-hidden', f !== 'all' && s.getAttribute('data-category-section') !== f);
      });
    });
  });

  /* ---- Search palette ---- */
  var dlg = $('#search-dialog'), input = $('#search-input'), results = $('#search-results');
  var index = null, sel = -1;
  function loadIndex() {
    if (index) return Promise.resolve(index);
    return fetch(dlg.getAttribute('data-index')).then(function (r) { return r.json(); })
      .then(function (d) { index = d; return d; }, function () { index = []; return index; });
  }
  function render(q) {
    q = q.trim().toLowerCase();
    var words = q.split(/\s+/).filter(Boolean);
    var hits = (index || []).map(function (e) {
      var t = e.title.toLowerCase(), d = (e.description || '').toLowerCase(), s = 0;
      if (!words.length) return { e: e, s: 1 };
      for (var i = 0; i < words.length; i++) {
        if (t.indexOf(words[i]) === 0) s += 6; else if (t.indexOf(words[i]) > -1) s += 4; else if (d.indexOf(words[i]) > -1) s += 1; else return { e: e, s: 0 };
      }
      return { e: e, s: s };
    }).filter(function (h) { return h.s > 0; }).sort(function (a, b) { return b.s - a.s; }).slice(0, 8);
    sel = hits.length ? 0 : -1;
    results.innerHTML = hits.length ? hits.map(function (h, i) {
      var e = h.e, esc = function (x) { return String(x).replace(/[&<>"]/g, function (c) { return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]; }); };
      return '<li role="option"><a href="' + esc(e.url) + '"' + (i === 0 ? ' class="is-selected"' : '') + '><span class="meta">' + esc(e.type) + '</span><strong>' + esc(e.title) + '</strong>' + (e.description ? '<span class="desc">' + esc(e.description) + '</span>' : '') + '</a></li>';
    }).join('') : '<li class="search__empty">No results</li>';
  }
  function openSearch() {
    if (!dlg || !dlg.showModal) return;
    if (!dlg.open) dlg.showModal();
    input.value = ''; loadIndex().then(function () { render(''); }); input.focus();
  }
  function closeSearch() { if (dlg && dlg.open) dlg.close(); }
  function move(d) {
    var items = $$('a', results); if (!items.length) return;
    items.forEach(function (a) { a.classList.remove('is-selected'); });
    sel = (sel + d + items.length) % items.length;
    items[sel].classList.add('is-selected'); items[sel].scrollIntoView({ block: 'nearest' });
  }
  if (dlg) {
    $$('[data-search-open]').forEach(function (b) { b.addEventListener('click', openSearch); });
    $$('[data-search-close]').forEach(function (b) { b.addEventListener('click', closeSearch); });
    dlg.addEventListener('click', function (e) { if (e.target === dlg) closeSearch(); });
    input.addEventListener('input', function () { loadIndex().then(function () { render(input.value); }); });
    input.addEventListener('keydown', function (e) {
      if (e.key === 'ArrowDown') { e.preventDefault(); move(1); }
      else if (e.key === 'ArrowUp') { e.preventDefault(); move(-1); }
      else if (e.key === 'Enter') { var a = $$('a', results)[sel]; if (a) location.href = a.href; }
    });
    doc.addEventListener('keydown', function (e) {
      if ((e.metaKey || e.ctrlKey) && !e.shiftKey && !e.altKey && e.key.toLowerCase() === 'k') {
        e.preventDefault(); dlg.open ? closeSearch() : openSearch();
      }
    });
  }
})();

/**
 * Irbox Web Book Reader
 * Zero-dependency, client-side EPUB and PDF reader.
 * Uses native DecompressionStream and Web APIs for instant, private reading.
 */

(function () {
  'use strict';

  // DOM Elements
  const shell = document.getElementById('reader-shell');
  const bookTitleEl = document.getElementById('reader-book-title');
  const chapterTitleEl = document.getElementById('reader-chapter-title');
  const progressPill = document.getElementById('reader-progress-pill');
  const progressBar = document.getElementById('reader-progress-bar');
  const loadingEl = document.getElementById('reader-loading');
  const loadingText = document.getElementById('reader-loading-text');
  const errorEl = document.getElementById('reader-error');
  const errorTitle = document.getElementById('reader-error-title');
  const errorMsg = document.getElementById('reader-error-message');
  const retryBtn = document.getElementById('reader-retry-btn');

  // EPUB View Elements
  const epubContainer = document.getElementById('reader-epub-container');
  const canvas = document.getElementById('reader-canvas');
  const prevBtn = document.getElementById('reader-prev-chapter');
  const nextBtn = document.getElementById('reader-next-chapter');
  const chapterCountBottom = document.getElementById('reader-chapter-count-bottom');

  // PDF View Elements
  const pdfContainer = document.getElementById('reader-pdf-container');
  const pdfObject = document.getElementById('reader-pdf-object');
  const pdfEmbed = document.getElementById('reader-pdf-embed');
  const pdfTitle = document.getElementById('reader-pdf-title');
  const pdfDownload = document.getElementById('reader-pdf-download');
  const pdfExternal = document.getElementById('reader-pdf-external');
  const pdfFallbackOpen = document.getElementById('reader-pdf-fallback-open');
  const pdfFallbackDownload = document.getElementById('reader-pdf-fallback-download');
  const pdfMobileView = document.getElementById('reader-pdf-mobile-view');
  const pdfMobileTitle = document.getElementById('reader-pdf-mobile-title');
  const pdfMobileOpen = document.getElementById('reader-pdf-mobile-open');
  const pdfMobileDownload = document.getElementById('reader-pdf-mobile-download');

  // Settings & TOC Elements
  const tocToggle = document.getElementById('reader-toc-toggle');
  const tocDrawer = document.getElementById('reader-toc-drawer');
  const tocClose = document.getElementById('reader-toc-close');
  const tocList = document.getElementById('reader-toc-list');
  const settingsToggle = document.getElementById('reader-settings-toggle');
  const settingsPopover = document.getElementById('reader-settings-popover');
  const fullscreenToggle = document.getElementById('reader-fullscreen-toggle');
  const fontDecBtn = document.getElementById('reader-font-dec');
  const fontIncBtn = document.getElementById('reader-font-inc');
  const fontDisplay = document.getElementById('reader-font-display');

  // Reader State
  let bookData = null;
  let bookType = null; // 'epub' | 'pdf'
  let bookFilename = '';
  let bookStorageKey = '';
  let currentChapterIndex = 0;
  let epubSpine = []; // Array of { id, href, fullPath }
  let epubManifest = {}; // id -> { id, href, mediaType, fullPath }
  let epubToc = []; // Array of { title, href, fullPath }
  let opfDir = '';
  let zipEntries = {};
  let zipBuffer = null;
  let createdObjectUrls = [];

  const safeStore = {
    get: (key, fallback = null) => {
      try {
        const val = localStorage.getItem(key);
        return val !== null ? val : fallback;
      } catch {
        return fallback;
      }
    },
    set: (key, val) => {
      try {
        localStorage.setItem(key, val);
      } catch {}
    },
  };

  // User Preferences
  const prefs = {
    theme: safeStore.get('irbox_reader_theme', 'sepia'),
    font: safeStore.get('irbox_reader_font', 'serif'),
    size: parseInt(safeStore.get('irbox_reader_size', '18'), 10),
    lh: safeStore.get('irbox_reader_lh', 'normal'),
  };

  function applyPreferences() {
    if (!shell) return;
    shell.setAttribute('data-theme', prefs.theme);
    shell.setAttribute('data-font', prefs.font);
    shell.setAttribute('data-lh', prefs.lh);
    document.documentElement.style.setProperty('--reader-font-size', prefs.size + 'px');
    shell.style.setProperty('--reader-font-size', prefs.size + 'px');

    if (fontDisplay) fontDisplay.textContent = prefs.size + 'px';

    // Update theme buttons
    document.querySelectorAll('[data-set-theme]').forEach((btn) => {
      btn.classList.toggle('is-active', btn.getAttribute('data-set-theme') === prefs.theme);
    });

    // Update font buttons
    document.querySelectorAll('[data-set-font]').forEach((btn) => {
      btn.classList.toggle('is-active', btn.getAttribute('data-set-font') === prefs.font);
    });

    // Update line height buttons
    document.querySelectorAll('[data-set-lh]').forEach((btn) => {
      btn.classList.toggle('is-active', btn.getAttribute('data-set-lh') === prefs.lh);
    });
  }

  function savePreference(key, val) {
    prefs[key] = val;
    safeStore.set('irbox_reader_' + key, val);
    applyPreferences();
  }

  // Pure Client-side ZIP Inflation using DecompressionStream
  async function inflateRaw(compressedBytes) {
    if (typeof DecompressionStream !== 'undefined') {
      const ds = new DecompressionStream('deflate-raw');
      const writer = ds.writable.getWriter();
      writer.write(compressedBytes);
      writer.close();
      const reader = ds.readable.getReader();
      const chunks = [];
      while (true) {
        const { done, value } = await reader.read();
        if (done) break;
        chunks.push(value);
      }
      const totalLen = chunks.reduce((acc, c) => acc + c.length, 0);
      const res = new Uint8Array(totalLen);
      let off = 0;
      for (const c of chunks) {
        res.set(c, off);
        off += c.length;
      }
      return res;
    }
    throw new Error('DecompressionStream not supported in this browser environment.');
  }

  function parseZipDirectory(buf) {
    const dv = new DataView(buf.buffer, buf.byteOffset, buf.byteLength);
    let eocd = -1;
    for (let i = buf.byteLength - 22; i >= Math.max(0, buf.byteLength - 65557); i--) {
      if (dv.getUint32(i, true) === 0x06054b50) {
        eocd = i;
        break;
      }
    }
    if (eocd === -1) {
      throw new Error('Invalid archive: End of Central Directory signature not found.');
    }

    const totalEntries = dv.getUint16(eocd + 10, true);
    const cdOffset = dv.getUint32(eocd + 16, true);
    const entries = {};

    let p = cdOffset;
    for (let i = 0; i < totalEntries; i++) {
      const sig = dv.getUint32(p, true);
      if (sig !== 0x02014b50) break;
      const comp = dv.getUint16(p + 10, true);
      const compSize = dv.getUint32(p + 20, true);
      const uncompSize = dv.getUint32(p + 24, true);
      const fnLen = dv.getUint16(p + 28, true);
      const extraLen = dv.getUint16(p + 30, true);
      const commentLen = dv.getUint16(p + 32, true);
      const lhOffset = dv.getUint32(p + 42, true);

      const fnBytes = new Uint8Array(buf.buffer, buf.byteOffset + p + 46, fnLen);
      const filename = new TextDecoder('utf-8').decode(fnBytes);

      entries[filename] = {
        filename,
        comp,
        compSize,
        uncompSize,
        lhOffset,
      };

      p += 46 + fnLen + extraLen + commentLen;
    }

    return entries;
  }

  async function readZipEntry(filename) {
    const entry = zipEntries[filename];
    if (!entry) return null;

    const dv = new DataView(zipBuffer.buffer, zipBuffer.byteOffset, zipBuffer.byteLength);
    const sig = dv.getUint32(entry.lhOffset, true);
    if (sig !== 0x04034b50) {
      throw new Error(`Corrupted ZIP local file header for ${filename}`);
    }

    const lhFnLen = dv.getUint16(entry.lhOffset + 26, true);
    const lhExtraLen = dv.getUint16(entry.lhOffset + 28, true);
    const dataOffset = entry.lhOffset + 30 + lhFnLen + lhExtraLen;
    const compSlice = new Uint8Array(
      zipBuffer.buffer,
      zipBuffer.byteOffset + dataOffset,
      entry.compSize
    );

    if (entry.comp === 0) {
      return compSlice;
    } else if (entry.comp === 8) {
      return await inflateRaw(compSlice);
    }
    throw new Error(`Unsupported compression method (${entry.comp}) for ${filename}`);
  }

  function resolvePath(baseDir, relativePath) {
    if (!baseDir) return relativePath;
    const parts = (baseDir + '/' + relativePath).split('/');
    const resolved = [];
    for (const part of parts) {
      if (part === '.' || part === '') continue;
      if (part === '..') {
        resolved.pop();
      } else {
        resolved.push(part);
      }
    }
    return resolved.join('/');
  }

  // EPUB Parser & Initializer
  async function loadEpub(buffer) {
    zipBuffer = buffer;
    loadingText.textContent = 'Unpacking EPUB package...';
    zipEntries = parseZipDirectory(zipBuffer);

    // 1. Locate container.xml
    const containerBytes = await readZipEntry('META-INF/container.xml');
    if (!containerBytes) {
      throw new Error('Invalid EPUB: META-INF/container.xml missing.');
    }
    const containerXml = new TextDecoder('utf-8').decode(containerBytes);
    const opfMatch = containerXml.match(/full-path=["']([^"']+)["']/i);
    const opfFullPath = opfMatch ? opfMatch[1] : 'EPUB/package.opf';

    opfDir = opfFullPath.includes('/') ? opfFullPath.slice(0, opfFullPath.lastIndexOf('/')) : '';

    // 2. Parse OPF file
    const opfBytes = await readZipEntry(opfFullPath);
    if (!opfBytes) {
      throw new Error(`Invalid EPUB: Package file ${opfFullPath} not found.`);
    }
    const opfXml = new TextDecoder('utf-8').decode(opfBytes);
    const parser = new DOMParser();
    const opfDoc = parser.parseFromString(opfXml, 'application/xml');

    // Extract Metadata
    const titleNode = opfDoc.querySelector('title') || opfDoc.querySelector('dc\\:title');
    const bookTitle = titleNode
      ? titleNode.textContent.trim()
      : bookFilename.replace(/\.epub$/i, '');
    bookTitleEl.textContent = bookTitle;
    document.title = `${bookTitle} — Web Book Reader`;

    // Extract Manifest
    epubManifest = {};
    const itemNodes = opfDoc.querySelectorAll('manifest > item');
    itemNodes.forEach((node) => {
      const id = node.getAttribute('id');
      const href = node.getAttribute('href');
      const mediaType = node.getAttribute('media-type');
      if (id && href) {
        epubManifest[id] = {
          id,
          href,
          mediaType,
          fullPath: resolvePath(opfDir, href),
        };
      }
    });

    // Extract Spine
    epubSpine = [];
    const itemrefNodes = opfDoc.querySelectorAll('spine > itemref');
    itemrefNodes.forEach((node) => {
      const idref = node.getAttribute('idref');
      const manifestItem = epubManifest[idref];
      if (manifestItem) {
        epubSpine.push(manifestItem);
      }
    });

    if (epubSpine.length === 0) {
      throw new Error('EPUB contains no readable spine chapters.');
    }

    // Extract TOC
    epubToc = [];
    // Try EPUB 3 nav document first
    const navItem = Object.values(epubManifest).find(
      (item) =>
        item.mediaType === 'application/xhtml+xml' &&
        opfXml.includes(`id="${item.id}"`) &&
        opfXml.includes('nav')
    );
    if (navItem) {
      try {
        const navBytes = await readZipEntry(navItem.fullPath);
        if (navBytes) {
          const navDoc = parser.parseFromString(
            new TextDecoder('utf-8').decode(navBytes),
            'application/xhtml+xml'
          );
          const navLinks = navDoc.querySelectorAll('nav[epub\\:type="toc"] a, nav a');
          navLinks.forEach((a) => {
            const href = a.getAttribute('href');
            const title = a.textContent.trim();
            if (href && title) {
              const full = resolvePath(opfDir, href.split('#')[0]);
              epubToc.push({ title, href, fullPath: full });
            }
          });
        }
      } catch (_) {}
    }

    // Fallback: If no TOC, use spine chapters
    if (epubToc.length === 0) {
      epubSpine.forEach((item, idx) => {
        const rawName = item.href
          .split('/')
          .pop()
          .replace(/\.x?html$/i, '')
          .replace(/[-_]+/g, ' ');
        const capName = rawName.charAt(0).toUpperCase() + rawName.slice(1);
        epubToc.push({
          title: `Chapter ${idx + 1} (${capName})`,
          href: item.href,
          fullPath: item.fullPath,
        });
      });
    }

    buildTocUI();

    // Check saved progress
    const savedIndex = parseInt(safeStore.get('irbox_book_progress_' + bookStorageKey, '0'), 10);
    currentChapterIndex =
      !isNaN(savedIndex) && savedIndex >= 0 && savedIndex < epubSpine.length ? savedIndex : 0;

    loadingEl.hidden = true;
    epubContainer.hidden = false;
    pdfContainer.hidden = true;

    await renderChapter(currentChapterIndex);
  }

  function buildTocUI() {
    if (!tocList) return;
    tocList.innerHTML = '';
    epubToc.forEach((entry, idx) => {
      const li = document.createElement('li');
      const a = document.createElement('a');
      a.href = '#';
      a.textContent = entry.title;
      a.setAttribute('data-toc-index', idx);
      a.addEventListener('click', async (e) => {
        e.preventDefault();
        // Find corresponding spine index
        const spineIdx = epubSpine.findIndex((item) => item.fullPath === entry.fullPath);
        if (spineIdx !== -1) {
          await renderChapter(spineIdx);
        } else {
          await renderChapter(idx);
        }
        closeToc();
      });
      li.appendChild(a);
      tocList.appendChild(li);
    });
  }

  // Render a single EPUB chapter
  async function renderChapter(index) {
    if (index < 0 || index >= epubSpine.length) return;
    currentChapterIndex = index;

    // Save progress
    safeStore.set('irbox_book_progress_' + bookStorageKey, index);

    // Clean up previous blob URLs to avoid memory leaks
    createdObjectUrls.forEach((url) => URL.revokeObjectURL(url));
    createdObjectUrls = [];

    const chapterItem = epubSpine[index];
    const chapterBytes = await readZipEntry(chapterItem.fullPath);
    if (!chapterBytes) {
      canvas.innerHTML = `<div class="card center muted"><p>Could not load chapter content (${chapterItem.href}).</p></div>`;
      return;
    }

    const chapterHtml = new TextDecoder('utf-8').decode(chapterBytes);
    const parser = new DOMParser();
    const doc = parser.parseFromString(chapterHtml, 'text/html');

    // Extract chapter title if available
    const h1 = doc.querySelector('h1, h2, title');
    const chapterName = h1 ? h1.textContent.trim() : `Chapter ${index + 1}`;
    chapterTitleEl.textContent = chapterName;

    // Resolve chapter relative directory for assets
    const chapterDir = chapterItem.fullPath.includes('/')
      ? chapterItem.fullPath.slice(0, chapterItem.fullPath.lastIndexOf('/'))
      : '';

    // Rewrite images
    const images = doc.querySelectorAll('img, image');
    for (const img of images) {
      const srcAttr = img.getAttribute('src') || img.getAttribute('xlink:href');
      if (
        srcAttr &&
        !srcAttr.startsWith('http://') &&
        !srcAttr.startsWith('https://') &&
        !srcAttr.startsWith('data:')
      ) {
        const cleanSrc = decodeURIComponent(srcAttr.split('#')[0].split('?')[0]);
        const fullImgPath = resolvePath(chapterDir, cleanSrc);
        const imgBytes = await readZipEntry(fullImgPath);
        if (imgBytes) {
          const mime = fullImgPath.endsWith('.svg')
            ? 'image/svg+xml'
            : fullImgPath.endsWith('.png')
              ? 'image/png'
              : fullImgPath.endsWith('.webp')
                ? 'image/webp'
                : 'image/jpeg';
          const blob = new Blob([imgBytes], { type: mime });
          const blobUrl = URL.createObjectURL(blob);
          createdObjectUrls.push(blobUrl);
          if (img.tagName.toLowerCase() === 'img') {
            img.setAttribute('src', blobUrl);
          } else {
            img.setAttribute('xlink:href', blobUrl);
          }
        }
      }
    }

    // Strip external scripts for security
    doc.querySelectorAll('script').forEach((s) => s.remove());

    // Extract body contents
    const bodyContent = doc.body ? doc.body.innerHTML : chapterHtml;
    canvas.innerHTML = bodyContent;

    // Update Pagination & Progress UI
    const total = epubSpine.length;
    const pct = Math.round(((index + 1) / total) * 100);
    progressPill.textContent = `${index + 1} / ${total} (${pct}%)`;
    progressBar.style.width = `${pct}%`;
    chapterCountBottom.textContent = `Chapter ${index + 1} of ${total}`;

    prevBtn.disabled = index === 0;
    nextBtn.disabled = index === total - 1;

    // Highlight active TOC item
    if (tocList) {
      tocList.querySelectorAll('a').forEach((a) => {
        const tocIdx = parseInt(a.getAttribute('data-toc-index'), 10);
        a.classList.toggle('is-active', tocIdx === index);
      });
    }

    // Scroll back to top
    window.scrollTo({ top: 0, behavior: 'smooth' });
    canvas.focus();
  }

  // PDF Viewer Handler
  function loadPdf(url, filename) {
    loadingEl.hidden = true;
    pdfContainer.hidden = false;
    epubContainer.hidden = true;

    const cleanTitle = filename.replace(/\.pdf$/i, '').replace(/[-_]+/g, ' ');
    bookTitleEl.textContent = cleanTitle;
    document.title = `${cleanTitle} — PDF Reader`;

    if (pdfTitle) pdfTitle.textContent = filename;
    if (pdfDownload) pdfDownload.setAttribute('href', url);
    if (pdfExternal) pdfExternal.setAttribute('href', url);
    if (pdfFallbackOpen) pdfFallbackOpen.setAttribute('href', url);
    if (pdfFallbackDownload) pdfFallbackDownload.setAttribute('href', url);
    if (pdfMobileOpen) pdfMobileOpen.setAttribute('href', url);
    if (pdfMobileDownload) pdfMobileDownload.setAttribute('href', url);
    if (pdfMobileTitle) pdfMobileTitle.textContent = cleanTitle;

    // Detect if browser environment has native inline PDF viewer support
    // (e.g., Chrome Android returns false; desktop Chrome/Firefox/Safari return true)
    const hasInlinePdf = typeof navigator !== 'undefined' && Boolean(navigator.pdfViewerEnabled);

    if (hasInlinePdf) {
      if (pdfMobileView) pdfMobileView.hidden = true;
      if (pdfObject) {
        pdfObject.hidden = false;
        pdfObject.setAttribute('data', url + '#toolbar=1&navpanes=1');
      }
      if (pdfEmbed) {
        pdfEmbed.setAttribute('src', url + '#toolbar=1&navpanes=1');
      }
    } else {
      // For mobile browsers (Chrome Android, etc.), do not navigate an embedded object/frame
      // to avoid the flashing "This content is blocked. Contact the site owner to fix the issue."
      if (pdfObject) pdfObject.hidden = true;
      if (pdfMobileView) pdfMobileView.hidden = false;
    }

    // Hide EPUB-only controls
    if (tocToggle) tocToggle.hidden = true;
    if (settingsToggle) settingsToggle.hidden = true;
    progressPill.textContent = 'PDF Mode';
  }

  // Toggle Controls
  function toggleToc() {
    const isHidden = tocDrawer.hasAttribute('hidden');
    if (isHidden) {
      tocDrawer.removeAttribute('hidden');
      tocToggle.setAttribute('aria-expanded', 'true');
      settingsPopover.setAttribute('hidden', '');
      settingsToggle.setAttribute('aria-expanded', 'false');
    } else {
      closeToc();
    }
  }

  function closeToc() {
    tocDrawer.setAttribute('hidden', '');
    tocToggle.setAttribute('aria-expanded', 'false');
  }

  function toggleSettings() {
    const isHidden = settingsPopover.hasAttribute('hidden');
    if (isHidden) {
      settingsPopover.removeAttribute('hidden');
      settingsToggle.setAttribute('aria-expanded', 'true');
      closeToc();
    } else {
      closeSettings();
    }
  }

  function closeSettings() {
    settingsPopover.setAttribute('hidden', '');
    settingsToggle.setAttribute('aria-expanded', 'false');
  }

  function toggleFullscreen() {
    if (!document.fullscreenElement) {
      document.documentElement.requestFullscreen().catch(() => {});
    } else {
      if (document.exitFullscreen) document.exitFullscreen();
    }
  }

  // Event Listeners
  if (prevBtn) {
    prevBtn.addEventListener('click', () => renderChapter(currentChapterIndex - 1));
  }
  if (nextBtn) {
    nextBtn.addEventListener('click', () => renderChapter(currentChapterIndex + 1));
  }
  if (tocToggle) {
    tocToggle.addEventListener('click', toggleToc);
  }
  if (tocClose) {
    tocClose.addEventListener('click', closeToc);
  }
  if (settingsToggle) {
    settingsToggle.addEventListener('click', toggleSettings);
  }
  if (fullscreenToggle) {
    fullscreenToggle.addEventListener('click', toggleFullscreen);
  }

  // Font size adjustments
  if (fontDecBtn) {
    fontDecBtn.addEventListener('click', () => {
      if (prefs.size > 14) savePreference('size', prefs.size - 2);
    });
  }
  if (fontIncBtn) {
    fontIncBtn.addEventListener('click', () => {
      if (prefs.size < 32) savePreference('size', prefs.size + 2);
    });
  }

  // Preference switchers
  document.querySelectorAll('[data-set-theme]').forEach((btn) => {
    btn.addEventListener('click', () =>
      savePreference('theme', btn.getAttribute('data-set-theme'))
    );
  });
  document.querySelectorAll('[data-set-font]').forEach((btn) => {
    btn.addEventListener('click', () => savePreference('font', btn.getAttribute('data-set-font')));
  });
  document.querySelectorAll('[data-set-lh]').forEach((btn) => {
    btn.addEventListener('click', () => savePreference('lh', btn.getAttribute('data-set-lh')));
  });

  // Global Keyboard Shortcuts
  window.addEventListener('keydown', (e) => {
    if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') return;

    if (e.key === 'ArrowRight' || e.key === 'PageDown') {
      if (bookType === 'epub' && currentChapterIndex < epubSpine.length - 1) {
        e.preventDefault();
        renderChapter(currentChapterIndex + 1);
      }
    } else if (e.key === 'ArrowLeft' || e.key === 'PageUp') {
      if (bookType === 'epub' && currentChapterIndex > 0) {
        e.preventDefault();
        renderChapter(currentChapterIndex - 1);
      }
    } else if (e.key.toLowerCase() === 'f' && !e.ctrlKey && !e.metaKey) {
      e.preventDefault();
      toggleFullscreen();
    } else if (e.key === 'Escape') {
      closeToc();
      closeSettings();
    }
  });

  // Click outside to close popovers
  document.addEventListener('click', (e) => {
    if (!settingsPopover.contains(e.target) && !settingsToggle.contains(e.target)) {
      closeSettings();
    }
    if (
      !tocDrawer.contains(e.target) &&
      !tocToggle.contains(e.target) &&
      !tocDrawer.hasAttribute('hidden')
    ) {
      closeToc();
    }
  });

  // Main Book Loader Entry Point
  async function initReader() {
    applyPreferences();

    const urlParams = new URLSearchParams(window.location.search);
    const bookParam = urlParams.get('book');

    if (!bookParam) {
      // Default to irbox-docs.epub if none specified
      window.location.replace(window.location.pathname + '?book=irbox-docs.epub');
      return;
    }

    // Check if loading local file from sessionStorage
    if (bookParam === 'local') {
      try {
        const localName = sessionStorage.getItem('irbox_temp_book_name') || 'Local Book';
        const localData = sessionStorage.getItem('irbox_temp_book_data');
        if (!localData) throw new Error('No local book data found in session storage.');

        bookFilename = localName;
        bookStorageKey = 'local_' + localName;

        if (localName.toLowerCase().endsWith('.pdf')) {
          bookType = 'pdf';
          // Convert Base64 data URL to binary Blob URL for CSP compatibility and memory efficiency
          try {
            const binaryStr = atob(localData.split(',')[1]);
            const len = binaryStr.length;
            const bytes = new Uint8Array(len);
            for (let i = 0; i < len; i++) {
              bytes[i] = binaryStr.charCodeAt(i);
            }
            const pdfBlob = new Blob([bytes], { type: 'application/pdf' });
            const pdfBlobUrl = URL.createObjectURL(pdfBlob);
            createdObjectUrls.push(pdfBlobUrl);
            loadPdf(pdfBlobUrl, localName);
          } catch {
            loadPdf(localData, localName);
          }
        } else {
          bookType = 'epub';
          const binaryStr = atob(localData.split(',')[1]);
          const len = binaryStr.length;
          const bytes = new Uint8Array(len);
          for (let i = 0; i < len; i++) {
            bytes[i] = binaryStr.charCodeAt(i);
          }
          await loadEpub(bytes);
        }
        return;
      } catch (err) {
        showError('Failed to read local file', err.message);
        return;
      }
    }

    bookFilename = bookParam;
    bookStorageKey = bookParam;
    const baseurl = document.documentElement.getAttribute('data-baseurl') || '';
    const archiveUrl = `${baseurl}/books/archive/${encodeURIComponent(bookParam)}`;

    if (bookParam.toLowerCase().endsWith('.pdf')) {
      bookType = 'pdf';
      loadPdf(archiveUrl, bookParam);
      return;
    }

    // EPUB mode: fetch and decompress
    bookType = 'epub';
    loadingText.textContent = `Fetching ${bookParam}...`;

    try {
      const response = await fetch(archiveUrl);
      if (!response.ok) {
        throw new Error(`Server returned HTTP ${response.status} (${response.statusText})`);
      }
      const arrayBuffer = await response.arrayBuffer();
      await loadEpub(new Uint8Array(arrayBuffer));
    } catch (err) {
      showError('Unable to open book', `Could not fetch or open "${bookParam}": ${err.message}`);
    }
  }

  function showError(title, message) {
    if (loadingEl) loadingEl.hidden = true;
    if (epubContainer) epubContainer.hidden = true;
    if (pdfContainer) pdfContainer.hidden = true;
    if (errorEl) {
      errorEl.hidden = false;
      errorTitle.textContent = title;
      errorMsg.textContent = message;
    }
  }

  if (retryBtn) {
    retryBtn.addEventListener('click', () => {
      window.location.href =
        (document.documentElement.getAttribute('data-baseurl') || '') + '/books/';
    });
  }

  // Initialize
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initReader);
  } else {
    initReader();
  }
})();

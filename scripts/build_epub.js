const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

function crc32(buf) {
  let table = [];
  for (let n = 0; n < 256; n++) {
    let c = n;
    for (let k = 0; k < 8; k++) {
      if (c & 1) c = 0xedb88320 ^ (c >>> 1);
      else c = c >>> 1;
    }
    table[n] = c;
  }
  let crc = 0 ^ -1;
  for (let i = 0; i < buf.length; i++) {
    crc = (crc >>> 8) ^ table[(crc ^ buf[i]) & 0xff];
  }
  return (crc ^ -1) >>> 0;
}

class SimpleZip {
  constructor() {
    this.files = [];
  }
  addFile(name, content, compress = true) {
    const data = Buffer.isBuffer(content) ? content : Buffer.from(content, 'utf8');
    const crc = crc32(data);
    let compressedData = data;
    let method = 0; // STORE
    if (compress) {
      compressedData = zlib.deflateRawSync(data);
      method = 8; // DEFLATE
    }
    this.files.push({
      name,
      data: compressedData,
      uncompressedSize: data.length,
      compressedSize: compressedData.length,
      crc,
      method,
    });
  }
  toBuffer() {
    const localHeaders = [];
    const centralHeaders = [];
    let offset = 0;

    for (const f of this.files) {
      const nameBuf = Buffer.from(f.name, 'utf8');

      // Local Header
      const lh = Buffer.alloc(30);
      lh.writeUInt32LE(0x04034b50, 0);
      lh.writeUInt16LE(20, 4);
      lh.writeUInt16LE(0, 6);
      lh.writeUInt16LE(f.method, 8);
      lh.writeUInt16LE(0, 10);
      lh.writeUInt16LE(0, 12);
      lh.writeUInt32LE(f.crc, 14);
      lh.writeUInt32LE(f.compressedSize, 18);
      lh.writeUInt32LE(f.uncompressedSize, 22);
      lh.writeUInt16LE(nameBuf.length, 26);
      lh.writeUInt16LE(0, 28);

      localHeaders.push(lh, nameBuf, f.data);

      // Central Directory Header
      const ch = Buffer.alloc(46);
      ch.writeUInt32LE(0x02014b50, 0);
      ch.writeUInt16LE(20, 4);
      ch.writeUInt16LE(20, 6);
      ch.writeUInt16LE(0, 8);
      ch.writeUInt16LE(f.method, 10);
      ch.writeUInt16LE(0, 12);
      ch.writeUInt16LE(0, 14);
      ch.writeUInt32LE(f.crc, 16);
      ch.writeUInt32LE(f.compressedSize, 20);
      ch.writeUInt32LE(f.uncompressedSize, 24);
      ch.writeUInt16LE(nameBuf.length, 28);
      ch.writeUInt16LE(0, 30);
      ch.writeUInt16LE(0, 32);
      ch.writeUInt16LE(0, 34);
      ch.writeUInt16LE(0, 36);
      ch.writeUInt32LE(0, 38);
      ch.writeUInt32LE(offset, 42);

      centralHeaders.push(ch, nameBuf);

      offset += 30 + nameBuf.length + f.data.length;
    }

    const centralDirOffset = offset;
    const centralDirSize = centralHeaders.reduce((sum, b) => sum + b.length, 0);

    const eocd = Buffer.alloc(22);
    eocd.writeUInt32LE(0x06054b50, 0);
    eocd.writeUInt16LE(0, 4);
    eocd.writeUInt16LE(0, 6);
    eocd.writeUInt16LE(this.files.length, 8);
    eocd.writeUInt16LE(this.files.length, 10);
    eocd.writeUInt32LE(centralDirSize, 12);
    eocd.writeUInt32LE(centralDirOffset, 16);
    eocd.writeUInt16LE(0, 20);

    return Buffer.concat([...localHeaders, ...centralHeaders, eocd]);
  }
}

function escapeXml(str) {
  return String(str || '')
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&apos;');
}

function htmlToXhtml(html) {
  let xhtml = html
    // Remove scripts and copy buttons
    .replace(/<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>/gi, '')
    .replace(/<button\b[^<]*(?:(?!<\/button>)<[^<]*)*<\/button>/gi, '')
    // Remove anchor links inserted by JS (# heading anchors)
    .replace(/<a class="anchor"[^>]*>#<\/a>/gi, '')
    // Close void tags
    .replace(/<(img|hr|br|input)([^>]*?)>/gi, (match, tag, attrs) => {
      if (attrs.trim().endsWith('/')) return match;
      return `<${tag}${attrs}/>`;
    })
    // Rewrite internal doc links /docs/slug/ -> slug.xhtml
    .replace(/href="(?:\/|https?:\/\/[^/]+)?\/docs\/([a-z0-9-]+)\/?"/gi, 'href="$1.xhtml"');

  return xhtml;
}

const rootDir = process.cwd();
const siteDir = path.join(rootDir, '_site');

// Read docs navigation
const docsNavContent = fs.readFileSync(path.join(rootDir, '_data/docs_nav.yml'), 'utf8');
const slugs = [...docsNavContent.matchAll(/- ([a-z0-9-]+)/g)].map((m) => m[1]);

console.log(`Building EPUB book with ${slugs.length} chapters...`);

const zip = new SimpleZip();

// 1. mimetype (MUST BE FIRST AND STORED)
zip.addFile('mimetype', 'application/epub+zip', false);

// 2. container.xml
zip.addFile(
  'META-INF/container.xml',
  `<?xml version="1.0" encoding="UTF-8"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
  <rootfiles>
    <rootfile full-path="EPUB/package.opf" media-type="application/oebps-package+xml"/>
  </rootfiles>
</container>`,
  true
);

// 3. style.css (e-reader typography)
const epubCss = `
@charset "utf-8";
body {
  margin: 5% 6%;
  font-family: serif;
  font-size: 1.05em;
  line-height: 1.6;
  color: #111;
  background: #fff;
}
h1, h2, h3, h4 {
  font-family: sans-serif;
  line-height: 1.25;
  margin-top: 1.4em;
  margin-bottom: 0.5em;
  page-break-after: avoid;
  break-after: avoid;
}
h1 {
  font-size: 1.9em;
  page-break-before: always;
  break-before: page;
}
h2 {
  font-size: 1.4em;
}
h3 {
  font-size: 1.15em;
}
p.lead {
  font-size: 1.15em;
  line-height: 1.5;
  color: #444;
  margin-bottom: 1.5em;
}
pre, code {
  font-family: monospace;
  font-size: 0.9em;
}
pre {
  background: #f4f4f4;
  padding: 12px;
  border-radius: 4px;
  overflow-x: auto;
  page-break-inside: avoid;
  break-inside: avoid;
}
code {
  background: #f4f4f4;
  padding: 2px 4px;
  border-radius: 2px;
}
pre code {
  background: none;
  padding: 0;
}
blockquote {
  margin: 1.2em 0;
  padding-left: 16px;
  border-left: 3px solid #ccc;
  color: #555;
  font-style: italic;
}
table {
  width: 100%;
  border-collapse: collapse;
  margin: 1.4em 0;
  page-break-inside: avoid;
  break-inside: avoid;
}
th, td {
  border: 1px solid #ddd;
  padding: 8px 12px;
  text-align: left;
}
th {
  background: #f7f7f7;
  font-weight: bold;
}
hr {
  border: 0;
  border-top: 1px solid #ddd;
  margin: 2em 0;
}
.cover {
  text-align: center;
  padding: 20% 0;
  page-break-before: always;
  break-before: page;
}
.cover h1 {
  font-size: 2.5em;
  margin-bottom: 0.2em;
}
.cover p {
  font-size: 1.2em;
  color: #555;
}
.cover .author {
  margin-top: 3em;
  font-weight: bold;
  font-size: 1.1em;
}
`;
zip.addFile('EPUB/style.css', epubCss, true);

// 4. Extract chapters
const chapters = [];
for (const slug of slugs) {
  const htmlPath = path.join(siteDir, 'docs', slug, 'index.html');
  if (!fs.existsSync(htmlPath)) continue;

  const rawHtml = fs.readFileSync(htmlPath, 'utf8');
  const titleMatch = rawHtml.match(/<h1 class="display-1">([^<]+)<\/h1>/);
  const title = titleMatch ? titleMatch[1].trim() : slug;

  const leadMatch = rawHtml.match(/<p class="lead">([\s\S]*?)<\/p>/);
  const lead = leadMatch ? leadMatch[1].trim() : '';

  const proseMatch = rawHtml.match(
    /<div class="prose" data-prose>([\s\S]*?)<\/div>\s*<(?:nav|div|p)/
  );
  let prose = proseMatch ? proseMatch[1] : '';

  const cleanXhtml = htmlToXhtml(prose);

  const chapterHtml = `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="en" lang="en">
<head>
  <meta charset="utf-8"/>
  <title>${escapeXml(title)}</title>
  <link rel="stylesheet" type="text/css" href="style.css"/>
</head>
<body>
  <h1>${escapeXml(title)}</h1>
  ${lead ? `<p class="lead">${escapeXml(lead)}</p>` : ''}
  ${cleanXhtml}
</body>
</html>`;

  const fileName = `${slug}.xhtml`;
  zip.addFile(`EPUB/${fileName}`, chapterHtml, true);
  chapters.push({ slug, title, fileName });
}

// 5. Cover Page
const coverHtml = `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="en" lang="en">
<head>
  <meta charset="utf-8"/>
  <title>Irbox Book</title>
  <link rel="stylesheet" type="text/css" href="style.css"/>
</head>
<body>
  <div class="cover">
    <h1>Irbox</h1>
    <p>Documentation, Architecture &amp; Field Notes</p>
    <div class="author">By Irbox</div>
    <p style="font-size: 0.9em; margin-top: 1em; color: #888;">Complete Reference Edition · 2026</p>
  </div>
</body>
</html>`;
zip.addFile('EPUB/cover.xhtml', coverHtml, true);

// 6. Navigation Document (nav.xhtml - EPUB 3 standard)
const navHtml = `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:epub="http://www.idpf.org/2007/ops" xml:lang="en" lang="en">
<head>
  <meta charset="utf-8"/>
  <title>Table of Contents</title>
  <link rel="stylesheet" type="text/css" href="style.css"/>
</head>
<body>
  <nav epub:type="toc" id="toc">
    <h1>Table of Contents</h1>
    <ol>
      <li><a href="cover.xhtml">Title Page</a></li>
      ${chapters.map((c) => `<li><a href="${c.fileName}">${escapeXml(c.title)}</a></li>`).join('\n      ')}
    </ol>
  </nav>
</body>
</html>`;
zip.addFile('EPUB/nav.xhtml', navHtml, true);

// 7. NCX Document (toc.ncx - EPUB 2 backward compatibility for older Kindles)
const ncxHtml = `<?xml version="1.0" encoding="UTF-8"?>
<ncx xmlns="http://www.daisy.org/z3986/2005/ncx/" version="2005-1">
  <head>
    <meta name="dtb:uid" content="urn:uuid:irbox-docs-book-2026"/>
    <meta name="dtb:depth" content="1"/>
    <meta name="dtb:totalPageCount" content="0"/>
    <meta name="dtb:maxPageNumber" content="0"/>
  </head>
  <docTitle><text>Irbox Documentation &amp; Book</text></docTitle>
  <navMap>
    <navPoint id="navpoint-1" playOrder="1">
      <navLabel><text>Title Page</text></navLabel>
      <content src="cover.xhtml"/>
    </navPoint>
    ${chapters
      .map(
        (c, idx) => `
    <navPoint id="navpoint-${idx + 2}" playOrder="${idx + 2}">
      <navLabel><text>${escapeXml(c.title)}</text></navLabel>
      <content src="${c.fileName}"/>
    </navPoint>`
      )
      .join('')}
  </navMap>
</ncx>`;
zip.addFile('EPUB/toc.ncx', ncxHtml, true);

// 8. Package Document (package.opf)
const opfHtml = `<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" version="3.0" unique-identifier="pub-id">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:identifier id="pub-id">urn:uuid:irbox-docs-book-2026</dc:identifier>
    <dc:title>Irbox — Documentation, Architecture &amp; Field Notes</dc:title>
    <dc:language>en</dc:language>
    <dc:creator>Irbox</dc:creator>
    <dc:publisher>Irbox</dc:publisher>
    <dc:description>Complete documentation and reference guide for Irbox projects and systems.</dc:description>
    <meta property="dcterms:modified">2026-10-24T12:00:00Z</meta>
  </metadata>
  <manifest>
    <item id="style" href="style.css" media-type="text/css"/>
    <item id="nav" href="nav.xhtml" media-type="application/xhtml+xml" properties="nav"/>
    <item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml"/>
    <item id="cover" href="cover.xhtml" media-type="application/xhtml+xml"/>
    ${chapters.map((c) => `<item id="${c.slug}" href="${c.fileName}" media-type="application/xhtml+xml"/>`).join('\n    ')}
  </manifest>
  <spine toc="ncx">
    <itemref idref="cover"/>
    ${chapters.map((c) => `<itemref idref="${c.slug}"/>`).join('\n    ')}
  </spine>
</package>`;
zip.addFile('EPUB/package.opf', opfHtml, true);

// Output to assets/book.epub and _site/book.epub
const epubBuffer = zip.toBuffer();

const outAssets = path.join(rootDir, 'assets', 'book.epub');
const outSite = path.join(siteDir, 'book.epub');

fs.writeFileSync(outAssets, epubBuffer);
if (fs.existsSync(siteDir)) {
  fs.writeFileSync(outSite, epubBuffer);
}

console.log(`✓ EPUB book generated successfully! (${epubBuffer.length} bytes)`);
console.log(`  Saved to: ${outAssets}`);
if (fs.existsSync(siteDir)) {
  console.log(`  Saved to: ${outSite}`);
}

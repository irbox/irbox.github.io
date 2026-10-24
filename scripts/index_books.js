#!/usr/bin/env node

/**
 * Irbox Book Archive Indexer
 * Scans `books/archive/` for EPUB and PDF books, extracts their metadata,
 * and generates `_data/books.yml` for Jekyll and the online web reader.
 *
 * Zero external dependencies — pure Node.js runtime.
 */

const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const rootDir = process.cwd();
const archiveDir = path.join(rootDir, 'books', 'archive');
const dataDir = path.join(rootDir, '_data');
const outputFile = path.join(dataDir, 'books.yml');

function formatBytes(bytes) {
  if (!bytes || bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
}

function cleanHtmlEntities(str) {
  if (!str) return '';
  return str
    .replace(/&amp;/g, '&')
    .replace(/&lt;/g, '<')
    .replace(/&gt;/g, '>')
    .replace(/&quot;/g, '"')
    .replace(/&#39;/g, "'")
    .replace(/&apos;/g, "'")
    .replace(/\s+/g, ' ')
    .trim();
}

function humanizeFilename(filename) {
  const base = path.basename(filename, path.extname(filename));
  return base
    .replace(/[-_]+/g, ' ')
    .replace(/\b\w/g, (c) => c.toUpperCase())
    .trim();
}

function parseZip(buf) {
  let eocdOffset = -1;
  for (let i = buf.length - 22; i >= Math.max(0, buf.length - 65557); i--) {
    if (buf.readUInt32LE(i) === 0x06054b50) {
      eocdOffset = i;
      break;
    }
  }
  if (eocdOffset === -1) return null;

  const totalEntries = buf.readUInt16LE(eocdOffset + 10);
  const cdSize = buf.readUInt32LE(eocdOffset + 12);
  const cdOffset = buf.readUInt32LE(eocdOffset + 16);

  const files = {};
  let p = cdOffset;
  for (let i = 0; i < totalEntries && p < cdOffset + cdSize; i++) {
    const sig = buf.readUInt32LE(p);
    if (sig !== 0x02014b50) break;
    const compression = buf.readUInt16LE(p + 10);
    const compressedSize = buf.readUInt32LE(p + 20);
    const uncompressedSize = buf.readUInt32LE(p + 24);
    const fnLen = buf.readUInt16LE(p + 28);
    const extraLen = buf.readUInt16LE(p + 30);
    const commentLen = buf.readUInt16LE(p + 32);
    const localHeaderOffset = buf.readUInt32LE(p + 42);
    const filename = buf.toString('utf8', p + 46, p + 46 + fnLen);

    files[filename] = {
      filename,
      compression,
      compressedSize,
      uncompressedSize,
      localHeaderOffset,
    };

    p += 46 + fnLen + extraLen + commentLen;
  }

  function readFile(filename) {
    const entry = files[filename];
    if (!entry) return null;
    const lhSig = buf.readUInt32LE(entry.localHeaderOffset);
    if (lhSig !== 0x04034b50) return null;
    const lhFnLen = buf.readUInt16LE(entry.localHeaderOffset + 26);
    const lhExtraLen = buf.readUInt16LE(entry.localHeaderOffset + 28);
    const dataOffset = entry.localHeaderOffset + 30 + lhFnLen + lhExtraLen;
    const compData = buf.subarray(dataOffset, dataOffset + entry.compressedSize);
    if (entry.compression === 0) {
      return compData;
    } else if (entry.compression === 8) {
      return zlib.inflateRawSync(compData);
    }
    return null;
  }

  return { files, readFile };
}

function parseEpub(filePath) {
  try {
    const buf = fs.readFileSync(filePath);
    const zip = parseZip(buf);
    if (!zip) return null;

    const containerXml = zip.readFile('META-INF/container.xml')?.toString('utf8');
    let opfPath = 'EPUB/package.opf';
    if (containerXml) {
      const match = containerXml.match(/full-path=["']([^"']+)["']/i);
      if (match) opfPath = match[1];
    }

    const opfContent = zip.readFile(opfPath)?.toString('utf8') || '';
    const titleMatch = opfContent.match(/<dc:title[^>]*>([\s\S]*?)<\/dc:title>/i);
    const authorMatch = opfContent.match(/<dc:creator[^>]*>([\s\S]*?)<\/dc:creator>/i);
    const descMatch = opfContent.match(/<dc:description[^>]*>([\s\S]*?)<\/dc:description>/i);
    const langMatch = opfContent.match(/<dc:language[^>]*>([\s\S]*?)<\/dc:language>/i);
    const dateMatch = opfContent.match(/<dc:date[^>]*>([\s\S]*?)<\/dc:date>/i);
    const itemrefs = opfContent.match(/<itemref\b[^>]*>/gi) || [];

    return {
      title: titleMatch ? cleanHtmlEntities(titleMatch[1]) : null,
      author: authorMatch ? cleanHtmlEntities(authorMatch[1]) : null,
      description: descMatch ? cleanHtmlEntities(descMatch[1]) : null,
      language: langMatch ? cleanHtmlEntities(langMatch[1]) : 'en',
      date: dateMatch ? cleanHtmlEntities(dateMatch[1]).slice(0, 10) : null,
      chapters: itemrefs.length || null,
    };
  } catch (err) {
    console.warn(`[index_books] Warning reading EPUB ${filePath}:`, err.message);
    return null;
  }
}

function parsePdf(filePath) {
  try {
    const buf = fs.readFileSync(filePath);
    const str = buf.toString('binary');

    const titleMatch = str.match(/\/Title\s*(?:\(([^)]+)\)|<([0-9a-fA-F]+)>)/);
    const authorMatch = str.match(/\/Author\s*(?:\(([^)]+)\)|<([0-9a-fA-F]+)>)/);
    const subjectMatch = str.match(/\/Subject\s*(?:\(([^)]+)\)|<([0-9a-fA-F]+)>)/);

    const pagesCountMatch = str.match(/\/Type\s*\/Pages[\s\S]*?\/Count\s+(\d+)/);
    const pageInstancesMatch = str.match(/\/Type\s*\/Page\b/g);

    let pages = null;
    if (pagesCountMatch) {
      pages = parseInt(pagesCountMatch[1], 10);
    } else if (pageInstancesMatch) {
      pages = pageInstancesMatch.length;
    }

    function decodePdfString(match) {
      if (!match) return null;
      if (match[1]) {
        return match[1].replace(/\\([\\()])/g, '$1').trim();
      }
      if (match[2]) {
        try {
          return Buffer.from(match[2], 'hex').toString('utf8').trim();
        } catch (_) {
          return null;
        }
      }
      return null;
    }

    return {
      title: decodePdfString(titleMatch),
      author: decodePdfString(authorMatch),
      description: decodePdfString(subjectMatch),
      pages: pages,
    };
  } catch (err) {
    console.warn(`[index_books] Warning reading PDF ${filePath}:`, err.message);
    return null;
  }
}

function indexBooks() {
  if (!fs.existsSync(archiveDir)) {
    fs.mkdirSync(archiveDir, { recursive: true });
  }
  if (!fs.existsSync(dataDir)) {
    fs.mkdirSync(dataDir, { recursive: true });
  }

  const entries = fs.readdirSync(archiveDir, { withFileTypes: true });
  const books = [];

  for (const entry of entries) {
    if (!entry.isFile()) continue;
    const ext = path.extname(entry.name).toLowerCase();
    if (ext !== '.epub' && ext !== '.pdf') continue;

    const fullPath = path.join(archiveDir, entry.name);
    const stats = fs.statSync(fullPath);
    const rawId = path
      .basename(entry.name, ext)
      .toLowerCase()
      .replace(/[^a-z0-9_-]+/g, '-');

    let meta = null;
    if (ext === '.epub') {
      meta = parseEpub(fullPath);
    } else if (ext === '.pdf') {
      meta = parsePdf(fullPath);
    }

    const title = meta?.title || humanizeFilename(entry.name);
    const author = meta?.author || 'Unknown';
    const description =
      meta?.description || `Personal archive document in ${ext.slice(1).toUpperCase()} format.`;
    const chapters = meta?.chapters || null;
    const pages = meta?.pages || null;

    books.push({
      id: rawId,
      filename: entry.name,
      title: title,
      author: author,
      description: description,
      format: ext.slice(1), // epub or pdf
      file_size: formatBytes(stats.size),
      bytes: stats.size,
      chapters: chapters,
      pages: pages,
      url: `/books/archive/${encodeURIComponent(entry.name)}`,
      reader_url: `/books/reader/?book=${encodeURIComponent(entry.name)}`,
      added_at: stats.mtime.toISOString().slice(0, 10),
    });
  }

  // Sort by added_at desc, then title
  books.sort((a, b) => b.added_at.localeCompare(a.added_at) || a.title.localeCompare(b.title));

  // Serialize to clean YAML
  const yamlLines = ['# Auto-generated by scripts/index_books.js — Do not edit manually', ''];
  for (const book of books) {
    yamlLines.push(`- id: ${JSON.stringify(book.id)}`);
    yamlLines.push(`  filename: ${JSON.stringify(book.filename)}`);
    yamlLines.push(`  title: ${JSON.stringify(book.title)}`);
    yamlLines.push(`  author: ${JSON.stringify(book.author)}`);
    yamlLines.push(`  description: ${JSON.stringify(book.description)}`);
    yamlLines.push(`  format: ${JSON.stringify(book.format)}`);
    yamlLines.push(`  file_size: ${JSON.stringify(book.file_size)}`);
    yamlLines.push(`  bytes: ${book.bytes}`);
    yamlLines.push(`  chapters: ${book.chapters !== null ? book.chapters : 'null'}`);
    yamlLines.push(`  pages: ${book.pages !== null ? book.pages : 'null'}`);
    yamlLines.push(`  url: ${JSON.stringify(book.url)}`);
    yamlLines.push(`  reader_url: ${JSON.stringify(book.reader_url)}`);
    yamlLines.push(`  added_at: ${JSON.stringify(book.added_at)}`);
    yamlLines.push('');
  }

  fs.writeFileSync(outputFile, yamlLines.join('\n'), 'utf8');

  console.log(`\x1b[32m✓ Indexed ${books.length} book(s) in books/archive/\x1b[0m`);
  console.log(`  Saved data to: ${outputFile}`);
  for (const b of books) {
    console.log(`  - [${b.format.toUpperCase()}] ${b.title} by ${b.author} (${b.file_size})`);
  }

  return books;
}

if (require.main === module) {
  indexBooks();
}

module.exports = { indexBooks };

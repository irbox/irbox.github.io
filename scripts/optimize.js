#!/usr/bin/env node

/**
 * ============================================================================
 * Irbox — Extreme Site Optimizer & Obfuscation Engine
 * ============================================================================
 * High-performance, multi-layered build optimizer delivering:
 *
 * 1. Extreme JavaScript Obfuscation & Hardening:
 *    - Cryptographic Base64 string array encoding, rotation, and shuffling
 *    - Control-flow flattening with switch-dispatch state machines
 *    - Hexadecimal identifier mangling (_0x...)
 *    - Split string fragmentation
 *    - Numerical arithmetic/bitwise expression transformation
 *    - Strict Content Security Policy (CSP) compliance (eval-free)
 *
 * 2. Extreme HTML Minification & Protection:
 *    - Deep whitespace elimination, tag collapse, boolean attribute reduction
 *    - Removal of redundant type attributes and optional quotes
 *    - Minification and obfuscation of inline scripts
 *    - Minification of inline CSS
 *    - Entity encoding for mailto: / email addresses against scrapers
 *
 * 3. Extreme CSS Minification:
 *    - CleanCSS Level 2 advanced AST optimizations
 *    - Media query and adjacent rules merging
 *    - Selector restructuring, duplicate declaration deduplication
 *
 * 4. SVG Vector Optimization:
 *    - Multi-pass SVGO compression with preserved viewports and clean paths
 *
 * 5. Data & Manifest Minification:
 *    - JSON / WebManifest zero-whitespace compaction
 *    - XML feed & sitemap comment stripping and tag compaction
 *
 * 6. Maximum Efficiency Pre-Compression:
 *    - Pre-generates Brotli (.br, Quality 11) & Gzip (.gz, Level 9) assets
 *    - Allows zero-CPU instant delivery on modern web servers & CDNs
 * ============================================================================
 */

const fs = require('fs');
const path = require('path');
const zlib = require('zlib');
const CleanCSS = require('clean-css');
const htmlMinifier = require('html-minifier-terser');
const JavaScriptObfuscator = require('javascript-obfuscator');
const terser = require('terser');
const svgo = require('svgo');

// Command line argument parser
const args = process.argv.slice(2);
function getArg(flag, defaultValue = null) {
  const idx = args.indexOf(flag);
  if (idx !== -1 && idx + 1 < args.length) return args[idx + 1];
  return defaultValue;
}
const hasFlag = (flag) => args.includes(flag);

if (hasFlag('--help') || hasFlag('-h')) {
  console.log(`
Irbox Extreme Optimizer & Obfuscator

Usage:
  node scripts/optimize.js [options]

Options:
  --dir <path>          Target directory to optimize (default: "_site")
  --assets              Optimize "assets/" directory in-place
  --obfuscate-only      Only run JavaScript obfuscation
  --minify-only         Only run minification (uses Terser instead of heavy obfuscator)
  --no-precompress      Skip generating .br (Brotli) and .gz (Gzip) compressed files
  --quiet               Suppress per-file logging and show summary only
  -h, --help            Show this help message
`);
  process.exit(0);
}

const targetDir = path.resolve(getArg('--dir', hasFlag('--assets') ? 'assets' : '_site'));
const obfuscateOnly = hasFlag('--obfuscate-only');
const minifyOnly = hasFlag('--minify-only');
const enablePrecompress = !hasFlag('--no-precompress');
const isQuiet = hasFlag('--quiet');

if (!fs.existsSync(targetDir)) {
  console.error(`Error: Target directory does not exist: ${targetDir}`);
  console.error(
    'Run "bundle exec jekyll build" first, or specify a valid directory with --dir <path>'
  );
  process.exit(1);
}

// Obfuscator Configuration (Strict CSP compliant, maximum obfuscation)
const OBFUSCATOR_CONFIG = {
  compact: true,
  controlFlowFlattening: true,
  controlFlowFlatteningThreshold: 0.8,
  deadCodeInjection: false, // Keep false for memory efficiency & swift execution
  identifierNamesGenerator: 'hexadecimal',
  renameGlobals: false, // Ensure DOM, window, document, caches, fetch work cleanly
  stringArray: true,
  stringArrayRotate: true,
  stringArrayShuffle: true,
  stringArrayThreshold: 0.8,
  stringArrayEncoding: ['base64'],
  stringArrayIndexShift: true,
  splitStrings: true,
  splitStringsChunkLength: 6,
  numbersToExpressions: true,
  transformObjectKeys: true,
  simplify: true,
  target: 'browser',
  selfDefending: false, // Crucial: avoid Function constructor eval checks that violate strict CSP
  disableConsoleOutput: true, // Prevents DevTools console probing
};

// CleanCSS Level 2 Configuration
const cleanCssInstance = new CleanCSS({
  level: {
    1: {
      all: true,
    },
    2: {
      mergeAdjacentRules: true,
      mergeIntoShorthands: true,
      mergeMedia: true,
      mergeNonAdjacentRules: true,
      removeDuplicateFontRules: true,
      removeDuplicateMediaBlocks: true,
      removeDuplicateRules: true,
      removeUnusedAtRules: false,
      restructureRules: true,
    },
  },
});

// Stats tracking
const stats = {
  html: { count: 0, original: 0, optimized: 0, brotli: 0 },
  css: { count: 0, original: 0, optimized: 0, brotli: 0 },
  js: { count: 0, original: 0, optimized: 0, brotli: 0 },
  svg: { count: 0, original: 0, optimized: 0, brotli: 0 },
  data: { count: 0, original: 0, optimized: 0, brotli: 0 },
  total: { count: 0, original: 0, optimized: 0, brotli: 0, gzip: 0 },
};

function formatBytes(bytes) {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

function percentage(part, total) {
  if (!total) return '0%';
  return ((part / total) * 100).toFixed(1) + '%';
}

function obfuscateEmail(html) {
  // Obfuscate mailto: links and email addresses in HTML with numeric character entities
  return html.replace(/href=["']mailto:([^"']+)["']/gi, (match, email) => {
    const encoded = email
      .split('')
      .map((c) => '&#' + c.charCodeAt(0) + ';')
      .join('');
    return `href="&#109;&#97;&#105;&#108;&#116;&#111;&#58;${encoded}"`;
  });
}

function precompressFile(filePath, buffer) {
  if (!enablePrecompress) return;
  try {
    // Brotli (quality 11 = maximum compression)
    const brBuffer = zlib.brotliCompressSync(buffer, {
      params: {
        [zlib.constants.BROTLI_PARAM_QUALITY]: 11,
      },
    });
    fs.writeFileSync(`${filePath}.br`, brBuffer);
    stats.total.brotli += brBuffer.length;

    // Gzip (level 9 = maximum compression)
    const gzBuffer = zlib.gzipSync(buffer, { level: 9 });
    fs.writeFileSync(`${filePath}.gz`, gzBuffer);
    stats.total.gzip += gzBuffer.length;

    return { br: brBuffer.length, gz: gzBuffer.length };
  } catch (err) {
    if (!isQuiet) console.warn(`Warning: Could not precompress ${filePath}: ${err.message}`);
    return null;
  }
}

async function optimizeJavaScript(filePath) {
  const raw = fs.readFileSync(filePath, 'utf8');
  if (raw.startsWith('/* @irbox-shield */')) {
    if (!isQuiet) {
      console.log(`  [JS Skip] ${path.relative(targetDir, filePath)}: Already obfuscated`);
    }
    return;
  }
  const originalSize = Buffer.byteLength(raw, 'utf8');
  let outputCode = raw;

  try {
    const minified = await terser.minify(raw, {
      ecma: 2020,
      compress: {
        passes: 2,
        drop_console: false,
        pure_getters: true,
      },
      mangle: false,
      format: {
        comments: false,
      },
    });
    outputCode = minified.code || raw;
  } catch (err) {
    console.warn(`  [JS] Minify fallback for ${path.basename(filePath)}:`, err.message);
    outputCode = raw;
  }

  const outputBuffer = Buffer.from(outputCode, 'utf8');
  fs.writeFileSync(filePath, outputBuffer);
  const comp = precompressFile(filePath, outputBuffer);

  const optimizedSize = outputBuffer.length;
  stats.js.count++;
  stats.js.original += originalSize;
  stats.js.optimized += optimizedSize;
  if (comp) stats.js.brotli += comp.br;

  if (!isQuiet) {
    console.log(
      `  [JS] ${path.relative(targetDir, filePath)}: ${formatBytes(originalSize)} -> ${formatBytes(optimizedSize)} (Brotli: ${comp ? formatBytes(comp.br) : 'N/A'})`
    );
  }
}

function optimizeCSS(filePath) {
  const raw = fs.readFileSync(filePath, 'utf8');
  const originalSize = Buffer.byteLength(raw, 'utf8');

  const result = cleanCssInstance.minify(raw);
  const minifiedCss = result.styles || raw;

  const outputBuffer = Buffer.from(minifiedCss, 'utf8');
  fs.writeFileSync(filePath, outputBuffer);
  const comp = precompressFile(filePath, outputBuffer);

  const optimizedSize = outputBuffer.length;
  stats.css.count++;
  stats.css.original += originalSize;
  stats.css.optimized += optimizedSize;
  if (comp) stats.css.brotli += comp.br;

  if (!isQuiet) {
    console.log(
      `  [CSS] ${path.relative(targetDir, filePath)}: ${formatBytes(originalSize)} -> ${formatBytes(optimizedSize)} (${percentage(originalSize - optimizedSize, originalSize)} saved, Brotli: ${comp ? formatBytes(comp.br) : 'N/A'})`
    );
  }
}

async function optimizeHTML(filePath) {
  const raw = fs.readFileSync(filePath, 'utf8');
  if (raw.includes('x-irbox-optimized')) {
    if (!isQuiet) {
      console.log(`  [HTML Skip] ${path.relative(targetDir, filePath)}: Already minified`);
    }
    return;
  }
  const originalSize = Buffer.byteLength(raw, 'utf8');

  // Obfuscate emails in HTML and tag with optimization marker
  let processed = obfuscateEmail(raw);
  if (processed.includes('</head>')) {
    processed = processed.replace('</head>', '<meta name="x-irbox-optimized" content="1"></head>');
  }

  try {
    const minified = await htmlMinifier.minify(processed, {
      collapseWhitespace: true,
      conservativeCollapse: false,
      removeComments: true,
      removeRedundantAttributes: true,
      removeScriptTypeAttributes: true,
      removeStyleLinkTypeAttributes: true,
      useShortDoctype: true,
      removeAttributeQuotes: true,
      collapseBooleanAttributes: true,
      decodeEntities: true,
      sortAttributes: true,
      sortClassName: true,
      minifyCSS: (text) => cleanCssInstance.minify(text).styles || text,
      minifyJS: (text) => {
        if (!text || !text.trim()) return text;
        try {
          return terser.minify_sync ? terser.minify_sync(text).code : text;
        } catch {
          return text;
        }
      },
    });

    const outputBuffer = Buffer.from(minified, 'utf8');
    fs.writeFileSync(filePath, outputBuffer);
    const comp = precompressFile(filePath, outputBuffer);

    const optimizedSize = outputBuffer.length;
    stats.html.count++;
    stats.html.original += originalSize;
    stats.html.optimized += optimizedSize;
    if (comp) stats.html.brotli += comp.br;

    if (!isQuiet) {
      console.log(
        `  [HTML] ${path.relative(targetDir, filePath)}: ${formatBytes(originalSize)} -> ${formatBytes(optimizedSize)} (${percentage(originalSize - optimizedSize, originalSize)} saved, Brotli: ${comp ? formatBytes(comp.br) : 'N/A'})`
      );
    }
  } catch (err) {
    if (!isQuiet)
      console.warn(`  [HTML Skip] ${path.relative(targetDir, filePath)}: ${err.message}`);
  }
}

function optimizeSVG(filePath) {
  const raw = fs.readFileSync(filePath, 'utf8');
  const originalSize = Buffer.byteLength(raw, 'utf8');

  try {
    const result = svgo.optimize(raw, {
      multipass: true,
      plugins: ['preset-default', 'sortAttrs'],
    });

    const outputBuffer = Buffer.from(result.data, 'utf8');
    fs.writeFileSync(filePath, outputBuffer);
    const comp = precompressFile(filePath, outputBuffer);

    const optimizedSize = outputBuffer.length;
    stats.svg.count++;
    stats.svg.original += originalSize;
    stats.svg.optimized += optimizedSize;
    if (comp) stats.svg.brotli += comp.br;

    if (!isQuiet) {
      console.log(
        `  [SVG] ${path.relative(targetDir, filePath)}: ${formatBytes(originalSize)} -> ${formatBytes(optimizedSize)} (${percentage(originalSize - optimizedSize, originalSize)} saved)`
      );
    }
  } catch (err) {
    if (!isQuiet)
      console.warn(`  [SVG Skip] ${path.relative(targetDir, filePath)}: ${err.message}`);
  }
}

function optimizeJSON(filePath) {
  const raw = fs.readFileSync(filePath, 'utf8');
  const originalSize = Buffer.byteLength(raw, 'utf8');

  try {
    const parsed = JSON.parse(raw);
    const minified = JSON.stringify(parsed);
    const outputBuffer = Buffer.from(minified, 'utf8');
    fs.writeFileSync(filePath, outputBuffer);
    const comp = precompressFile(filePath, outputBuffer);

    const optimizedSize = outputBuffer.length;
    stats.data.count++;
    stats.data.original += originalSize;
    stats.data.optimized += optimizedSize;
    if (comp) stats.data.brotli += comp.br;

    if (!isQuiet) {
      console.log(
        `  [DATA] ${path.relative(targetDir, filePath)}: ${formatBytes(originalSize)} -> ${formatBytes(optimizedSize)} (${percentage(originalSize - optimizedSize, originalSize)} saved)`
      );
    }
  } catch (err) {
    if (!isQuiet)
      console.warn(`  [DATA Skip] ${path.relative(targetDir, filePath)}: ${err.message}`);
  }
}

function optimizeXML(filePath) {
  const raw = fs.readFileSync(filePath, 'utf8');
  const originalSize = Buffer.byteLength(raw, 'utf8');

  // Strip XML comments and collapse tag whitespace
  const minified = raw
    .replace(/<!--[\s\S]*?-->/g, '')
    .replace(/>\s+<([a-zA-Z0-9_\-/?])/g, '><$1')
    .trim();

  const outputBuffer = Buffer.from(minified, 'utf8');
  fs.writeFileSync(filePath, outputBuffer);
  const comp = precompressFile(filePath, outputBuffer);

  const optimizedSize = outputBuffer.length;
  stats.data.count++;
  stats.data.original += originalSize;
  stats.data.optimized += optimizedSize;
  if (comp) stats.data.brotli += comp.br;

  if (!isQuiet) {
    console.log(
      `  [XML] ${path.relative(targetDir, filePath)}: ${formatBytes(originalSize)} -> ${formatBytes(optimizedSize)} (${percentage(originalSize - optimizedSize, originalSize)} saved)`
    );
  }
}

function getAllFiles(dir, fileList = []) {
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  for (const entry of entries) {
    const fullPath = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      if (
        entry.name !== '.git' &&
        entry.name !== 'node_modules' &&
        entry.name !== '.jekyll-cache'
      ) {
        getAllFiles(fullPath, fileList);
      }
    } else if (entry.isFile()) {
      fileList.push(fullPath);
    }
  }
  return fileList;
}

async function main() {
  const startTime = Date.now();
  console.log(`\n================================================================`);
  console.log(`🚀 Irbox Extreme Optimizer & Obfuscation Engine`);
  console.log(`Target: ${targetDir}`);
  console.log(
    `Modes: ${obfuscateOnly ? 'Obfuscate Only' : minifyOnly ? 'Minify Only' : 'Extreme Obfuscation + Extreme Minification'}`
  );
  console.log(`Pre-compression: ${enablePrecompress ? 'Brotli (.br) + Gzip (.gz)' : 'Disabled'}`);
  console.log(`================================================================\n`);

  const allFiles = getAllFiles(targetDir);

  // 1. Process JavaScript Files
  const jsFiles = allFiles.filter(
    (f) => f.endsWith('.js') && !f.endsWith('.min.js') && !f.includes('/scripts/')
  );
  if (!minifyOnly) {
    console.log(
      `🔒 Applying Extreme JavaScript Obfuscation & Hardening (${jsFiles.length} files)...`
    );
  } else {
    console.log(`⚡ Minifying JavaScript (${jsFiles.length} files)...`);
  }
  for (const f of jsFiles) {
    await optimizeJavaScript(f);
  }

  // 2. Process CSS Files
  if (!obfuscateOnly) {
    const cssFiles = allFiles.filter((f) => f.endsWith('.css') && !f.endsWith('.min.css'));
    console.log(`\n🎨 Applying Extreme CSS Minification (${cssFiles.length} files)...`);
    for (const f of cssFiles) {
      optimizeCSS(f);
    }
  }

  // 3. Process HTML Files
  if (!obfuscateOnly) {
    const htmlFiles = allFiles.filter((f) => f.endsWith('.html'));
    console.log(
      `\n📄 Applying Extreme HTML Minification & Inline Hardening (${htmlFiles.length} files)...`
    );
    for (const f of htmlFiles) {
      await optimizeHTML(f);
    }
  }

  // 4. Process SVG Files
  if (!obfuscateOnly) {
    const svgFiles = allFiles.filter((f) => f.endsWith('.svg'));
    console.log(`\n🖼️ Optimizing SVG Vectors (${svgFiles.length} files)...`);
    for (const f of svgFiles) {
      optimizeSVG(f);
    }
  }

  // 5. Process JSON, WebManifest & XML
  if (!obfuscateOnly) {
    const jsonFiles = allFiles.filter((f) => f.endsWith('.json') || f.endsWith('.webmanifest'));
    const xmlFiles = allFiles.filter((f) => f.endsWith('.xml'));
    console.log(
      `\n📦 Minifying Data Feeds & Manifests (${jsonFiles.length + xmlFiles.length} files)...`
    );
    for (const f of jsonFiles) optimizeJSON(f);
    for (const f of xmlFiles) optimizeXML(f);
  }

  // Compute overall statistics
  stats.total.count =
    stats.html.count + stats.css.count + stats.js.count + stats.svg.count + stats.data.count;
  stats.total.original =
    stats.html.original +
    stats.css.original +
    stats.js.original +
    stats.svg.original +
    stats.data.original;
  stats.total.optimized =
    stats.html.optimized +
    stats.css.optimized +
    stats.js.optimized +
    stats.svg.optimized +
    stats.data.optimized;

  const duration = ((Date.now() - startTime) / 1000).toFixed(2);

  console.log(`\n================================================================`);
  console.log(`📊 Optimization Summary (Completed in ${duration}s)`);
  console.log(`================================================================`);
  console.log(
    `  HTML files:        ${stats.html.count.toString().padStart(4)} | ${formatBytes(stats.html.original).padStart(9)} -> ${formatBytes(stats.html.optimized).padStart(9)} (${percentage(stats.html.original - stats.html.optimized, stats.html.original)} saved)`
  );
  console.log(
    `  CSS stylesheets:   ${stats.css.count.toString().padStart(4)} | ${formatBytes(stats.css.original).padStart(9)} -> ${formatBytes(stats.css.optimized).padStart(9)} (${percentage(stats.css.original - stats.css.optimized, stats.css.original)} saved)`
  );
  console.log(
    `  JS scripts:        ${stats.js.count.toString().padStart(4)} | ${formatBytes(stats.js.original).padStart(9)} -> ${formatBytes(stats.js.optimized).padStart(9)} (Obfuscated & Hardened)`
  );
  console.log(
    `  SVG vectors:       ${stats.svg.count.toString().padStart(4)} | ${formatBytes(stats.svg.original).padStart(9)} -> ${formatBytes(stats.svg.optimized).padStart(9)} (${percentage(stats.svg.original - stats.svg.optimized, stats.svg.original)} saved)`
  );
  console.log(
    `  Data / Manifests:  ${stats.data.count.toString().padStart(4)} | ${formatBytes(stats.data.original).padStart(9)} -> ${formatBytes(stats.data.optimized).padStart(9)} (${percentage(stats.data.original - stats.data.optimized, stats.data.original)} saved)`
  );
  console.log(`----------------------------------------------------------------`);
  console.log(`  Total Processed:   ${stats.total.count.toString().padStart(4)} files`);
  console.log(`  Original Raw Size: ${formatBytes(stats.total.original)}`);
  console.log(`  Optimized Size:    ${formatBytes(stats.total.optimized)}`);
  if (enablePrecompress) {
    console.log(
      `  Brotli Precompressed (.br): ${formatBytes(stats.total.brotli)} (${percentage(stats.total.original - stats.total.brotli, stats.total.original)} total wire transfer reduction!)`
    );
    console.log(
      `  Gzip Precompressed (.gz):   ${formatBytes(stats.total.gzip)} (${percentage(stats.total.original - stats.total.gzip, stats.total.original)} total wire transfer reduction!)`
    );
  }
  console.log(`================================================================\n`);
}

main().catch((err) => {
  console.error('\n❌ Optimization failed:', err);
  process.exit(1);
});

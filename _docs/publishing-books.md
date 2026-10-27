---
title: Publishing books (EPUB & PDF)
description: A simple guide for anyone to publish, archive, and read books in EPUB and PDF formats.
category: Writing content
---

This workspace includes built-in support for generating, archiving, and reading books and articles as **EPUB e-books** and **PDF documents**. Whether you are archiving reference manuals, publishing personal essays, or building a personal digital library, you can publish and read books online without complex command-line toolchains or paid reader subscriptions.

## 1. Quick export (One-Click)

You can download and read the complete workspace as an e-book or printable document at any time:

- **[Download complete EPUB book](/assets/book.epub)** — Optimized for Apple Books, Amazon Kindle, Kobo, and e-ink readers.
- **[Read in Web Book Reader](/books/reader/?book=irbox-docs.epub)** — Read right in your browser with adjustable typography, themes, and chapter navigation.
- **Print / Save as PDF** — Use the **Print book** button or press <kbd>Ctrl</kbd> + <kbd>P</kbd> (or <kbd>⌘</kbd> + <kbd>P</kbd> on Mac). The browser will render a clean, publication-ready PDF book with headers, page numbers, and page breaks.

---

## 2. Personal Book Archive (For Laymen)

You can maintain a personal digital library of books on your site. Anyone can add new books in two simple steps:

### Step 1: Drop your book into `books/archive/`

Copy any `.epub` or `.pdf` file directly into the `books/archive/` folder:

```text
books/archive/
├── irbox-docs.epub
├── architecture-primer.pdf
└── your-new-book.epub
```

### Step 2: Build the site

Rebuild the site using the standalone C SSG:

```bash
make build
```

The C SSG automatically mirrors your archive and presents all book metadata, cards, and reading controls at **`/books/`**.

---

## 3. The In-Browser Web Book Reader

Every book placed in your archive is immediately readable online at **`/books/reader/`**:

- **Client-Side Privacy**: EPUB decompression and rendering happen completely inside your browser memory using native Web APIs (`DecompressionStream`). Your books are never uploaded to third-party cloud services.
- **Adjustable Typography**: Switch between Literary Serif (`Libron`), Modern Sans (`Figtree`), and Monospace fonts, with interactive `A-` / `A+` font size controls.
- **Reading Themes**: Choose between **Light** (clean paper), **Sepia** (warm eye-comfort), **Dark** (night mode), and **Black** (OLED true black).
- **Table of Contents**: Slide out the TOC drawer at any time to jump directly to any chapter.
- **Keyboard Shortcuts**:
  - <kbd>&larr;</kbd> / <kbd>PageUp</kbd>: Previous chapter
  - <kbd>&rarr;</kbd> / <kbd>PageDown</kbd>: Next chapter
  - <kbd>F</kbd>: Toggle fullscreen reading
  - <kbd>Esc</kbd>: Close settings or TOC drawer
- **Automatic Bookmarking**: Your current chapter and reading position are automatically saved in `localStorage`, resuming right where you left off when you return.
- **Instant Drag-and-Drop Reader**: You can also drag and drop any local `.epub` or `.pdf` directly onto the `/books/` shelf to open and read it instantly without saving it to the server.

---

## 4. Reading your EPUB on external devices

The generated `.epub` file follows the modern **W3C EPUB 3.3 standard** while maintaining backward compatibility with EPUB 2 e-readers.

### Apple Books (iPhone, iPad, Mac)

1. Download the `.epub` file on your device.
2. Tap the file in your Downloads folder, or AirDrop it from your Mac to your iPhone or iPad.
3. It will automatically open in **Apple Books** with adjustable typography, night mode, and chapter navigation.

### Amazon Kindle

Amazon Kindle natively supports EPUB files:

1. Visit [amazon.com/sendtokindle](https://www.amazon.com/sendtokindle).
2. Drag and drop your `.epub` file.
3. The book will synchronize across your Kindle e-reader and the Kindle app on iOS and Android.

### Kobo, Android & Desktop

- **Android**: Open the file using **Google Play Books** or **ReadEra**.
- **Kobo**: Connect your Kobo via USB and drag the `.epub` into the device storage, or use Calibre.
- **Desktop**: Open in **Thorium Reader**, **Apple Books**, or **Calibre**.

---

## 5. How to write a book in Markdown

Writing a book from scratch in this workspace requires only standard Markdown files.

### Step 1: Add your chapter to `_docs/`

Create a new file in `_docs/`, for example `_docs/my-chapter.md`:

```markdown
---
title: My First Chapter
description: A short overview of what this chapter covers.
category: Part I
---

## Introduction

Write your content here in standard Markdown. You can include:

- Bullet points and lists
- Code blocks with syntax highlighting
- Quotes and tables
```

### Step 2: Assign your chapter category

Assign the `category` in front matter matching your target documentation section:

```yaml
---
title: My First Chapter
description: A short overview of what this chapter covers.
category: Writing content
---
```

### Step 3: Distribution

Your ready-to-read `.epub` edition is available in `assets/book.epub` and `books/archive/irbox-docs.epub`, containing all your documentation chapters for offline reading on Kindle, Kobo, or mobile e-readers.

---

## 6. Printing & exporting to PDF

The site features a dedicated **publication print stylesheet**:

1. Open any documentation page or post in your web browser.
2. Press <kbd>Ctrl</kbd> + <kbd>P</kbd> (or <kbd>⌘</kbd> + <kbd>P</kbd> on Mac), or click the **Print** button in the document header.
3. In the destination dropdown, select **Save as PDF**.
4. Set margins to **Default** or **Minimum** and ensure **Background graphics** is checked.
5. Click **Save**.

### Print stylesheet features

- **Page breaks**: Automatically places major sections on fresh pages while preventing awkward breaks inside code blocks and tables.
- **Footnotes & links**: Automatically prints the target URL in parentheses next to external links.
- **Zero website clutter**: Hides navigation bars, search modals, drawers, and buttons, producing a book-like appearance on paper.

---

## 7. Publishing to commercial stores

If you want to sell or distribute your book publicly:

- **Amazon KDP (Kindle Direct Publishing)**: Upload the `.epub` directly in the KDP dashboard; Amazon automatically converts it into their Kindle format.
- **Apple Books for Authors**: Submit your `.epub` file via the Apple Books Publishing Portal.
- **Google Play Books**: Upload your `.epub` file in the Google Play Books Partner Center.
- **Gumroad or Payhip**: Distribute your `.epub` and `.pdf` files directly to your audience.

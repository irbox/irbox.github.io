# Personal Book Archive (`books/archive/`)

Welcome to your personal digital book archive. This folder is designed for effortless, layman-friendly publishing and archiving of digital books.

## How to add books

1. **Drop your book files here:**
   Copy or save any `.epub` or `.pdf` file into this directory (`books/archive/`):
   - Example: `books/archive/my-favorite-novel.epub`
   - Example: `books/archive/cheat-sheet.pdf`

2. **Build the site:**
   Run the build command in your terminal:

   ```bash
   make build
   ```

   _Tip: The C SSG automatically mirrors your archive and makes your books immediately available!_

3. **Enjoy on your website:**
   - Browse your digital shelf at `/books/`
   - Read any book directly in your browser with chapter navigation, custom fonts, dark/sepia/light modes, and reading progress at `/books/reader/?book=your-filename.epub`
   - Download them to any device (Kindle, Kobo, Apple Books, Android, iPad, e-ink readers)

## Supported Formats

- **EPUB (`.epub`)**: Full in-browser reader support with typography controls, dark/sepia themes, Table of Contents, and reading position bookmarking.
- **PDF (`.pdf`)**: In-browser reading with native responsive viewer, zoom, and instant download fallback.

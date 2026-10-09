---
title: Writing docs
description: Add a documentation page and put it in the sidebar.
category: Writing content
---
1. Create `_docs/my-topic.md`:

   ```yaml
   ---
   title: My topic
   description: One sentence shown under the title and in search.
   category: Guides
   ---
   ```

2. Add `my-topic` to a section in `_data/docs_nav.yml`.
3. Write Markdown. `##` and `###` headings automatically appear in the "On this page" outline.

## Useful Markdown

Code blocks get syntax highlighting and a copy button. Blockquotes work well as notes.

```js
console.log("Hello, docs!");
```

> **Note:** Pages not listed in `docs_nav.yml` still build and are searchable, but won't appear in the sidebar.

## Linking between pages

Link with `{% raw %}{{ site.baseurl }}{% endraw %}` in front of absolute paths so links keep working when the site is served from a sub-path:

```markdown
[Getting started]({% raw %}{{ site.baseurl }}{% endraw %}/docs/getting-started/)
```

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
   category: Writing content
   ---
   ```

2. Assign a `category` matching one of the documentation sections:
   - `Introduction`
   - `Writing content`
   - `Customizing`
   - `My projects`
   - `Reference`

3. Write standard Markdown. `##` and `###` headings automatically appear in the sticky "On this page" outline.

## Useful Markdown

Code blocks get syntax highlighting and a one-click copy button. Blockquotes render as highlighted callouts.

```c
#include <stdio.h>

int main(void) {
    printf("Hello from C23 SSG!\n");
    return 0;
}
```

## Linking between pages

Link directly with absolute root paths:

```markdown
[Getting started](/docs/getting-started/)
```

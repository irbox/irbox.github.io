---
title: Writing posts
description: Publish a blog post.
category: Writing content
---
Create `_posts/YYYY-MM-DD-my-title.md`:

```yaml
---
title: My title
description: Shown on cards and in search.
tags: [Release, Notes]
cover: /assets/images/my-cover.png   # optional
cover_alt: Describe the image
author: me                           # key from _data/authors.yml
related_docs:
  - { title: Getting started, href: /docs/getting-started/ }
---
```

The newest post is featured at the top of the blog index. The index paginates (9 per page by default; see `paginate` in `_config.yml`).

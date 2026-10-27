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
cover: /assets/images/placeholders/cover-1.svg # optional
cover_alt: Describe the image
related_docs:
  - { title: Getting started, href: /docs/getting-started/ }
---
```

The newest post is featured prominently at the top of the blog index. Author profile information is pulled automatically from your `_config.cfg` settings (`author_name`, `author_role`, `author_avatar`).

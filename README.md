# msg
An incredibly opinionated, hackable, minimal Static Site Generator.

> See https://github.com/compromyse/compromyse.xyz for an example site.

### Compilation & Usage
```sh
# REQUIREMENTS: CMake, Git, GCC/Clang, GNUMake/Ninja
$ git clone https://github.com/compromyse/msg
$ mkdir build && cd build
$ cmake ..
$ make # or ninja
$ ./msg -h
msg: The Minimal Static Site Generator

Usage: ./msg [-h] [-w] [-v] [-o <output>] <directory>
        -h         : Help
        -w         : Watch working directory for changes
        -v         : Verbose
        -o <output>: Output directory
        <directory>: Working directory
```

### Site Structure
```sh
.
├── assets
│   └── me.webp
├── config.cfg
├── index.html
├── partials
│   ├── footer.html
│   ├── navbar.html
├── projects.html
└── templates
    └── base.html
```

### Features

#### Includes - include files from `/partials`

```html
<!-- index.html -->
<html>
  <body>
    {{ include "navbar.html" }}
    ...
  </body>
</html>
```

#### Contentfor - define content for templates

```html
<!-- templates/base.html -->
<html>
  <head>
    {{ content "head" }}
  </head>
  <body>
    {{ body }}
  </body>
</html>

<!-- index.html -->
{{ contentfor "head" }}
<title>HOME</title>
{{ endcontent }}

<p>...</p>
```

#### Eachdo - iterate over resources
```html
<!-- projects/xyz.html -->
title = XYZ
links = [
    href = https://example.org
    label = abc
    ___
    href = https://google.com
    label = test
]
---

<h1>XYZ!</h1>

<!-- index.html -->
{{ eachdo resources.projects }}
<p>{{ put title }}</p>
{{ endeachdo }}

{{ eachdo page.links }}
<p>{{ put href }}</p>
<p>{{ put label }}</p>
{{ endeachdo }}
```

```html
<!-- config.cfg -->
links = [
    href = https://github.com/compromyse
    label = GITHUB
    ___
    href = https://www.linkedin.com/in/compromyse
    label = LINKEDIN
]

<!-- index.html -->
{{ eachdo config.links }}
<a href="{{ put href }}" class="button" target="_blank">{{ put label }}</a>
{{ endeachdo }}
```

#### Template - specify template for page
```html
<!-- projects/xyz.html -->
template = base_tailwind.html
---

<h1 class="p-2">XYZ!</h1>
```

`Licensed under GPLv3`

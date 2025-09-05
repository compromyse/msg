# msg
An incredibly opinionated, hackable, minimal Static Site Generator.

> See https://github.com/compromyse/compromyse.xyz for an example site.

The point of this project was very simple; I was in the mood to write a static site generator. I figured I'd start with the most basic feature, "includes," and it ended up expanding to supporting nested for loops. All I required was a simple static site generator that doesn't overdo it. Besides, who doesn't like writing a lexer and generation engine in C.

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

The site structure is actually fairly simple. `/config.cfg` describes the static directories, in this case only "assets." It could be any number of files & folders, though.

Since I wanted to manually define all the resources (pages), I added a list `resources` in `/config.cfg` to encapsulate the collection of web pages. This was largely inspired by C build systems, where each C file is listed in an array of filenames (See [CMakeLists.txt for msg](https://github.com/compromyse/msg/blob/main/CMakeLists.txt#L6)).

The partials though, must be placed in the hardcoded `/partials` folder. They may, however, be placed in subdirectories therein. I didn't see much of a point allowing multiple partial directories (and likewise with templates).

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

This directive simply fetches the file content of the operand, in this case `/partials/navbar.html` and replaces the caller's body with it.

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

In this particular case, the template must have `content` directives, whose bodies are defined using `contentfor` directives. The example is fairly self-explanatory.

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

These are actually fairly complicated; EACHDOs iterate over some array of strings or nested-configs.

In the first example, the source being iterated over is the current page's `links` config. For each link, it's simply printing the respective href and label.

```html
<!-- projects/xyz.html -->
links = [
    href = https://example.org
    label = abc
    ___
    href = https://google.com
    label = test
]
---

<h1>XYZ!</h1>

{{ eachdo page.links }}
<p>{{ put href }}</p>
<p>{{ put label }}</p>
{{ endeachdo }}
```

With this example, `/index.html` is iterating over the `projects` resource (just the /projects directory excluding index.html). For each of the pages, it's simply printing the page's title in a paragraph tag.

```html
<!-- projects/xyz.html -->
title = XYZ
---

<h1>XYZ!</h1>

<!-- index.html -->
{{ eachdo resources.projects }}
<p>{{ put title }}</p>
{{ endeachdo }}
```

This example is much like the first one, but instead of the links being defined in the current page, it's defined in `/config.cfg`.

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

#### Page Options - specify template, priority in EACHDO iterations, etc.

Some page options can be defined in a page's config, such as the template that needs to be used, and the priority of this particular page when the resource containing the page (projects) is iterated over.

```html
<!-- projects/xyz.html -->
template = base_tailwind.html
priority = 100
---

<h1 class="p-2">XYZ!</h1>
```

`Licensed under GPLv3`

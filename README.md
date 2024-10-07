![github-header-image](https://github.com/user-attachments/assets/87620587-e955-410d-82d8-484e16513903)

## About The Project

There are a lot of static site generators out there already, but nothing particularly minimalist; everything felt bloated. Now, here I am rolling my own.

## Features
View `example_site` for an example.

#### Partials
The include directive, `{{ include "hello.html" }}`, includes the operand "hello.html" from the partials folder. It essentially just copies and pastes `partials/hello.html` in it's place. Partials can include other partials too.

```html
<html>
  <head></head>
  <body>
    {{ include "hello.html" }}
  </body>
</html>
```

#### TODO:
- [ ] Allow sub-directories.
- [ ] Config file.
- [x] Include directive.
- [ ] For loop directive.

## Compilation & Usage
All you need to compile `msg` is Clang and GNUMake. With both of them installed, run the following to compile `msg`.

```sh
make
```

Replace `[directory_name]` with your site's root directory. By default, this is taken as the current directory.
```sh
./build/msg [directory_name]
```

> P.S. If you'd like to use GCC instead of Clang for compilation, just run `make CC=gcc` instead.

## Development
Run the debug target to generate an executable for development.
```sh
make debug
```

You can also install [Bear](https://github.com/rizsotto/Bear) and run the following to generate compile_commands.json.
```sh
bear -- make
```

Run `make format` before pushing changes.

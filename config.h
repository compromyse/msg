#include <stdio.h>

#define DIRECTORY "compromyse.xyz"
#define PARTIALS "partials"
#define ASSETS "assets"
#define OUTPUT "dist"
#define BASE_TEMPLATE "base.html"

static const char *html_resources[] = { "index", "posts/a", NULL };
static const char *md_resources[] = { "posts/b", NULL };

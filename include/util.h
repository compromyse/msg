#ifndef __UTIL_H
#define __UTIL_H

typedef struct {
  void *ptr;
} ptr_wrapper_t;

char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

#endif

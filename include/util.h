#ifndef __UTIL_H
#define __UTIL_H

typedef struct {
  void *ptr;
} ptr_wrapper_t;

char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);
ptr_wrapper_t *wrap_ptr(void *ptr);
void *get_wrapped(ptr_wrapper_t *wrapper);

#endif

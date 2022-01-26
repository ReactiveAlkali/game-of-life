#ifndef DYN_STRING_H
#define DYN_STRING_H

#include <string.h>
#include <stdbool.h>

typedef struct STRING String;
struct STRING
{
  char *s;
  size_t size;
  size_t capacity;
};

String *
string_create ();

void
string_destroy (String *str);

bool
string_push_back (String *str, char c);

void
string_pop_back (String *str);

void
string_clear (String *str);

#endif

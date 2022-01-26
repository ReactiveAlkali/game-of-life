#include "String.h"
#include <stdlib.h>

static const size_t INIT_CAPACITY = 64;

/* double the capcity of the string */
static bool
string_expand (String *str)
{
  bool success = true;

  char *new_s = realloc(str->s, str->capacity * 2);
  if (!new_s)
    {
      success = false;
      goto done;
    }
  str->s        = new_s;
  str->capacity *= 2;

 done:
  return success;
}

String *
string_create ()
{
  String *new_string = malloc(sizeof(String));
  if (!new_string)
    goto done;

  new_string->capacity = INIT_CAPACITY;
  new_string->size     = 0;
  new_string->s        = malloc(sizeof(char) * INIT_CAPACITY);
  if (!new_string)
    {
      free(new_string);
      new_string = NULL;
      goto done;
    }
  new_string->s[0] = '\0';
 
 done:
  return new_string;
}

void
string_destroy (String *str)
{
  free(str->s);
  free(str);
}

bool
string_push_back (String *str, char c)
{
  bool success = true;

  /* try to resize if we're full */
  if (str->size == str->capacity - 1)
    {
      success = string_expand(str);
      if (!success)
        goto done;
    }

  str->s[str->size]     = c;
  str->s[str->size + 1] = '\0';
  ++(str->size);
      
 done:
  return success;
}

void
string_pop_back (String *str)
{
  if (str->size > 0)
    --str->size;
  str->s[str->size] = '\0';
}

void
string_clear (String *str)
{
  str->size = 0;
  str->s[0] = '\0';
}

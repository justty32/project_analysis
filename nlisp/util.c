#include "util.h"
#include "list.h"
#include <string.h>

void data_init(data_t *data) {
  data->ptr = NULL;
  data->size = 0;
  data->type = NULL;
}

void data_set(data_t *data, const char *type, void *ptr, size_t size) {
  data->type = type;
  data->ptr = ptr;
  data->size = size;
}

int data_is_type(const data_t *data, const char *type) {
  return !strcmp(data->type, type);
}

int data_is_true(const data_t *data) {
  if (data == NULL)
    return 0;
  if (data_is_type(data, DATATYPE_LIST)) {
    if (data->ptr == NULL) {
      return 0;
    }
  }
  return 1;
}

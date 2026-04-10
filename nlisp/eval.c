#include "eval.h"
#include <string.h>

static data_t exec(const char *symbol, list_node_t *args) {
  data_t ret;
  data_init(&ret);
  data_set(&ret, DATATYPE_LIST, args, sizeof(list_node_t *));
  if (symbol == NULL) {
    return ret;
  }
  if (!strcmp(symbol, "quote")) {
    return ret;
  } else if (!strcmp(symbol, "if")) {
    if (args == NULL) {
      return ret;
    } else {
      list_node_t *cond = list_get_node(args, 1);
      list_node_t *res_true = list_get_node(args, 2);
      list_node_t *res_false = list_get_node(args, 3);
      list_node_t *res = NULL;
      int cond_res = 0;
      if (cond != NULL)
        cond_res = data_is_true(&cond->data);
      res = cond_res ? res_true : res_false;
      if (res != NULL) {
        if (data_is_type(&res->data, DATATYPE_LIST)) {
          ret = eval(res);
        } else {
          ret = res->data;
        }
      } else {
        data_set(&ret, DATATYPE_LIST, NULL, sizeof(list_node_t *));
      }
      return ret;
    }
  } else if (!strcmp(symbol, "def")) {
  }
  return ret;
}

// (1 2 3) -> (1 2 3)
// (+ 1 2) -> 3
// (+) -> +
// () -> ()
// NULL -> ()
// ((+) 1 2) -> 3
// (quote (1 2 3)) -> (1 2 3)
// (1 (1 2 3)) -> (1 (1 2 3))
data_t eval(list_node_t *list) {
  data_t ret;
  data_init(&ret);
  // if arg is NULL, return empty list (NULL)
  if (list == NULL) {
    data_set(&ret, DATATYPE_LIST, NULL, sizeof(list_node_t *));
    return ret;
  }
  // if first element is list, eval it
  if (data_is_type(&list->data, DATATYPE_LIST)) {
    // evaluate
    data_t evaluated_data = eval((list_node_t *)list->data.ptr);
    // after eval, if it is list
    if (data_is_type(&evaluated_data, DATATYPE_LIST)) {
      // if returned another list, free old one
      if (list->data.ptr != evaluated_data.ptr && list->data.ptr != NULL) {
        list_free((list_node_t *)list->data.ptr);
      }
    }
    list->data = evaluated_data;
  }
  // after eval, if first element stil not atom, just return arg
  if (!data_is_type(&list->data, DATATYPE_ATOM)) {
    data_set(&ret, DATATYPE_LIST, list, sizeof(list_node_t *));
    return ret;
  }
  // execute the atom, the arg[0] is atom itself
  return exec(list->data.ptr, list);
}

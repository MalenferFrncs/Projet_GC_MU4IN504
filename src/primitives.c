#include <stdlib.h>
#include <stdio.h>

#include "mlvalues.h"
#include "memory.h"

#define MAKE_BIN_PRIM(name,op)                      \
  mlvalue ml_##name(mlvalue a, mlvalue b) {         \
    return Val_long(Long_val(a) op Long_val(b));    \
  }

MAKE_BIN_PRIM(add, +)
MAKE_BIN_PRIM(sub, -)
MAKE_BIN_PRIM(mul, *)
MAKE_BIN_PRIM(div, /)
MAKE_BIN_PRIM(or, ||)
MAKE_BIN_PRIM(and, &&)
MAKE_BIN_PRIM(ne, !=)
MAKE_BIN_PRIM(eq, ==)
MAKE_BIN_PRIM(lt, <)
MAKE_BIN_PRIM(le, <=)
MAKE_BIN_PRIM(gt, >)
MAKE_BIN_PRIM(ge, >=)


mlvalue ml_not(mlvalue a) {
	return Val_long(!Long_val(a));
}


mlvalue ml_print(mlvalue a) {
  if (Is_long(a)) {
    putchar(Long_val(a));
  } else {
    for (unsigned int i = 0; i < Size(a); i++) {
      ml_print(Field(a,i));
    }
  }
  return Val_long(0);
}

#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H

#include "mlvalues.h"

enum primitives { ADD, AND, DIV, EQ, GE, GT, LE, LT, MUL, NE, NOT, OR, PRINT, SUB };

mlvalue ml_add(mlvalue a, mlvalue b);
mlvalue ml_sub(mlvalue a, mlvalue b);
mlvalue ml_mul(mlvalue a, mlvalue b);
mlvalue ml_div(mlvalue a, mlvalue b);
mlvalue ml_or(mlvalue a, mlvalue b);
mlvalue ml_and(mlvalue a, mlvalue b);
mlvalue ml_not(mlvalue a);
mlvalue ml_ne(mlvalue a, mlvalue b);
mlvalue ml_eq(mlvalue a, mlvalue b);
mlvalue ml_lt(mlvalue a, mlvalue b);
mlvalue ml_le(mlvalue a, mlvalue b);
mlvalue ml_gt(mlvalue a, mlvalue b);
mlvalue ml_ge(mlvalue a, mlvalue b);
mlvalue ml_print(mlvalue a);


#endif /* _PRIMITIVES_H */

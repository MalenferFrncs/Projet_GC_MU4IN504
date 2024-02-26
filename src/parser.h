#ifndef _PARSER_H
#define _PARSER_H

#include "mlvalues.h"
#include "instruct.h"
#include "primitives.h"

/* Parses the code contained in file |filename| */
code_t* parse(char* filename);

#endif /* _PARSER_H */

#ifndef _MLVALUES_H
#define _MLVALUES_H

#include <stdint.h>

typedef int64_t mlvalue;
typedef uint64_t header_t;
typedef enum { WHITE, GRAY, BLACK } color_t;
typedef enum { ENV_T, CLOSURE_T, BLOCK_T } tag_t;

/* If a mlvalue ends with 1, it's an integer, otherwise it's a pointer. */
#define Is_long(v)  (((v) & 1) != 0)
#define Is_block(v) (((v) & 1) == 0)

#define Val_long(v) (((v) << 1) + 1)
#define Long_val(v) (((uint64_t)(v)) >> 1)

#define Val_ptr(p) ((mlvalue)(p))
#define Ptr_val(v) ((mlvalue*)(v))
#define Val_hd(hd) ((mlvalue)(hd))

/* Structure of the header:
     +--------+-------+-----+
     | size   | color | tag |
     +--------+-------+-----+
bits  63    10 9     8 7   0
*/
#define Size_hd(hd)  ((hd) >> 10)
#define Color_hd(hd) (((hd) >> 8) & 3)
#define Tag_hd(hd)   ((hd) & 0xFF)

#define Hd_val(v) (((header_t*)(v))[-1])
#define Field(v,n) (Ptr_val(v)[n])
#define Field0(v) Field(v,0)
#define Field1(v) Field(v,1)
#define Size(v) Size_hd(Hd_val(v))
#define Color(v) Color_hd(Hd_val(v))
#define Tag(v)  Tag_hd(Hd_val(v))

#define WHITE 0
#define GRAY 1
#define BLACK 2
#define Make_header(size,color,tag)                                     \
  ((header_t)(((size) << 10) | (((color) & 3) << 8) | ((tag) & 0xFF)))

#define Addr_closure(c) Long_val(Field0(c))
#define Env_closure(c)  Field1(c)

mlvalue make_empty_block(tag_t tag);
mlvalue make_block(size_t size, tag_t tag);

#define Make_empty_env() make_empty_block(ENV_T)
#define Make_env(size) make_block(size,ENV_T)

mlvalue make_closure(uint64_t addr, mlvalue env);

#define Unit Val_long(0)


void print_val(mlvalue val);
char* val_to_str(mlvalue val);


/* A bytecode is represented as a uint64_t. */
typedef uint64_t code_t;

int print_instr(code_t* prog, int pc);
void print_prog(code_t* code);


#endif /* _MLVALUES_H */

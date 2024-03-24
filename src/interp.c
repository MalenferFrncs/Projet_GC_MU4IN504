#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "domain_state.h"
#include "interp.h"
#include "config.h"
#include "mlvalues.h"
#include "instruct.h"
#include "alloc.h"
#include "primitives.h"

/* Helpers to manipulate the stack. Note that |Caml_state->sp| always point to the
   first empty element in the stack; hence the prefix -- in POP, but
   postfix ++ in PUSH. */
#define POP_STACK() stack[--Caml_state->sp]
#define PUSH_STACK(x) stack[Caml_state->sp++] = x

mlvalue caml_interprete(code_t *prog)
{

  mlvalue *stack = Caml_state->stack;

  register unsigned int pc = 0;
  unsigned int extra_args = 0;
  unsigned int trap_sp = 0;

  while (1)
  {

#ifdef DEBUG
    printf("pc=%d  Caml_state->accu=%s  Caml_state->sp=%d extra_args=%d trap_sp=%d stack=[",
           pc, val_to_str(Caml_state->accu), Caml_state->sp, extra_args, trap_sp);
    if (Caml_state->sp > 0)
    {
      printf("%s", val_to_str(stack[Caml_state->sp - 1]));
    }
    for (int i = Caml_state->sp - 2; i >= 0; i--)
    {
      printf(";%s", val_to_str(stack[i]));
    }
    printf("]  Caml_state->env=%s\n", val_to_str(Caml_state->env));
    print_instr(prog, pc);
#endif

    switch (prog[pc++])
    {
    case CONST:
      Caml_state->accu = Val_long(prog[pc++]);
      break;

    case PRIM:
      switch (prog[pc++])
      {
      case ADD:
        Caml_state->accu = ml_add(Caml_state->accu, POP_STACK());
        break;
      case SUB:
        Caml_state->accu = ml_sub(Caml_state->accu, POP_STACK());
        break;
      case DIV:
        Caml_state->accu = ml_div(Caml_state->accu, POP_STACK());
        break;
      case MUL:
        Caml_state->accu = ml_mul(Caml_state->accu, POP_STACK());
        break;
      case OR:
        Caml_state->accu = ml_or(Caml_state->accu, POP_STACK());
        break;
      case AND:
        Caml_state->accu = ml_and(Caml_state->accu, POP_STACK());
        break;
      case NOT:
        Caml_state->accu = ml_not(Caml_state->accu);
        break;
      case NE:
        Caml_state->accu = ml_ne(Caml_state->accu, POP_STACK());
        break;
      case EQ:
        Caml_state->accu = ml_eq(Caml_state->accu, POP_STACK());
        break;
      case LT:
        Caml_state->accu = ml_lt(Caml_state->accu, POP_STACK());
        break;
      case LE:
        Caml_state->accu = ml_le(Caml_state->accu, POP_STACK());
        break;
      case GT:
        Caml_state->accu = ml_gt(Caml_state->accu, POP_STACK());
        break;
      case GE:
        Caml_state->accu = ml_ge(Caml_state->accu, POP_STACK());
        break;
      case PRINT:
        Caml_state->accu = ml_print(Caml_state->accu);
        break;
      }
      break;

    case BRANCH:
      pc = prog[pc];
      break;

    case BRANCHIFNOT:
      if (Long_val(Caml_state->accu) == 0)
      {
        pc = prog[pc];
      }
      else
      {
        pc++;
      }
      break;

    case PUSH:
      PUSH_STACK(Caml_state->accu);
      break;

    case POP:
      POP_STACK();
      break;

    case ACC:
      Caml_state->accu = stack[Caml_state->sp - prog[pc++] - 1];
      break;

    case ENVACC:
      Caml_state->accu = Field(Caml_state->env, prog[pc++]);
      break;

    case APPLY:
    {                          // Modifié
      uint64_t n = prog[pc++]; /*
      mlvalue* tmp = malloc(n * sizeof(mlvalue)); // TODO: remove malloc
      for (uint64_t i = 0; i < n; i++) {
        tmp[i] = POP_STACK();
      }
      PUSH_STACK(Caml_state->env);
      PUSH_STACK(Val_long(pc));
      PUSH_STACK(Val_long(extra_args));
      /* push in reverse order to keep the initial order
      for (int i = n-1; i >= 0; i--) {
        PUSH_STACK(tmp[i]);
      }
      free(tmp);  */
      if (Caml_state->sp + 3 > Stack_size)
      {
        printf("stack overflow\n");
        return EXIT_FAILURE;
      }
      uint64_t sp_val = Caml_state->sp - 1;
      for (uint64_t i = 0; i < n; i++)
      {
        stack[sp_val - i + 3] = stack[sp_val - i];
      }
      stack[Caml_state->sp - n] = Caml_state->env;
      stack[Caml_state->sp - n + 1] = Val_long(pc);
      stack[Caml_state->sp - n + 2] = Val_long(extra_args);
      Caml_state->sp = Caml_state->sp + 3;
      pc = Addr_closure(Caml_state->accu);
      Caml_state->env = Env_closure(Caml_state->accu);
      extra_args = n - 1;
      break;
    }

    case APPTERM:
    { // Modifié
      uint64_t n = prog[pc++];
      uint64_t m = prog[pc++]; /*
      mlvalue* tmp = malloc(n * sizeof(mlvalue)); // TODO: remove malloc
      for (uint64_t i = 0; i < n; i++) {
        tmp[i] = POP_STACK();
      }
      for (uint64_t i = 0; i < m-n; i++) {
        POP_STACK();
      }
      /* push in reverse order to keep the initial order
      for (int i = n-1; i >= 0; i--) {
        PUSH_STACK(tmp[i]);
      }
      free(tmp);  */

      uint64_t base = Caml_state->sp - n - (m - n); /*plancher des adresses*/
      for (uint64_t i = 0; i < n; i++)
      {
        stack[base + i] = stack[Caml_state->sp - n + i];
      }
      Caml_state->sp = Caml_state->sp - (m - n);
      pc = Addr_closure(Caml_state->accu);
      Caml_state->env = Env_closure(Caml_state->accu);
      extra_args += n - 1;
      break;
    }

    case RETURN:
    {
      uint64_t n = prog[pc++];
      for (uint64_t i = 0; i < n; i++)
      {
        POP_STACK();
      }
      if (extra_args == 0)
      {
        extra_args = Long_val(POP_STACK());
        pc = Long_val(POP_STACK());
        Caml_state->env = POP_STACK();
      }
      else
      {
        extra_args--;
        pc = Addr_closure(Caml_state->accu);
        Caml_state->env = Env_closure(Caml_state->accu);
      }
      break;
    }

    case RESTART:
    {
      unsigned int n = Size(Caml_state->env);
      for (unsigned int i = n - 1; i > 0; i--)
      {
        PUSH_STACK(Field(Caml_state->env, i));
      }
      Caml_state->env = Field(Caml_state->env, 0);
      extra_args += n - 1;
      break;
    }

    case GRAB:
    {
      uint64_t n = prog[pc++];
      if (extra_args >= n)
      {
        extra_args -= n;
      }
      else
      {
        mlvalue closure_env = Make_env(extra_args + 2);
        Field(closure_env, 0) = Caml_state->env;
        for (unsigned int i = 0; i <= extra_args; i++)
        {
          Field(closure_env, i + 1) = POP_STACK();
        }
        Caml_state->accu = make_closure(pc - 3, closure_env);
        extra_args = Long_val(POP_STACK());
        pc = Long_val(POP_STACK());
        Caml_state->env = POP_STACK();
      }
      break;
    }

    case CLOSURE:
    {
      uint64_t addr = prog[pc++];
      uint64_t n = prog[pc++];
      if (n > 0)
      {
        PUSH_STACK(Caml_state->accu);
      }
      mlvalue closure_env = Make_env(n + 1);
      Field(closure_env, 0) = Val_long(addr);
      for (uint64_t i = 0; i < n; i++)
      {
        Field(closure_env, i + 1) = POP_STACK();
      }
      Caml_state->accu = make_closure(addr, closure_env);
      break;
    }

    case CLOSUREREC:
    {
      uint64_t addr = prog[pc++];
      uint64_t n = prog[pc++];
      if (n > 0)
      {
        PUSH_STACK(Caml_state->accu);
      }
      mlvalue closure_env = Make_env(n + 1);
      Field(closure_env, 0) = Val_long(addr);
      for (uint64_t i = 0; i < n; i++)
      {
        Field(closure_env, i + 1) = POP_STACK();
      }
      Caml_state->accu = make_closure(addr, closure_env);
      PUSH_STACK(Caml_state->accu);
      break;
    }

    case OFFSETCLOSURE:
    {
      Caml_state->accu = make_closure(Long_val(Field(Caml_state->env, 0)), Caml_state->env);
      break;
    }

    case MAKEBLOCK:
    {
      uint64_t n = prog[pc++];
      mlvalue blk = make_block(n, BLOCK_T);
      if (n > 0)
      {
        Field(blk, 0) = Caml_state->accu;
        for (unsigned int i = 1; i < n; i++)
        {
          Field(blk, i) = POP_STACK();
        }
      }
      Caml_state->accu = blk;
      break;
    }

    case GETFIELD:
    {
      uint64_t n = prog[pc++];
      Caml_state->accu = Field(Caml_state->accu, n);
      break;
    }

    case VECTLENGTH:
    {
      Caml_state->accu = Val_long(Size(Caml_state->accu));
      break;
    }

    case GETVECTITEM:
    {
      uint64_t n = Long_val(POP_STACK());
      Caml_state->accu = Field(Caml_state->accu, n);
      break;
    }

    case SETFIELD:
    {
      uint64_t n = prog[pc++];
      Field(Caml_state->accu, n) = POP_STACK();
      break;
    }

    case SETVECTITEM:
    {
      uint64_t n = Long_val(POP_STACK());
      mlvalue v = POP_STACK();
      Field(Caml_state->accu, n) = v;
      Caml_state->accu = Unit;
      break;
    }

    case ASSIGN:
    {
      uint64_t n = prog[pc++];
      stack[Caml_state->sp - n - 1] = Caml_state->accu;
      Caml_state->accu = Unit;
      break;
    }

    case PUSHTRAP:
    {
      uint64_t addr = prog[pc++];
      PUSH_STACK(Val_long(extra_args));
      PUSH_STACK(Caml_state->env);
      PUSH_STACK(Val_long(trap_sp));
      PUSH_STACK(Val_long(addr));
      trap_sp = Caml_state->sp;
      break;
    }

    case POPTRAP:
    {
      POP_STACK(); // popping pc
      trap_sp = Long_val(POP_STACK());
      POP_STACK(); // popping Caml_state->env
      POP_STACK(); // popping extra_args
      break;
    }

    case RAISE:
    {
      if (trap_sp == 0)
      {
        fprintf(stderr, "Uncaught exception: %s\n", val_to_str(Caml_state->accu));
        exit(EXIT_FAILURE);
      }
      else
      {
        Caml_state->sp = trap_sp;
        pc = Long_val(POP_STACK());
        trap_sp = Long_val(POP_STACK());
        Caml_state->env = POP_STACK();
        extra_args = Long_val(POP_STACK());
        break;
      }
    }

    case STOP:
      return Caml_state->accu;

    default:
      fprintf(stderr, "Unkown bytecode: %lu at offset %d\n", prog[pc - 1], pc - 1);
      exit(EXIT_FAILURE);
    }
  }
}

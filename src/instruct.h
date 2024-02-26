#ifndef _INSTRUCT_H
#define _INSTRUCT_H

enum instructions {
  CONST, PRIM, BRANCH, BRANCHIFNOT, PUSH, POP, ACC,
  ENVACC, CLOSURE, APPLY, RETURN, STOP,
  CLOSUREREC, OFFSETCLOSURE,
  GRAB, RESTART,
  APPTERM,
  MAKEBLOCK, GETFIELD, VECTLENGTH, GETVECTITEM,
  SETFIELD, SETVECTITEM, ASSIGN,
  PUSHTRAP, POPTRAP, RAISE
};

#define Has_arg2(i) (i == CLOSURE || i == CLOSUREREC || i == APPTERM)

#define Has_arg1(i) (Has_arg2(i) || (i == CONST) || (i == PRIM) ||         \
                     (i == BRANCH) || (i == BRANCHIFNOT) || (i == ACC) ||  \
                     (i == ENVACC) || (i == APPLY) || (i == RETURN) ||     \
                     (i == GRAB) || (i == MAKEBLOCK) || (i == GETFIELD) || \
                     (i == SETFIELD) || (i == ASSIGN) || (i == PUSHTRAP))



#define Arg1_is_label(i) (i == BRANCH || i == BRANCHIFNOT ||    \
                          i == CLOSURE || i == CLOSUREREC ||    \
                          i == PUSHTRAP)

#endif /* _INSTRUCT_H */

#include "minic.h"

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments\n", argv[0]);
  
  Token *tok = tokenize(argv[1]);
  Function *prog = parse(tok);
  
  // traverse the AST to emit assembly
  codegen(prog);

  return 0;
}

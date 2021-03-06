#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"


/* add Windows portability */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif
/**/


int main(int argc, char** argv) {

  /* Create Parsers*/
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  /* Define grammars */
  mpca_lang(MPCA_LANG_DEFAULT,
            "\
            number : /-?[0-9]+/ ;                           \
            operator : '+' | '-' | '*' | '/' ;              \
            expr : <number> | '(' <operator> <expr>+ ')' ;  \
            lispy : /^/ <operator> <expr>+ /$/ ;            \
            "
            , Number, Operator, Expr, Lispy);
  
  puts("Lispy version 0.0.0.0.2");
  puts("Press Ctrl+c to Exit\n");

  while (1) {
    char* input = readline("pew pew nya~> ");
    add_history(input);

    /* Attempt to parse input*/
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      /* Load abstract syntax tree from output */
      mpc_ast_t* ast = r.output;
      printf("Tag: %s\n", ast->tag);
      printf("contents: %s\n", ast->contents);
      printf("Number of children: %i\n\n"
             , ast->children_num);

      /* Get first child*/
      mpc_ast_t* child_0 = ast->children[0];
      printf("To me, your first born belongs!\n");
      printf("First Child Tag: %s\n", child_0->tag);
      printf("First Child Number of Children: %i\n\n"
             , child_0->children_num);

      /* Get all children info for funs */
      int children = ast->children_num;

      for(int n =0; n < children; ++n) {
        mpc_ast_t* child_n = ast->children[n];

        printf("Child number: %i\n", n);
        printf("Tag: %s\n", child_n->tag);
        printf("Contents: %s\n", child_n->contents);
        printf("Number of children for this Child: %i\n\n"
               , child_n->children_num);
      }

    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  /* Undefine and Delete our Parsers */
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  
  return 0;
}

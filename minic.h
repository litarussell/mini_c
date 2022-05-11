#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> // 用于支持变长参数
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

typedef struct Node Node;

// 
// tokenizer.c
// 

// token
typedef enum {
  TK_IDENT,   // Identifiers
  TK_PUNCT,   // 标点符号 punctuators
  TK_KEYWORD, // keyword
  TK_NUM,     // 数字
  TK_EOF,     // 文件结束标识符
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
  TokenKind kind; // Token类型
  Token *next;
  int val;        // if kind is TK_NUM, its value
  char *loc;      // Token location
  int len;        // Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);

// 
// parser.c
// 

// local variable
typedef struct Obj Obj;
struct Obj {
  Obj *next;
  char *name; // variable name
  int offset; // offset of RBP
};

// Function
typedef struct Function Function;
struct Function {
  Node *body;
  Obj *locals;
  int stack_size;
};

// AST node
typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_NEG,       // unary -
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_ASSIGN,    // =
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_FOR,       // "for" or "while"
  ND_BLOCK,     // { ... }
  ND_EXPR_STMT, // Expression statement
  ND_VAR,       // Variable
  ND_NUM,       // 整型
} NodeKind;

// AST node type
struct Node {
  NodeKind kind;  
  Node *next;
  Node *lhs;
  Node *rhs;

  Node *body;     // block

  // "if" or "for" statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  Obj *var;       // used if kind == ND_VAR
  int val;        // used if kind == ND_NUM
};

Function *parse(Token *tok);


// 
// codegen.c
// 

void codegen(Function *prog);

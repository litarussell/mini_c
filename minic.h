#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> // 用于支持变长参数
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

// 
// tokenizer.c
// 

typedef enum {
  TK_IDENT, // Identifiers
  TK_PUNCT, // 标点符号 punctuators
  TK_NUM,   // 数字
  TK_EOF,   // 文件结束标识符
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
  ND_EXPR_STMT, // Expression statement
  ND_VAR,       // Variable
  ND_NUM,       // 整型
} NodeKind;


// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *next;
  Node *lhs;
  Node *rhs;
  char name;      // used if kind == ND_VAR
  int val;        // used if kind == ND_NUM
};

Node *parse(Token *tok);


// 
// codegen.c
// 

void codegen(Node *node);

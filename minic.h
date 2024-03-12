#pragma once

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> // 用于支持变长参数
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

typedef struct Type Type;
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
bool consume(Token **rest, Token *tok, char *str);
Token *tokenize(char *input);

// 
// parser.c
// 

// local variable
typedef struct Obj Obj;
struct Obj {
  Obj *next;
  char *name; // variable name
  Type *ty;   // Type
  int offset; // offset of RBP
};

// Function
typedef struct Function Function;
struct Function {
  Function *next;
  char *name;
  Obj *params;

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
  ND_ADDR,      // unary &
  ND_DEREF,     // unary *
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_FOR,       // "for" or "while"
  ND_BLOCK,     // { ... }
  ND_FUNCALL,   // Function call
  ND_EXPR_STMT, // Expression statement
  ND_VAR,       // Variable
  ND_NUM,       // 整型
} NodeKind;

// AST node type
struct Node {
  NodeKind kind;  
  Node *next;
  Type *ty;       // Type, e.g. int or pointer to int
  Token *tok;     // representative token

  Node *lhs;
  Node *rhs;

  Node *body;     // block

  char *funcname; // function name
  Node *args;     // 函数参数

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
// type.c
//

typedef enum {
  TY_INT,
  TY_PTR,
  TY_FUNC,
  TY_ARRAY,
} TypeKind;

struct Type {
  TypeKind kind;
  int size;

  Type *base;       // 指针

  Token *name;      // 变量、函数名称

  int array_len;    // 数组的长度

  Type *return_ty;  // 函数返回类型
  Type *params;     // 函数形参
  Type *next;       // 下一个类型
};

// 声明一个全局变量, 定义在type.c中
extern Type *ty_int;

// 判断是否为整型
bool is_integer(Type *ty);
// 复制类型
Type *copy_type(Type *ty);
// 构建一个指针类型, 并指向基类
Type *pointer_to(Type *base);
// 构建一个函数类型
Type *func_type(Type *return_ty);
// 构造一个数组类型
Type *array_of(Type *base, int size);
// 为节点内的所有节点添加类型
void add_type(Node *node);

// 
// codegen.c
// 语义分析与代码生成
// 

void codegen(Function *prog);

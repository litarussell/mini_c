#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> // 用于支持变长参数
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

typedef enum {
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

// 省略号 函数参数表
static void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// consume the current token if it matches `op`
static bool equal(Token *tok, char *op) {
  return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

// ensure that the current token is `s`
static Token *skip(Token *tok, char *s) {
  if (!equal(tok, s))
    error("expected '%s'", s);
  return tok->next;
}

// ensure that the current token is TK_NUM
static int get_number(Token *tok) {
  if (tok->kind != TK_NUM)
    error("expected a number");
  return tok->val;
}

// create a new token
static Token *new_token(TokenKind kind, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

// Tokenize `p` and renturn new tokens
static Token *tokenize(char *p) {
  Token head = {};
  Token *cur = &head;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    // number
    if (isdigit(*p)) {
      cur = cur->next = new_token(TK_NUM, p, p);
      char *q = p;
      cur->val = strtoul(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // punctuator
    if (*p == '+' || *p == '-') {
      cur = cur->next = new_token(TK_PUNCT, p, p + 1);
      p++;
      continue;
    }

    error("invalid token");
  }

  cur = cur->next = new_token(TK_EOF, p, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: invalid number of arguments\n", argv[0]);

  Token *tok = tokenize(argv[1]);

  printf("  .global main\n");
  printf("main:\n");

  // the first token must be a number
  printf("  mov $%d, %%rax\n", get_number(tok)); // "5+20-4"

  tok = tok->next;

  while (tok->kind != TK_EOF) {
    if (equal(tok, "+")) {
      printf("  add $%d, %%rax\n", get_number(tok->next));
      tok = tok->next->next;
      continue;
    }

    tok = skip(tok, "-");
    printf("  sub $%d, %%rax\n", get_number(tok));
    tok = tok->next;
  }
  
  printf("  ret\n");
  return 0;
}

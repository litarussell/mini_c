#include "minic.h"

Type *ty_int = &(Type){TY_INT};

bool is_integer(Type *ty) {
  return ty->kind == TY_INT;
}

Type *copy_type(Type *ty) {
  Type *ret = calloc(1, sizeof(Type));
  *ret = *ty;
  return ret;
}

Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->base = base;
  return ty;
}

// 设置函数类型
Type *func_type(Type *return_ty) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_FUNC;
  ty->return_ty = return_ty;
  return ty;
}

void add_type(Node *node) {
  if (!node || node->ty)
    return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);

  for (Node *n = node->body; n; n = n->next)
    add_type(n);
  
  // 为函数参数增加类型信息
  for (Node *n = node->args; n; n = n->next)
    add_type(n);

  switch (node->kind) {
  // 将节点类型设置为 节点左部的类型
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_NEG:
  case ND_ASSIGN:
    node->ty = node->lhs->ty;
    return;
  // 将节点类型设置为 int
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_NUM:
  case ND_FUNCALL:
    node->ty = ty_int;
    return;
  // 将节点类型设置为 变量的类型
  case ND_VAR:
    node->ty = node->var->ty;
    return;
  // 将节点类型设置为 指针, 并指向左部的类型
  case ND_ADDR:
    node->ty = pointer_to(node->lhs->ty);
    return;
  // 节点类型: 如果解引用指向的是指针, 则为指针指向的类型; 否则就报错
  case ND_DEREF:
    if (node->lhs->ty->kind != TY_PTR)
      error_tok(node->tok, "invalid pointer dereference");
    node->ty = node->lhs->ty->base;
    return;
  default:
    break;
  }
}

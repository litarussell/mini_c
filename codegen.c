#include "minic.h"

static int depth;
static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static Function *current_fn;

static void gen_expr(Node *node);

static int count(void) {
  static int i = 1;
  return i++;
}

static void push(void) {
  printf("  push %%rax\n");
  depth++;
}

static void pop(char *arg) {
  printf("  pop %s\n", arg);
  depth--;
}

// Round up `n` to the nearest multiple of `align`. for instance,
// align_to(5, 8) returns 8 and align_to(11, 8) return 16
static int align_to(int n, int align) {
  return (n + align - 1) / align * align;
}

// compute the absolute address of a given node
// it's an error if a given node does not reside memory
static void gen_addr(Node *node) {
  switch(node->kind) {
  case ND_VAR:
    // 偏移量是相对于rbp的
    printf("  # 获取变量%s的栈内地址为%d(rbp)\n", node->var->name, node->var->offset);
    printf("  lea %d(%%rbp), %%rax\n", node->var->offset);
    return;
  // 解引用
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  default:
    break;
  }

  error_tok(node->tok, "not an lvalue");
}

// generate code for a given node
static void gen_expr(Node *node) {
  // 生成各个根节点
  switch (node->kind) {
  case ND_NUM:
    // 加载数字到rax寄存器中
    printf("  # 加载立即数到rax寄存器中\n");
    printf("  mov $%d, %%rax\n", node->val);
    return;
  case ND_NEG:
    gen_expr(node->lhs);
    printf("  neg %%rax\n");
    return;
  case ND_VAR:
    gen_addr(node);
    printf("  mov (%%rax), %%rax\n");
    return;
  // 解引用
  case ND_DEREF:
    gen_expr(node->lhs);
    printf("  # 读取寄存器rax中存放的地址, 得到的值存入rax中\n");
    printf("  mov (%%rax), %%rax\n");
    return;
  case ND_ADDR:
    gen_addr(node->lhs);
    return;
  // 赋值
  case ND_ASSIGN:
    gen_addr(node->lhs);
    push();
    gen_expr(node->rhs);
    pop("%rdi");
    // 赋值
    printf("  mov %%rax, (%%rdi)\n");
    return;
  // 函数调用
  case ND_FUNCALL: {
    int nargs = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      gen_expr(arg);
      push();
      nargs++;
    }

    for (int i = nargs - 1; i >= 0; i--)
      pop(argreg[i]);
    
    printf("  mov $0, %%rax\n");
    printf("  call %s\n", node->funcname);
    return;
  }
  default:
    break;
  }

  // 递归到最右节点
  gen_expr(node->rhs);
  // 结果压栈
  push();
  gen_expr(node->lhs);
  pop("%rdi");

  switch (node->kind) {
  case ND_ADD:
    printf("  add %%rdi, %%rax\n");
    return;

  case ND_SUB:
    printf("  sub %%rdi, %%rax\n");
    return;
  
  case ND_MUL:
    printf("  imul %%rdi, %%rax\n");
    return;

  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv %%rdi\n");
    return;
  
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
    printf("  cmp %%rdi, %%rax\n");

    if (node->kind == ND_EQ)
      printf("  sete %%al\n");
    else if (node->kind == ND_NE)
      printf("  setne %%al\n");
    else if (node->kind == ND_LT)
      printf("  setl %%al\n");
    else if (node->kind == ND_LE)
      printf("  setle %%al\n");

    printf("  movzb %%al, %%rax\n");
    return;
  default:
    break;
  }

  error_tok(node->tok, "invalid expression");
}

// 生成语句
static void gen_stmt(Node *node) {
  switch (node->kind) {
  // 生成if语句
  case ND_IF: {
    // 代码段计数
    int c = count();
    printf("\n# =====分支语句%d=====\n", c);
    // 生成条件内语句
    printf("\n# count表达式%d\n", c);
    gen_expr(node->cond);
    // 判断结果是否为0, 为0则跳转到else分支
    printf("  # 若结果为0, 则跳转到分支%d的.L.else.%d段\n", c, c);
    printf("  cmp $0, %%rax\n");
    printf("  je .L.else.%d\n", c);
    // 生成符合条件后的语句
    gen_stmt(node->then);
    printf("  jmp .L.end.%d\n", c);
    printf(".L.else.%d:\n", c);
    if (node->els)
      gen_stmt(node->els);
    printf(".L.end.%d:\n", c);
    return;
  }
  case ND_FOR: {
    int c = count();
    if (node->init)
      gen_stmt(node->init);
    printf(".L.begin.%d:\n", c);
    if (node->cond) {
      gen_expr(node->cond);
      printf("  cmp $0, %%rax\n");
      printf("  je .L.end.%d\n", c);
    }
    gen_stmt(node->then);
    if (node->inc)
      gen_expr(node->inc);
    printf("  jmp .L.begin.%d\n", c);
    printf(".L.end.%d:\n", c);
    return;
  }
  case ND_BLOCK:
    for (Node *n = node->body; n; n = n->next)
      gen_stmt(n);
    return;
  case ND_RETURN:
    printf("# 返回语句\n");
    gen_expr(node->lhs);
    // 无条件跳转语句, 跳转到.L.return段
    printf("  # 跳转到.L.return段\n");
    printf("  jmp .L.return.%s\n", current_fn->name);
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    return;
  default:
    break;
  }

  error_tok(node->tok, "invalid statement");
}

// 计算本地变量的偏移
// assign offsets to local variables
static void assign_lvar_offsets(Function *prog) {
  for (Function *fn = prog; fn ; fn = fn->next) {
    int offset = 0;
    for (Obj *var = fn->locals; var; var = var->next) {
      offset += 8;
      var->offset = -offset;
    }

    fn->stack_size = align_to(offset, 16);
  }
}

void codegen(Function *prog) {
  assign_lvar_offsets(prog);

  for (Function *fn = prog; fn; fn = fn->next) {
    printf("  .global %s\n", fn->name);
    printf("%s:\n", fn->name);
    current_fn = fn;

    printf("  push %%rbp\n");
    printf("  mov %%rsp, %%rbp\n");
    printf("  sub $%d, %%rsp\n", fn->stack_size);

    // 将函数参数添加到栈中
    int i = 0;
    for (Obj *var = fn->params; var; var = var->next)
      printf("  mov %s, %d(%%rbp)\n", argreg[i++], var->offset);

    gen_stmt(fn->body);
    assert(depth == 0);

    printf(".L.return.%s:\n", fn->name);
    printf("  mov %%rbp, %%rsp\n");
    printf("  pop %%rbp\n");
    printf("  ret\n");
  }
}

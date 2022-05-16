# cmake最低版本号
cmake_minimum_required( VERSION 3.10.0 )

# 项目名称
project( miniC C )

# 可执行文件rvcc的依赖文件
add_executable(miniC
  main.c
#   preprocess.c
#   string.c
  tokenize.c
  parse.c
  type.c
  codegen.c
#   unicode.c
#   hashmap.c
)

# 编译参数
target_compile_options(miniC PRIVATE -std=c11 -g -fno-common)
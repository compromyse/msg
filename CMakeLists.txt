cmake_minimum_required(VERSION 3.21)
project(msg C)

set(SRC
  src/copy.c
  src/engine.c
  src/filehandler.c
  src/lexer.c
  src/main.c
  src/template.c
)

add_executable(msg ${SRC})
target_include_directories(msg PRIVATE include)
target_link_libraries(msg markdown)

set(C_COMPILE_OPTIONS
  # -O3
  -O0

  -Wall
  -Wextra
  # -Werror

  -g3
  -glldb

  -std=c99

  -Wno-unused-result
  -Wno-unused-variable
)

target_compile_options(msg PRIVATE
  $<$<COMPILE_LANGUAGE:C>: ${C_COMPILE_OPTIONS}>
)

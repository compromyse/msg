cmake_minimum_required(VERSION 3.21)
add_subdirectory(discount/cmake)

project(msg C)

set(SRC
  src/config.c
  src/copy.c
  src/engine/contentfor.c
  src/engine/eachdo.c
  src/engine/engine.c
  src/engine/include.c
  src/filehandler.c
  src/lexer.c
  src/list.c
  src/main.c
  src/msg.c
  src/template.c
  src/util.c
)

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

add_executable(msg ${SRC})
target_include_directories(msg PRIVATE include)

target_include_directories(msg PRIVATE discount)
target_link_libraries(msg libmarkdown)

# The compiler isn't very happy with discount
target_compile_options(libmarkdown PRIVATE -w)
target_compile_options(mkd2html PRIVATE -w)
target_compile_options(makepage PRIVATE -w)
target_compile_options(markdown PRIVATE -w)
target_compile_options(common PRIVATE -w)

target_compile_options(msg PRIVATE
  $<$<COMPILE_LANGUAGE:C>: ${C_COMPILE_OPTIONS}>
)

install(TARGETS msg)

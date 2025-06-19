cmake_minimum_required(VERSION 3.21)
project(msg C)

set(SRC
  src/filehandler.c
  src/main.c
)

add_executable(msg ${SRC})
target_include_directories(msg PRIVATE include)
target_link_libraries(msg markdown)

set(C_COMPILE_OPTIONS
  # -O3
  -Og

  -Wall
  -Wextra
  # -Werror

  -g

  -std=c99

  -Wno-unused-result
)

target_compile_options(msg PRIVATE
  $<$<COMPILE_LANGUAGE:C>: ${C_COMPILE_OPTIONS}>
)

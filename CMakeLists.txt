cmake_minimum_required(VERSION 3.0)
project(Oberon0 LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(oberon0c
src/main.c
src/utils.h
src/utils.c
src/scanner.h
src/scanner.c
src/objects.h
src/objects.c
src/types.h
src/types.c
src/parser.h
src/parser.c
src/generator.h
src/generator.c
src/abstract_machine.h
src/abstract_machine.c
)

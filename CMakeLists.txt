cmake_minimum_required(VERSION 1.0)
project(Oberon0 LANGUAGES C)

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

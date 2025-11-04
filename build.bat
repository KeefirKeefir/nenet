@echo off

py ./make_unity.py
gcc ./unity.c -o ./out/nenet -DDEBUG -std=c23 -Wall -Wextra -g -O3 -L./lib -lraylib -lopengl32 -lgdi32 -lwinmm
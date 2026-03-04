#!/bin/sh


CC=gcc

SOURCE_FILES="dynamic_array.c lexer.c parser.c codegen.c ../isa/isa_table.c tasm.c"
CFLAGS="-Wall -Wextra -g"
OUTPUT_EXEC="tasm"

$CC $SOURCE_FILES -o $OUTPUT_EXEC $CFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful.Executable: $OUTPUT_EXEC"
else
    echo "Build Failed."
fi

#!/bin/sh


CC=gcc

SOURCE_FILES="tvm.c ./isa/isa_table.c file_handler.c"
CFLAGS="-Wall -Wextra -g"
OUTPUT_EXEC="tvm"

$CC $SOURCE_FILES -o $OUTPUT_EXEC $CFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful.Executable: $OUTPUT_EXEC"
else
    echo "Build Failed."
fi

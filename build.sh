#!/bin/sh


CC=gcc

SOURCE_FILES="main.c tvm.c instructions.c"
CFLAGS="-Wall -Wextra -g"
OUTPUT_EXEC="main"

$CC $SOURCE_FILES -o $OUTPUT_EXEC $CFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful.Executable: $OUTPUT_EXEC"
else
    echo "Build Failed."
fi

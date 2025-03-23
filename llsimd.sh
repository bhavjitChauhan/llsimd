#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <input>"
    exit 1
fi

if [ -z "$2" ]; then
	set -- "$1" "libllsimd.so"
fi

if [ ! -f "$2" ]; then
	echo "Error: $2 not found"
	exit 1
fi

basename=$(basename "$1" .c)

set -x

clang --target=x86_64-unknown-linux-gnu -S -emit-llvm "$1" -o "$basename.ll"
opt -load-pass-plugin "./$2" -passes=llsimd -S "$basename.ll" -o "$basename.out.ll"
llvm-as "$basename.out.ll" -o "$basename.out.bc"
clang "$basename.out.bc" -o "$basename" &>/dev/null

gcc decompressor/a.c -o a
gcc decompressor/b.c -o b -lm
gcc decompressor/c.c -o c
./c $1 | ./b d | ./a
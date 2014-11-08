gcc shortnumber.c -o shortnumber.out
tr -d '.' < curve1.dat | ./shortnumber.out c compressed.out
./shortnumber.out d compressed.out > decompressed.out
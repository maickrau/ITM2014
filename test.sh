gcc shortnumber.c -o shortnumber.out
gcc curve.c -o curve.out
gcc addcomma.c -o addcomma.out

#tr -d '.' < curve1.dat | ./shortnumber.out c compressed.out
#or
tr -d '.' < curve1.dat | ./curve.out c | ./shortnumber.out c compressed.out

gcc curve_mini.c -o curve_mini.out
gcc addcomma_mini.c -o addcomma_mini.out
gcc shortnumber_mini.c -o shortnumber_mini.out

#./shortnumber.out d compressed.out | ./addcomma.out > decompressed.out
#or
./shortnumber_mini.out compressed.out | ./curve_mini.out d | ./addcomma_mini.out > decompressed.out
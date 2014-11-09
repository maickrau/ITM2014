gcc shortnumber.c -o shortnumber.out
gcc curve.c -o curve.out
gcc addcomma.c -o addcomma.out

#tr -d '.' < curve1.dat | ./shortnumber.out c compressed.out
#or
tr -d '.' < curve1.dat | ./curve.out c | ./shortnumber.out c compressed.out

#./shortnumber.out d compressed.out | ./addcomma.out > decompressed.out
#or
./shortnumber.out d compressed.out | ./curve.out d | ./addcomma.out > decompressed.out
gcc smallnumber.c -o smallnumber.out
gcc smallnumber_mini.c -o smallnumber_mini.out
gcc curve.c -o curve.out -lm
gcc curve_mini.c -o curve_mini.out -lm
./curve.out c < curve1.dat | ./smallnumber.out c compressed.out
./smallnumber_mini.out compressed.out | ./curve_mini.out > decompressed.out
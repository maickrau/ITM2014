gzip -d smallnumber.c.gz
gcc smallnumber_mini.c -o smallnumber_mini.out
./smallnumber.out d compressed.dat > curve1.dat

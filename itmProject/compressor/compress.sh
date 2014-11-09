gcc compressor/curve.c -o compressor/curve.out
gcc compressor/shortnumber.c -o compressor/shortnumber.out
tr -d '.' < $1 | ./compressor/curve.out c | ./compressor/shortnumber.out c compressor/temp.temp
cat compressor/temp.temp
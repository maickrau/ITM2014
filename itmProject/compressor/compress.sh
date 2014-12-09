#!/bin/bash

gcc compressor/numberdiff.c -o numberdiff.out
g++ compressor/paleocompresser.cpp -std=c++0x -o paleocompresser.out
g++-4.7 compressor/finalcompressor.cpp -std=c++0x -o finalcompressor.cpp

if [ "$1" == "data/curve1.dat" ]
then
	tr -d '.' < $1 | ./numberdiff.out c temp.temp
	echo -n 1 > temp2.temp
	cat temp.temp >> temp2.temp
	cat temp2.temp
fi

if [ "$1" == "data/ty.txt" ]
then
	sed 's/^\(-*[0-9]*\.[0-9]\)$/\10/g' < $1 | sed 's/^\(-*[0-9]*\)$/\1.00/g' | sed 's/\.//g' > ty_fixed.temp
	./numberdiff.out c temp.temp < ty_fixed.temp
	echo -n 5 > temp2.temp
	cat temp.temp >> temp2.temp
	cat temp2.temp
fi

if [ "$1" == "data/paleo.csv" ]
then
	./paleocompresser.out c $1 temp.temp
	echo -n 2 > temp2.temp
	cat temp.temp >> temp2.temp
	cat temp2.temp
fi

if [ "$1" == "data/group.stock.dat" ]
then
	echo -n 3 > temp2.temp
	cat compressor/stock.c >> temp2.temp
	cat temp2.temp
fi

if [ "$1" == "data/bucket.1.dat" ] || [ "$1" == "data/bucket.2.dat" ] || [ "$1" == "data/bucket.3.dat" ] || [ "$1" == "data/monty_python_data_1.dat" ] || [ "$1" == "data/monty_python_data_2.dat" ]
then
	echo -n 4 > temp2.temp
	cat $1 >> temp2.temp
	cat temp2.temp
fi

if [ "$1" == "sdata/caravan.sdat" ]
then
	bzip2 --best < $2 > temp.temp
	echo -n 6 > temp2.temp
	cat temp.temp >> temp2.temp
	cat temp2.temp
fi

if [ "$1" == "sdata/final.sdat" ]
then
	./finalcompressor.out $2 $1 temp.temp c
	echo -n 8 > temp2.temp
	cat temp.temp >> temp2.temp
	cat temp2.temp
fi

if [ "$1" == "data/group.boring_bytes.dat" ]
then
	echo -n 7 > temp2.temp
	cat temp2.temp
fi

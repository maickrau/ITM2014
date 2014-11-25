#!/bin/bash
gcc decompressor/numberdiff.c -o numberdiff.out
g++-4.7 decompressor/paleocompresser.cpp -std=c++11 -o paleocompresser.out
a=$(head -c 1 < $1)
dd ibs=1 skip=1 < $1 > fixed.temp
if [ "$a" == "1" ]
then
	./numberdiff.out d fixed.temp y > temp.temp
	cat temp.temp
fi
if [ "$a" == "5" ]
then
	./numberdiff.out d fixed.temp n > temp.temp
	cat temp.temp
fi
if [ "$a" == "2" ]
then
	./paleocompresser.out d fixed.temp temp.temp
	cat temp.temp
fi
if [ "$a" == "3" ]
then
	cp fixed.temp stock.c
	gcc stock.c -o stock.out
	./stock.out
fi
if [ "$a" == "4" ]
then
	cat fixed.temp
fi

gcc decompressor/numberdiff.c -o numberdiff.out
g++ decompressor/paleocompresser.cpp -std=c++0x -o paleocompresser.out

a=$(head -c 1 < $1)

dd ibs=1 skip=1 < $1 > fixed.temp

if [ "$a" == "1" ] 
then
	./numberdiff.out d fixed.temp > temp.temp
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

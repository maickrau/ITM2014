# dd ibs=1 skip=1 < curve1.dat > curve1_firstbyte.dat
# head -c 1 < curve1.dat

gcc compressor/numberdiff.c -o numberdiff.out
g++ compressor/paleocompresser.cpp -std=c++0x -o paleocompresser.out

if [ "$1" == "data/curve1.dat" ] 
then
	tr -d '.' < $1 | ./numberdiff.out c temp.temp
	echo -n 1 > temp2.temp
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

if [ "$1" == "data/bucket.1.dat" ] 
then
	echo -n 4 > temp2.temp
	cat $1 >> temp2.temp
	cat temp2.temp
fi
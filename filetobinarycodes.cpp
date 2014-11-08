#include <iostream>
#include <fstream>
#include <bitset>

int main (){

    std::bitset<24> codes; //Here put the file sizes in bits. This will the data for decompression function.

    std::ifstream input("codesfordecompression.bin",std::ios::binary);

    int j=0;
    for(int i=0;i<3;i++){ //i is smaller than the number of bytes of the file
	std::bitset<8> abyte;
	unsigned long n;
	input.seekg(i,input.beg); //seek the ith byte
	input.read(reinterpret_cast<char*>(&n), 1) ; //read the ith byte
	abyte = n;
	for(int t=0; t<8; t++){

		codes[j]=abyte[t]; 
		j++;
	}	

    } 
    for(int i=0;i<24;i++){std::cout << codes[i];} //test

}

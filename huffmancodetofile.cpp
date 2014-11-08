#include <iostream>
#include <fstream>      
#include <bitset>              

void huffmancodetobinaryfile(int huffmancodes[], int length){


  const int l = 24; //Unfortunately, we need to hardcode the length (how many bits) of bitstream here. l has to be constant. It also has to be n*8 where 1<=n and n is an integer. For example, if we got huffman codes of 20 bits, here we should put 24. If 999bits then here is 1000.


  std::bitset<l> codesinbits;

  for(int i = 0; i < length; i++){ //read through all integers in huffmancodes array

        if( huffmancodes[i]==1 ){ //if position i is 1

		codesinbits.set(i); //set the binary bit on position i to be 1

	}
        std::cout  << codesinbits[i]; //test print
   }
   
    std::cout << "\n";

  std::ofstream output("huffmancode.bin",std::ios::binary); // Actually, what is this file are huffman codes with some extra zeros. Because the original huffman code migth not be integer times of 8. Hence it is very important that we know the original code length when decoding using this file. Or else, we will decode a few more zeros.

  std::bitset<8> packasbyte; //Or we can say that it will be pack as an unsigned char
  int j = 0;
  while( j < l){
 
 	for(int i = 0; i < 8 ; i++){ //packasbyte is a 8 bits bitset that can be cast to char(a byte)

		packasbyte[i] = codesinbits[j];
                j++;
		if (i == 8){i = 0;}
		std::cout  << packasbyte[i]; //test print

        }
	unsigned long i = packasbyte.to_ulong();  //Once done, it has to become unsigned long firstly
	unsigned char c = static_cast<unsigned char>( i );//cast unsigned long to a char
        output.write (reinterpret_cast<char*>(&c), 1); //put the char to the file. Each time this is done, the file gets a byte longer.
        std::cout << "\n";
  }
  output.close(); //close the file.
  
}


int main ()
{

  int huffmancodes[] = {0,1,0,0,0,1,1,0,1,1,1,0,1,0,0,1,1}; //just for test. Replace it by real huffman codes for practical use.

  const int length = sizeof(huffmancodes)/sizeof(int);

  std::cout << "Huffman code length: " << length << "\n";

  //std::cout << huffmancodes[1];
  huffmancodetobinaryfile(huffmancodes, length); //This function is what is important. It takes an integer array of huffmancodes as a parameter, and the code length as another parameter. Besides, we have to code the LENGTH OF BITSTREAM SEPAPATELY.
  return 0;

}

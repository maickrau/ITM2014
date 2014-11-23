/**
The test program for decoding numbers in numbers.txt
Not so "adaptable".

For decoding letters and space in alphabet.txt, it might be an good idea to use language that already have some Huffman functions. 
For example, Matlab has it, if the relative package was installed.
In this way, the decoding program can be very small.
All that needed is a c++ program to read bits in Huffman code file and write it in a temp file that can be read by Matlab function. This temp file will be deleted after data is loaded in Matlab program.
The rest decoding work will be done by Matlab program. 


**/
#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>

int main (){

    std::bitset<243311> code; //Here put the original code length

    std::ifstream input("aBITisaBit.bin",std::ios::binary); //File that contains Huffman codes

    int j=0;
    for(int i=0;i<30414;i++){ //i increament to the size of file in bytes.
	std::bitset<8> abyte;
	unsigned long n;
	input.seekg(i,input.beg); //seek the ith byte
	input.read(reinterpret_cast<char*>(&n), 1) ; //read the ith byte
	abyte = n;
	for(int t=0; t<8; t++){

		code[j]=abyte[t]; 
		j++;
	}	

    } 

    std::vector<char> str; //Decoded data is here before saving in a file

    for(int i=0;i<243311;){
        //The following are a hard-coded Huffman tree in the form of "if(){}else{}" Not so adaptable and easy way for it can only be used for the Huffman code dictionary. But this can be short if eliminate spaces.
	if(code[i]==0){
		i++;
		if(code[i]==0){
			i++;
			if(code[i]==0){
				i++;
				if(code[i]==0){str.push_back('5');i++;}
				else{str.push_back('2');i++;}
			}
			else{
				i++;
				if(code[i]==0){str.push_back('3');i++;}
				else{str.push_back('4');i++;}
			}
		}
		else{i++;
			if(code[i]==0){
				i++;
				if(code[i]==0){str.push_back('6');i++;}
				else{str.push_back('7');i++;}
			}
			else{str.push_back('1');i++;}

		}
	}
	else{i++;
		if(code[i]==0){
			i++; 
			if(code[i]==0){
				i++;
				if(code[i]==0){
					str.push_back('8');
					i++;}
				else{
					i++;
					if(code[i]==0){
						str.push_back('9');
						i++;}
					else{i++;}
				}
			}
			else{str.push_back('0');i++;}
		}
		else{str.push_back(',');i++;}
	}

   } //test

  std::ofstream fileout ("numbersrestored.txt"); //data
  if (fileout.is_open() )
  {
    char out;
    for(int i = 0; i<str.size(); ++i)
    {    
       out = str[i];
     fileout << out;  
    }
  fileout.close();
  }
  else std::cout << "File problem";

}

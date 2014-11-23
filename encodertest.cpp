/**

Compile with : g++ encodertest.cpp -o anyname.out

Run : 
To encode roma alphbets and space "name.out alphbets.txt"
To encode numbers and comma "name.out numbers.txt"
Other symbols are not supported unless manually coded in program

A not so automatic program to encode symbols because of the above reasons.

The symbol and its corresponding code should be manually set.

Same for the encoding command.

The encoded codes in binary digits are saved in integer vector huffmancodes. By the way, it just a name. Actually this program will encode by whatever kind of symbols to codes mapping setting.

binarycodetobinaryfile() function is for saving codes in a file, for testing. 


**/
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset> 

using namespace std;

void binarycodetobinaryfile(vector<int>& binarycodes,int length){


  std::ofstream output("aBITisaBit.bin",std::ios::binary); // Actually, what's in this file is codes with some extra zeros. Because the original length migth not be integer times of 8. 

  std::bitset<8> packasbyte; //Or we can say that it will be pack as an unsigned char
  int j = 0;
  while( j < length){
 
 	for(int i = 0; i < 8 ; i++){ //packasbyte is a 8 bits bitset that can be cast to char(a byte)

		packasbyte[i] = binarycodes[j];
                j++;
		if (i == 8){i = 0;}

        }
	unsigned long i = packasbyte.to_ulong();  //Once done, it has to become unsigned long firstly
	unsigned char c = static_cast<unsigned char>( i );//cast unsigned long to a char
        output.write (reinterpret_cast<char*>(&c), 1); //put the char to the file. Each time this is done, the file gets a byte longer.
  }
  output.close(); //close the file.
  
}



int main(int argc, char** argv){

  std::vector<int> huffmancodes; //This vector contains codes for the input file
  std::vector<int>::iterator it;
  it = huffmancodes.begin();

  //Huffmancodes are manually programed below. New entry has to be add inorder to process extra symbols
  int acode[]={1,0,0};
  int bcode[]={1,1,0,1,1,0,0,0};
  int ccode[]={1,1,0,1,1,0,0,1,0,0};
  int dcode[]={1,1,0,1,0,0,0};
  int ecode[]={0,1,0,0};
  int fcode[]={1,1,0,1,1,0,1};
  int gcode[]={1,1,0,1,0,0,1};
  int hcode[]={0,0,0,0,1,1};
  int icode[]={0,1,1};
  int jcode[]={1,1,0,1,1,1};
  int kcode[]={1,0,1,0};
  int lcode[]={1,1,0,0};
  int mcode[]={1,0,1,1,0};
  int ncode[]={0,0,1,0};
  int ocode[]={0,1,0,1};
  int pcode[]={1,0,1,1,1};
  int qcode[]={1,1,0,1,1,0,0,1,0,1,0,1};
  int rcode[]={0,0,0,0,0};
  int scode[]={0,0,0,1};
  int tcode[]={1,1,1};
  int ucode[]={0,0,1,1};
  int vcode[]={0,0,0,0,1,0};
  int wcode[]={1,1,0,1,1,0,0,1,0,1,0,0};
  int xcode[]={1,1,0,1,1,0,0,1,0,1,1,0,0};
  int ycode[]={1,1,0,1,0,1};
  int zcode[]={1,1,0,1,1,0,0,1,0,1,1,1};
  int spacecode[]={1,1,0,1,1,0,0,1,1};

  int a0code[]={1,0,1};
  int a1code[]={0,1,1};
  int a2code[]={0,0,0,1};
  int a3code[]={0,0,1,0};
  int a4code[]={0,0,1,1};
  int a5code[]={0,0,0,0};
  int a6code[]={0,1,0,0};
  int a7code[]={0,1,0,1};
  int a8code[]={1,0,0,0};
  int a9code[]={1,0,0,1,0};
  int commacode[]={1,1};

  char ch;
  std::ifstream filein(argv[1]);
     while(filein.get(ch)) //This loop read each character then put the corresponding huffman code to the vector
     {
	if (ch=='a')
		  //This and those "for" loops do the same thing.
		  huffmancodes.insert(it, acode, acode+sizeof(acode)/sizeof(int));
	else if (ch=='b')
		  {for(int i=0;i<sizeof(bcode)/sizeof(int);i++) {huffmancodes.push_back(bcode[i]);}}
	else if (ch=='c')
		  huffmancodes.insert(it, ccode, ccode+sizeof(ccode)/sizeof(int));
	else if (ch=='d')
		  {for(int i=0;i<sizeof(dcode)/sizeof(int);i++) {huffmancodes.push_back(dcode[i]);}}
	else if (ch=='e')
		  huffmancodes.insert(it, ecode, ecode+sizeof(ecode)/sizeof(int));
	else if (ch=='f')
		  {for(int i=0;i<sizeof(fcode)/sizeof(int);i++) {huffmancodes.push_back(fcode[i]);}}
	else if (ch=='g')
		  huffmancodes.insert(it, gcode, gcode+sizeof(gcode)/sizeof(int));
	else if (ch=='h')
		  {for(int i=0;i<sizeof(hcode)/sizeof(int);i++) {huffmancodes.push_back(hcode[i]);}}
	else if (ch=='i')
		  huffmancodes.insert(it, icode, icode+sizeof(icode)/sizeof(int));
	else if (ch=='j')
		  {for(int i=0;i<sizeof(jcode)/sizeof(int);i++) {huffmancodes.push_back(jcode[i]);}}
        else if (ch=='k')
		  huffmancodes.insert(it, kcode, kcode+sizeof(kcode)/sizeof(int));
        else if (ch=='l')
		  {for(int i=0;i<sizeof(lcode)/sizeof(int);i++) {huffmancodes.push_back(lcode[i]);}}
        else if (ch=='m')
		  huffmancodes.insert(it, mcode, mcode+sizeof(mcode)/sizeof(int));
        else if (ch=='n')
		  {for(int i=0;i<sizeof(ncode)/sizeof(int);i++) {huffmancodes.push_back(ncode[i]);}}
        else if (ch=='o')
		  huffmancodes.insert(it, ocode, ocode+sizeof(ocode)/sizeof(int));
        else if (ch=='p')
		  huffmancodes.insert(it, pcode, pcode+sizeof(pcode)/sizeof(int));
        else if (ch=='q')
		  {for(int i=0;i<sizeof(qcode)/sizeof(int);i++) {huffmancodes.push_back(qcode[i]);}}
        else if (ch=='r')
		  huffmancodes.insert(it, rcode, rcode+sizeof(rcode)/sizeof(int));
        else if (ch=='s')
		  {for(int i=0;i<sizeof(scode)/sizeof(int);i++) {huffmancodes.push_back(scode[i]);}}
        else if (ch=='t')
		  huffmancodes.insert(it, tcode, tcode+sizeof(tcode)/sizeof(int));
        else if (ch=='u')
		  {for(int i=0;i<sizeof(ucode)/sizeof(int);i++) {huffmancodes.push_back(ucode[i]);}}
        else if (ch=='v')
		  huffmancodes.insert(it, vcode, vcode+sizeof(vcode)/sizeof(int));
        else if (ch=='w')
		  {for(int i=0;i<sizeof(wcode)/sizeof(int);i++) {huffmancodes.push_back(wcode[i]);}}
        else if (ch=='x')
		  huffmancodes.insert(it, xcode, xcode+sizeof(xcode)/sizeof(int));
        else if (ch=='y')
		  {for(int i=0;i<sizeof(ycode)/sizeof(int);i++) {huffmancodes.push_back(ycode[i]);}}
        else if (ch=='z')
		  huffmancodes.insert(it, zcode, zcode+sizeof(zcode)/sizeof(int));
        else if(ch==' ')
		  huffmancodes.insert(it, spacecode, spacecode+sizeof(spacecode)/sizeof(int));

	//These are for numbers and comma
        else if (ch=='0')
		  huffmancodes.insert(it, a0code, a0code+sizeof(a0code)/sizeof(int));
        else if (ch=='1')
		  huffmancodes.insert(it, a1code, a1code+sizeof(a1code)/sizeof(int));
        else if (ch=='2')
		  huffmancodes.insert(it, a2code, a2code+sizeof(a2code)/sizeof(int));
        else if (ch=='3')
		  huffmancodes.insert(it, a3code, a3code+sizeof(a3code)/sizeof(int));
        else if (ch=='4')
		  huffmancodes.insert(it, a4code, a4code+sizeof(a4code)/sizeof(int));
        else if (ch=='5')
		  huffmancodes.insert(it, a5code, a5code+sizeof(a5code)/sizeof(int));
        else if (ch=='6')
		  huffmancodes.insert(it, a6code, a6code+sizeof(a6code)/sizeof(int));
        else if (ch=='7')
		  huffmancodes.insert(it, a7code, a7code+sizeof(a7code)/sizeof(int));
        else if (ch=='8')
		  huffmancodes.insert(it, a8code, a8code+sizeof(a8code)/sizeof(int));
        else if (ch=='9')
		  huffmancodes.insert(it, a9code, a9code+sizeof(a9code)/sizeof(int));
        else if (ch==',')
		  huffmancodes.insert(it, commacode, commacode+sizeof(commacode)/sizeof(int));


	else 
		  std::cout << "Can not find the symbol.";

	it = huffmancodes.end(); //Renew the iterator

   }
  filein.close(); //Close the file

  const int length = huffmancodes.size();

  std::cout << "Code length: " << length << "bits" << "\n";

  binarycodetobinaryfile(huffmancodes, length); 

/**
  int out;
  std::ofstream fileout ("textformathuffmancodes.txt");
  if (fileout.is_open() )
  {
    for(it = huffmancodes.begin(); it<huffmancodes.end(); it++)
    {
     fileout << *it ;
    }
  fileout.close();
  }
  else cout << "File problem";
**/


  return 0;


}

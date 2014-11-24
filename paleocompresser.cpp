/***

Compresses the paleo data. Lots of stuff purposebuilt just for this specific data.

Compile: g++ paleocompresser.cpp -std=c++11 -o paleocompressor.out
or -std=c++0x in melkinkari

Compress: ./paleocompressor.out c paleo.csv compressed.out
Decompress: ./paleocompressor.out d compressed.out decompressed.out
Compress with diagnostics: ./paleocompressor.out c paleo.csv compressed.out d
Compress and print alphabet/numberstrings: ./paleocompressor.out c paleo.csv compressed.out alphabets.txt numbers.txt

Data is compressed per row. Each column has two ways of decompressing it. Flags tell which one is used.

First column: flag tells whether the genus is the same as the above row. Compresses either genus and species or just species as alphabetstrings.
Second column: flag tells whether it's empty. Otherwise encoded as 12-bit number
3. column: flag tells whether it's 4-bit or 8-bit integer
4.-7. columns: are compressed as an integer and a comma
8. column: flag tells whether it's "0"
9. column: flag tells whether it's 4-bit or 6-bit integer
10. column: flag tells whether it's the same as the third column
11.-13. columns: flag tells whether it's two columns divided, #DIV/0! is a valid division. Otherwise encoded as a numberstring
14. column: flag tells whether it's (dm/(dm-ah))^2. Otherwise numberstring.

A numberstring is [0-9,]*. Length is encoded as 3 or 4 bits and each number/comma is encoded as 3 or 4 bits.

An alphabetstring is [a-z ]*. Length is encoded as 5 bits (note: max length is 39 because of holes), each alphabet is encoded as 5 bits.

***/

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>

class BitWriter
{
public:
	BitWriter(FILE* s) : totalWritten(0), stream(s), bitLoc(0), nextBits(0) {};
	~BitWriter()
	{
		flushWrite();
	};
	void writeBits(char* target, int bits);
	size_t written()
	{
		return totalWritten;
	};
private:
	size_t totalWritten;
	void flushWrite();
	FILE* stream;
	int bitLoc;
	char nextBits;
};

class BitReader
{
public:
	BitReader(FILE* s) : stream(s), bitLoc(0), nextBits(0) {};
	void readBits(char* target, int bits);
	bool eof()
	{
		return feof(stream);
	};
private:
	FILE* stream;
	int bitLoc;
	int nextBits;
};

void BitReader::readBits(char* target, int bits)
{
	int written = 0;
	while (written < bits)
	{
		if (bitLoc == 0)
		{
			nextBits = fgetc(stream);
		}
		if (written % 8 == 0)
		{
			*target = 0;
		}
		*target |= (nextBits&(1<<bitLoc) ? 1 : 0) << written%8;
		bitLoc++;
		bitLoc %= 8;
		written++;
		if (written % 8 == 0)
		{
			target++;
		}
	}
}

void BitWriter::writeBits(char* target, int bits)
{
	totalWritten += bits;
	int read = 0;
	while (read < bits)
	{
		if (bitLoc == 0)
		{
			nextBits = 0;
		}
		nextBits |= ((*target)&(1<<(read%8)) ? 1 : 0) << bitLoc;
		bitLoc++;
		bitLoc %= 8;
		if (bitLoc == 0)
		{
			fputc(nextBits, stream);
		}
		read++;
		if (read % 8 == 0)
		{
			target++;
		}
	}
}

std::string strReplace(std::string in, char a, char b)
{
	for (int i = 0; i < in.size(); i++)
	{
		if (in[i] == a)
		{
			in[i] = b;
		}
	}
	return in;
}

void BitWriter::flushWrite()
{
	if (bitLoc != 0)
	{
		fputc(nextBits, stream);
	}
}

class PaleoCompressor
{
public:
	PaleoCompressor(BitWriter& write, FILE* read) : write(write), read(read), lastGenus() 
	{
		memset(compressedBitsPart, 0, 14*sizeof(size_t));
		memset(alphabetCount, 0, 40*sizeof(int));
		memset(numberCount, 0, 12*sizeof(int));
		memset(hasNumber, 0, 11*sizeof(bool));
		memset(hasAlphabet, 0, 39*sizeof(bool));
		compressedBitsAlphabet = 0;
		compressedBitsAlphabetLengths = 0;
		compressedBitsNumberLengths = 0;
		compressedBitsNumber = 0;
		compressedBitsByteable = 0;
		compressedBitsFlags = 0;
		compressedBitsShortable = 0;
		compressedBitsLobes = 0;
		compressedBitsMaxDm = 0;
		maxNumberLen = 0;
		maxAlphabetLen = 0;
	};
	void compress()
	{
		wasteLine();
		int lineNum = 0;
		for (int i = 0; i < 4128; i++)
		{
			compressLine(lineNum);
			lineNum++;
		}
	};
	void printDiagnostics()
	{
		std::cout << "parts:\n";
		int sum = 0;
		for (int i = 0; i < 14; i++)
		{
			std::cout << compressedBitsPart[i]/8 << "\n";
			sum += compressedBitsPart[i];
		}
		std::cout << "sum: " << sum/8 << "\n";
		std::cout << "writer: " << write.written()/8 << "\n";
		std::cout << "flags:\n";
		std::cout << compressedBitsFlags/8 << "\n";
		std::cout << "lobes:\n";
		std::cout << compressedBitsLobes/8 << "\n";
		std::cout << "maxDm:\n";
		std::cout << compressedBitsMaxDm/8 << "\n";
		std::cout << "byteable:\n";
		std::cout << compressedBitsByteable/8 << "\n";
		std::cout << "shortable:\n";
		std::cout << compressedBitsShortable/8 << "\n";
		std::cout << "alphabet:\n";
		std::cout << compressedBitsAlphabet/8 << "\n";
		std::cout << "alphabet lengths:\n";
		std::cout << compressedBitsAlphabetLengths/8 << "\n";
		std::cout << "max alphabet: " << maxAlphabetLen << "\n";
		for (int i = 0 ; i < 40; i++)
		{
			if (hasAlphabet[i])
			{
				std::cout << i << ": " << alphabetCount[i] << "\n";
			}
		}
		std::cout << "\n";
		std::cout << "number:\n";
		std::cout << compressedBitsNumber/8 << "\n";
		std::cout << "number lengths:\n";
		std::cout << compressedBitsNumberLengths/8 << "\n";
		std::cout << "max number: " << maxNumberLen << "\n";
		for (int i = 0; i < 12; i++)
		{
			if (hasNumber[i])
			{
				std::cout << i << ": " << numberCount[i] << "\n";
			}
		}
		std::cout << "\n";
	};
	std::string alphabetsToBeCompressed;
	std::string numbersToBeCompressed;
private:
	size_t compressedBitsPart[14];
	size_t compressedBitsAlphabet;
	size_t compressedBitsNumber;
	size_t compressedBitsByteable;
	size_t compressedBitsFlags;
	size_t compressedBitsShortable;
	size_t compressedBitsAlphabetLengths;
	size_t compressedBitsNumberLengths;
	size_t compressedBitsLobes;
	size_t compressedBitsMaxDm;
	int currentPartCounter;
	int maxNumberLen;
	int maxAlphabetLen;
	bool hasNumber[12];
	bool hasAlphabet[40];
	int numberCount[12];
	int alphabetCount[40];
	void wasteLine()
	{
		char a = fgetc(read);
		while (a != '\n')
		{
			a = fgetc(read);
		}
	};
	void compressLine(int lineNum)
	{
		if (lineNum == 3880)
		{
			wasteLine();
			return;
		}
		std::string parts[14];
		int currentPart = 0;
		char a = fgetc(read);
		while (!feof(read) && a != '\n')
		{
			if (a == ';')
			{
				currentPart++;
				a = fgetc(read);
				continue;
			}
			parts[currentPart] += a;
			a = fgetc(read);
		}
		int flags = 0;
		flags |= hasNameBit(parts[0]);
		flags |= (parts[1] == "") << 1;
		flags |= hasMaxDmBit(parts[2])<<2;
		for (int i = 3; i < 7; i++)
		{
			flags |= isByteable(parts[i])<<(i);
		}
		flags |= (parts[7] == "0")<<7;
		flags |= hasLobeBit(parts[8])<<8;
		flags |= (parts[9] == parts[2])<<9;
		flags |= isFloatable(parts[4], parts[3], parts[10])<<10;
		flags |= isFloatable(parts[4], parts[5], parts[11])<<11;
		flags |= isFloatable(parts[6], parts[3], parts[12])<<12;
		flags |= isWhorlable(parts[3], parts[7], parts[13])<<13;

		flags = (flags&7) | (((flags>>7)&255)<<3);

		write.writeBits((char*)&flags, 10);

		compressedBitsFlags += 10;
		currentPartCounter = 0;
		compressName(parts[0], flags&1);
		currentPartCounter = 1;
		if (flags&2)
		{
		}
		else
		{
			compressShortable(parts[1]);
		}
		currentPartCounter = 2;
		compressMaxDm(parts[2], flags&4);
		for (int i = 3; i < 7; i++)
		{
			currentPartCounter = i;
			compressByteable(parts[i]);
		}
		currentPartCounter = 7;
		if (flags&(1<<3))
		{
		}
		else
		{
			compressNumber(parts[7]);
		}
		currentPartCounter = 8;
		compressLobe(parts[8], flags&(1<<4));
		currentPartCounter = 9;
		if (flags&(1<<5))
		{
		}
		else
		{
			compressNumber(parts[9]);
		}
		for (int i = 10; i < 13; i++)
		{
			currentPartCounter = i;
			if (flags&(1<<(i-4)))
			{
			}
			else
			{
				compressNumber(parts[i]);
			}
		}
		currentPartCounter = 13;
		if (flags&(1<<9))
		{
		}
		else
		{
			compressNumber(parts[13]);
		}
	};
	bool hasLobeBit(std::string str)
	{
		if (str == "")
		{
			return true;
		}
		int num = atoi(str.c_str());
		return num < 17;
	}
	void compressLobe(std::string str, bool lobeBit)
	{
		unsigned int num = 0;
		if (str == "")
		{
			compressedBitsPart[currentPartCounter] += 4;
			compressedBitsLobes += 4;
			write.writeBits((char*)&num, 4);
			return;
		}
		num = atoi(str.c_str());
		num--;
		if (lobeBit)
		{
			compressedBitsPart[currentPartCounter] += 4;
			compressedBitsLobes += 4;
			write.writeBits((char*)&num, 4);
		}
		else
		{
			compressedBitsPart[currentPartCounter] += 6;
			compressedBitsLobes += 6;
			write.writeBits((char*)&num, 6);
		}
	}
	bool hasMaxDmBit(std::string str)
	{
		int num = atoi(str.c_str());
		if (num < 19)
		{
			return true;
		}
		return false;
	}
	void compressMaxDm(std::string str, bool hasBit)
	{
		unsigned int num = atoi(str.c_str());
		if (num > 0)
		{
			num -= 3;
		}
		if (hasBit)
		{
			write.writeBits((char*)&num, 4);
			compressedBitsPart[currentPartCounter] += 4;
			compressedBitsMaxDm += 4;
		}
		else
		{
			if (num == 270) num = 223;
			if (num == 282) num = 224;
			if (num == 287) num = 225;
			if (num == 297) num = 226;
			if (num == 347) num = 227;
			if (num == 397) num = 228;
			if (num == 477) num = 229;
			if (num == 497) num = 230;
			if (num == 597) num = 231;
			write.writeBits((char*)&num, 8);
			compressedBitsPart[currentPartCounter] += 8;
			compressedBitsMaxDm += 8;
		}
	}
	void compressShortable(std::string str)
	{
		compressedBitsPart[currentPartCounter] += 13;
		compressedBitsShortable += 13;
		int writeThis = atoi(str.c_str());
		assert(writeThis >= 0 && writeThis < 8192);
		write.writeBits((char*)&writeThis, 13);
	}
	bool hasNameBit(std::string line)
	{
		std::string genus;
		std::string species;
		int loc = 0;
		while (line[loc] != ' ')
		{
			genus += line[loc];
			loc++;
		}
		return genus != lastGenus;
	};
	void compressAlphabetString(std::string str)
	{
		alphabetsToBeCompressed += str;
		char size = str.size();
		if (size == 39)
		{
			size = 34;
		}
		if (size >= 31) size--;
		if (size >= 30) size--;
		if (size >= 25) size -= 3;
		if (size >= 1) size--;
		compressedBitsPart[currentPartCounter] += 5+5*str.size();
		compressedBitsAlphabet += 5*str.size();
		compressedBitsAlphabetLengths += 5;
		write.writeBits(&size, 5);
		if (size > maxAlphabetLen)
		{
			maxAlphabetLen = size;
		}
		hasAlphabet[size] = true;
		alphabetCount[size]++;
		for (int i = 0; i < str.size(); i++)
		{
			unsigned char writeThis = str[i]-'a'+1;
			if (str[i] == ' ')
			{
				writeThis = 0;
			}
			assert(writeThis >= 0 && writeThis < 32);
			write.writeBits((char*)&writeThis, 5);
		}
	};
	void compressName(std::string line, bool compressGenus)
	{
		std::string genus;
		std::string species;
		int loc = 0;
		while (loc < line.size() && line[loc] != ' ')
		{
			genus += line[loc];
			loc++;
		}
		loc++;
		while (loc < line.size())
		{
			species += line[loc];
			loc++;
		}
		if (compressGenus)
		{
			compressAlphabetString(genus);
		}
		compressAlphabetString(species);
		lastGenus = genus;
	};
	bool isByteable(std::string line)
	{
		if (line.size() == 0)
		{
			return false;
		}
		int num = 0;
		int pos = 0;
		while (pos < line.size())
		{
			if (line[pos] == ',')
			{
				pos++;
				continue;
			}
			if (line[pos] < '0' || line[pos] > '9')
			{
				return false;
			}
			num *= 10;
			num += line[pos]-'0';
			pos++;
		}
		if (num >= 8192 && num != 8722)
		{
			return false;
		}
		return true;
	};
	bool isWhorlable(std::string line1, std::string line2, std::string line3)
	{
		if (!isNumerable(line1) || !isNumerable(line2) || !isNumerable(line3))
		{
			return false;
		}
		if (line1.size() < 1 || line2.size() < 1 || line3.size() < 1)
		{
			return false;
		}
		line1 = strReplace(line1, ',', '.');
		line2 = strReplace(line2, ',', '.');
		line3 = strReplace(line3, ',', '.');
		double a = atof(line1.c_str());
		double b = atof(line2.c_str());
		b = a-b;
		double c = a/b;
		c *= c;
		char cmp[20] {0};
		sprintf(cmp, "%.9f", c);
		return line3 == cmp;
	};
	bool isFloatable(std::string line1, std::string line2, std::string line3)
	{
		if (line2 == "0" && line3 == "#DIV/0!")
		{
			return true;
		}
		if (!isNumerable(line1) || !isNumerable(line2) || !isNumerable(line3))
		{
			return false;
		}
		if (line1.size() < 1 || line2.size() < 1 || line3.size() < 1)
		{
			return false;
		}
		line1 = strReplace(line1, ',', '.');
		line2 = strReplace(line2, ',', '.');
		line3 = strReplace(line3, ',', '.');
		double a = atof(line1.c_str());
		double b = atof(line2.c_str());
		double c = a/b;
		char cmp[20] {0};
		sprintf(cmp, "%.9f", c);
		return line3 == cmp;
	};
	bool isNumerable(std::string line)
	{
		int pos = 0;
		while (pos < line.size())
		{
			if ((line[pos] < '0' || line[pos] > '9') && line[pos] != ',')
			{
				return false;
			}
			pos++;
		}
		return true;
	};
	void compressNumber(std::string line)
	{
		numbersToBeCompressed += line;
		unsigned char size = line.size();
		if (size == 1)
		{
			size = 10;
		}
		else if (size == 10)
		{
			size = 1;
		}
		hasNumber[size] = true;
		numberCount[size]++;
		if (size > maxNumberLen)
		{
			maxNumberLen = size;
		}
		if (size < 4)
		{
			write.writeBits((char*)&size, 3);
			compressedBitsPart[currentPartCounter] += 3;
			compressedBitsNumberLengths += 3;
		}
		else
		{
			size |= 4;
			write.writeBits((char*)&size, 4);
			compressedBitsPart[currentPartCounter] += 4;
			compressedBitsNumberLengths += 4;
		}
		for (int i = 0; i < line.size(); i++)
		{
			char writeThis = line[i]-'0'+1;
			if (line[i] == ',')
			{
				writeThis = 0;
			}
			if (writeThis < 4)
			{
				write.writeBits(&writeThis, 3);
				compressedBitsPart[currentPartCounter] += 3;
				compressedBitsNumber += 3;
			}
			else
			{
				writeThis |= 4;
				write.writeBits(&writeThis, 4);
				compressedBitsPart[currentPartCounter] += 4;
				compressedBitsNumber += 4;
			}
		}
	};
	void compressByteable(std::string line)
	{
		if (line == "87,22")
		{
			unsigned int writeThis = 1;
			write.writeBits((char*)&writeThis, 2);
			writeThis = 3;
			write.writeBits((char*)&writeThis, 2);
			compressedBitsPart[currentPartCounter] += 4;
			compressedBitsByteable += 4;
			return;
		}
		unsigned int writeThis = 0;
		int test = 0;
		int pos = 0;
		unsigned int commaPos = line.size()-1;
		while (pos < line.size())
		{
			if (line[pos] == ',')
			{
				commaPos = pos;
				pos++;
				continue;
			}
			writeThis *= 10;
			writeThis += line[pos]-'0';
			pos++;
		}
		commaPos = line.size()-1-commaPos;
		assert(commaPos < 4);
		assert(writeThis < 8192);
		if (writeThis == 0)
		{
			write.writeBits((char*)&writeThis, 2);
			compressedBitsPart[currentPartCounter] += 2;
			compressedBitsByteable += 2;
			return;
		}
		int writeBits = 8;
		unsigned int flags = 0;
		if (writeThis < 128)
		{
			flags = 1;
			writeBits = 7;
		}
		else if (writeThis < 1024)
		{
			flags = 2;
			writeBits = 10;
		}
		else if (writeThis < 8192)
		{
			flags = 3;
			writeBits = 13;
		}
		write.writeBits((char*)&flags, 2);
		write.writeBits((char*)&commaPos, 2);
		write.writeBits((char*)&writeThis, writeBits);
		compressedBitsPart[currentPartCounter] += 4+writeBits;
		compressedBitsByteable += 4+writeBits;
	};
	BitWriter& write;
	FILE* read;
	std::string lastGenus;
};

class PaleoDecompressor
{
public:
	PaleoDecompressor(BitReader& in, std::ostream& out) : in(in), out(out), lastGenus() {};
	void decompress()
	{
		printFirstLine();
		for (int i = 0; i < 4128; i++)
		{
			decompressLine(i);
		}
	}
private:
	void printFirstLine()
	{
		out << ";;max dm;dm;ww;wh;uw;ah;lobes;max dm;ww/dm;ww/wh;uw/dm;WER\n";
	}
	void decompressPossibleWhorl(int flags, int pos, std::string dm, std::string ah)
	{
		if (flags&(1<<pos))
		{
			dewhorl(dm, ah);
		}
		else
		{
			out << decompressNumber();
		}
	}
	void decompressPossibleFloat(int flags, int pos, std::string up, std::string down)
	{
		if (flags&(1<<pos))
		{
			defloat(up, down);
		}
		else
		{
			out << decompressNumber();
		}
		out << ";";
	}
	void dewhorl(std::string dm, std::string ah)
	{
		dm = strReplace(dm, ',', '.');
		ah = strReplace(ah, ',', '.');
		double a = atof(dm.c_str());
		double b = atof(ah.c_str());
		b = a-b;
		double c = a/b;
		c *= c;
		char outStr[20] {0};
		sprintf(outStr, "%1.9f", c);
		std::string val = strReplace(std::string(outStr), '.', ',');
		out << val;
	}
	void defloat(std::string left, std::string right)
	{
		if (right == "0")
		{
			out << "#DIV/0!";
			return;
		}
		left = strReplace(left, ',', '.');
		right = strReplace(right, ',', '.');
		double a = atof(left.c_str());
		double b = atof(right.c_str());
		double c = a/b;
		char outStr[20] {0};
		sprintf(outStr, "%1.9f", c);
		std::string val = strReplace(std::string(outStr), '.', ',');
		out << val;
	}
	void decompressLine(int lineNum)
	{
		if (lineNum == 3880)
		{
			out << "kangastus tuominen;1453;;;;;;;;;;;;\n";
			return;
		}

		int flags = 0;
		in.readBits((char*)&flags, 10);

		if (flags&1)
		{
			lastGenus = decompressAlphabet();
		}
		std::string species = decompressAlphabet();
		out << lastGenus << " ";
		out << species << ";";
		if (flags&2)
		{
			out << ";";
		}
		else
		{
			decompressShortable();
		}
		std::string parts[10];
		parts[2] = decompressMaxDm(flags&4);
		out << parts[2] << ";";
		for (int i = 3; i < 7; i++)
		{
			parts[i] = decompressByte();
			out << parts[i];
			out << ";";
		}
		if (flags&(1<<3))
		{
			parts[7] = "0";
		}
		else
		{
			parts[7] = decompressNumber();
		}
		out << parts[7];
		out << ";";
		parts[8] = decompressLobe(flags&(1<<4));
		out << parts[8];
		out << ";";
		if (flags&(1<<5))
		{
			out << parts[2];
		}
		else
		{
			out << decompressNumber();
		}
		out << ";";
		decompressPossibleFloat(flags, 6, parts[4], parts[3]);
		decompressPossibleFloat(flags, 7, parts[4], parts[5]);
		decompressPossibleFloat(flags, 8, parts[6], parts[3]);
		decompressPossibleWhorl(flags, 9, parts[3], parts[7]);
		out << "\n";
	};
	std::string decompressLobe(bool hasBit)
	{
		unsigned int num = 0;
		if (hasBit)
		{
			in.readBits((char*)&num, 4);
		}
		else
		{
			in.readBits((char*)&num, 6);
		}
		if (num > 0)
		{
			num++;
		}
		if (num == 0)
		{
			return "";
		}
		std::stringstream stream;
		stream << num;
		std::string ret;
		stream >> ret;
		return ret;
	}
	std::string decompressMaxDm(bool hasBit)
	{
		unsigned int num = 0;
		if (hasBit)
		{
			in.readBits((char*)&num, 4);
		}
		else
		{
			in.readBits((char*)&num, 8);
			if (num == 223) num = 270;
			if (num == 224) num = 282;
			if (num == 225) num = 287;
			if (num == 226) num = 297;
			if (num == 227) num = 347;
			if (num == 228) num = 397;
			if (num == 229) num = 477;
			if (num == 230) num = 497;
			if (num == 231) num = 597;
		}
		if (num > 0)
		{
			num += 3;
		}
		std::stringstream stream;
		stream << num;
		std::string ret;
		stream >> ret;
		return ret;
	}
	void decompressShortable()
	{
		int readHere = 0;
		in.readBits((char*)&readHere, 13);
		out << readHere << ";";
	}
	std::string decompressNumber()
	{
		std::string ret;
		unsigned char size = 0;
		in.readBits((char*)&size, 3);
		if (size & 4)
		{
			unsigned char top = 0;
			in.readBits((char*)&top, 1);
			if (top)
			{
				size += 4;
			}
		}
		if (size == 1)
		{
			size = 10;
		}
		else if (size == 10)
		{
			size = 1;
		}
		for (int i = 0; i < size; i++)
		{
			unsigned char next = 0;
			in.readBits((char*)&next, 3);
			if (next & 4)
			{
				unsigned char top = 0;
				in.readBits((char*)&top, 1);
				if (top)
				{
					next += 4;
				}
			}
			if (next == 0)
			{
				ret += ',';
			}
			else
			{
				ret += '0'+next-1;
			}
		}
		return ret;
	};
	std::string decompressAlphabet()
	{
		std::string ret;
		unsigned char size = 0;
		in.readBits((char*)&size, 5);
		if (size >= 1) size++;
		if (size >= 25) size += 3;
		if (size >= 30) size++;
		if (size >= 31) size++;
		if (size == 34) size = 39;
		for (int i = 0; i < size; i++)
		{
			unsigned char next = 0;
			in.readBits((char*)&next, 5);
			if (next == 0)
			{
				ret += ' ';
			}
			else
			{
				ret += 'a'+next-1;
			}
		}
		return ret;
	};
	std::string decompressByte()
	{
		unsigned char mode = 0;
		in.readBits((char*)&mode, 2);
		if (mode == 0)
		{
			return "0";
		}
		unsigned int commaPos = 0;
		in.readBits((char*)&commaPos, 2);
		if (mode == 1 && commaPos == 3)
		{
			return "87,22";
		}
		unsigned int num = 0;
		if (mode == 1)
		{
			in.readBits((char*)&num, 7);
		}
		if (mode == 2)
		{
			in.readBits((char*)&num, 10);
		}
		if (mode == 3)
		{
			in.readBits((char*)&num, 13);
		}
		std::string ret;

		std::stringstream stream;
		stream << num;
		stream >> ret;
		if (commaPos > 0)
		{
			if (commaPos == ret.size())
			{
				ret = "0,"+ret;
			}
			else if (commaPos > ret.size())
			{
				std::string temp = ret;
				ret = "0,";
				for (int i = 0; i < commaPos-temp.size(); i++)
				{
					ret += "0";
				}
				ret += temp;
			}
			else
			{
				ret = ret.substr(0, ret.size()-commaPos)+','+ret.substr(ret.size()-commaPos, commaPos);
			}
		}
		return ret;
	};
	BitReader& in;
	std::ostream& out;
	std::string lastGenus;
};

int main(int argc, char** argv)
{
	double a = 11/40.2;
	if (*argv[1] == 'c')
	{
		FILE* in = fopen(argv[2], "rb");
		FILE* out = fopen(argv[3], "wb");
		BitWriter writer {out};
		PaleoCompressor comp {writer, in};
		comp.compress();
		if (argc == 5)
		{
			comp.printDiagnostics();
		}
	}
	else if (*argv[1] == 'd')
	{
		FILE* in = fopen(argv[2], "rb");
		std::ofstream out {argv[3]};
		BitReader reader {in};
		PaleoDecompressor decomp {reader, out};
		decomp.decompress();
	}
	else if (*argv[1] == 's')
	{
		std::cout << "measure";
		FILE* in = fopen(argv[2], "rb");
		FILE* out = fopen(argv[3], "wb");
		BitWriter writer {out};
		PaleoCompressor comp {writer, in};
		comp.compress();
		std::ofstream alphabets{argv[4]};
		alphabets << comp.alphabetsToBeCompressed;
		std::ofstream numbers{argv[5]};
		numbers << comp.numbersToBeCompressed;
	}
}
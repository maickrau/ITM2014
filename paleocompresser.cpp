#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

class BitWriter
{
public:
	BitWriter(FILE* s) : stream(s), bitLoc(0), nextBits(0) {};
	~BitWriter()
	{
		flushWrite();
	};
	void writeBits(char* target, int bits);
private:
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
		compressedBitsAlphabet = 0;
		compressedBitsNumber = 0;
		compressedBitsMisc = 0;
	};
	void compress()
	{
		wasteFirstLine();
		while (!feof(read))
		{
			compressLine();
		}
	};
	void printDiagnostics()
	{
		std::cout << "parts:\n";
		for (int i = 0; i < 14; i++)
		{
			std::cout << compressedBitsPart[i]/8 << "\n";
		}
		std::cout << "alphabet:\n";
		std::cout << compressedBitsAlphabet/8 << "\n";
		std::cout << "number:\n";
		std::cout << compressedBitsNumber/8 << "\n";
		std::cout << "misc:\n";
		std::cout << compressedBitsMisc/8 << "\n";
	};
private:
	size_t compressedBitsPart[14];
	size_t compressedBitsAlphabet;
	size_t compressedBitsNumber;
	size_t compressedBitsMisc;
	int currentPartCounter;
	void wasteFirstLine()
	{
		char a = fgetc(read);
		while (a != '\n')
		{
			a = fgetc(read);
		}
	};
	void compressLine()
	{
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
		int extraFlags = 0;
		int extraFlagPos = 0;
		flags |= hasNameBit(parts[0]);
		for (int i = 1; i < 10; i++)
		{
			flags |= isByteable(parts[i])<<(i);
			if (!isByteable(parts[i]))
			{
				extraFlags |= isNumerable(parts[i])<<(extraFlagPos);
				extraFlagPos++;
			}
		}
		flags |= isFloatable(parts[4], parts[3], parts[10])<<10;
		if (!isFloatable(parts[4], parts[3], parts[10]))
		{
			extraFlags |= isNumerable(parts[10]) << extraFlagPos;
			extraFlagPos++;
		}
		flags |= isFloatable(parts[4], parts[5], parts[11])<<11;
		if (!isFloatable(parts[4], parts[5], parts[11]))
		{
			extraFlags |= isNumerable(parts[11]) << extraFlagPos;
			extraFlagPos++;
		}
		flags |= isFloatable(parts[6], parts[3], parts[12])<<12;
		if (!isFloatable(parts[6], parts[3], parts[12]))
		{
			extraFlags |= isNumerable(parts[12]) << extraFlagPos;
			extraFlagPos++;
		}
		flags |= isByteable(parts[13]) << 13;
		if (!isByteable(parts[13]))
		{
			extraFlags |= isNumerable(parts[13]) << extraFlagPos;
			extraFlagPos++;
		}
		unsigned char writeExtraFlags = extraFlagPos;
		write.writeBits((char*)&flags, 14);
		write.writeBits((char*)&writeExtraFlags, 4);
		write.writeBits((char*)&extraFlags, extraFlagPos);
		currentPartCounter = 0;
		compressName(parts[0], flags&1);
		extraFlagPos = 0;
		for (int i = 1; i < 10; i++)
		{
			currentPartCounter = i;
			if (flags&(1<<(i)))
			{
				compressByteable(parts[i]);
			}
			else if (extraFlags&(1<<(extraFlagPos)))
			{
				compressNumber(parts[i]);
				extraFlagPos++;
			}
			else
			{
				extraFlagPos++;
			}
		}
		for (int i = 10; i < 13; i++)
		{
			currentPartCounter = i;
			if (flags&(1<<i))
			{
			}
			else if (extraFlags&(1<<extraFlagPos))
			{
				compressNumber(parts[i]);
				extraFlagPos++;
			}
			else
			{
				extraFlagPos++;
			}
		}
		currentPartCounter = 13;
		if (flags&(1<<13))
		{
			compressByteable(parts[13]);
		}
		else if (extraFlags&(1<<extraFlagPos))
		{
			compressNumber(parts[13]);
		}
		else
		{
		}
	};
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
		char size = str.size();
		compressedBitsPart[currentPartCounter] += 6+5*size;
		compressedBitsAlphabet += 6+5*size;
		write.writeBits(&size, 6);
		for (int i = 0; i < size; i++)
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
	void compressAnyString(std::string str)
	{
		char size = str.size();
		compressedBitsPart[currentPartCounter] += 4+8*size;
		compressedBitsMisc += 4+8*size;
		write.writeBits(&size, 4);
		for (int i = 0; i < size; i++)
		{
			char writeThis = str[i];
			write.writeBits(&writeThis, 8);
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
			if (line[pos] < '0' || line[pos] > '9')
			{
				return false;
			}
			num *= 10;
			num += line[pos]-'0';
			if (num >= 256)
			{
				return false;
			}
			pos++;
		}
		return true;
	};
	bool isFloatable(std::string line1, std::string line2, std::string line3)
	{
		if (!isNumerable(line1) || !isNumerable(line2) || !isNumerable(line3))
		{
			return false;
		}
		if (line1.size() < 2 || line2.size() < 2 || line3.size() < 2)
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
		unsigned char size = line.size();
		compressedBitsPart[currentPartCounter] += 4;
		compressedBitsNumber += 4;
		write.writeBits((char*)&size, 4);
		for (int i = 0; i < size; i++)
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
		compressedBitsPart[currentPartCounter] += 1;
		unsigned char writeThis = 0;
		int test = 0;
		int pos = 0;
		while (pos < line.size())
		{
			test *= 10;
			test += line[pos]-'0';
			assert(test < 256);
			writeThis *= 10;
			writeThis += line[pos]-'0';
			pos++;
		}
		write.writeBits((char*)&writeThis, 8);
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
			decompressLine(i == 3);
		}
	}
private:
	void printFirstLine()
	{
		out << ";;max dm;dm;ww;wh;uw;ah;lobes;max dm;ww/dm;ww/wh;uw/dm;WER\n";
	}
	void decompressPossibleFloat(int flags, int pos, int extraFlags, unsigned char& extraFlagPos, std::string up, std::string down, bool printNum)
	{
		if (flags&(1<<pos))
		{
			defloat(up, down, printNum);
		}
		else if (extraFlags&(1<<extraFlagPos))
		{
			out << decompressNumber();
			extraFlagPos++;
		}
		else
		{
			out << decompressMisc();
			extraFlagPos++;
		}
		out << ";";
	}
	void defloat(std::string left, std::string right, bool printNum)
	{
		left = strReplace(left, ',', '.');
		right = strReplace(right, ',', '.');
		if (printNum)
		{
			std::cout << left << " " << right << " ";
		}
		double a = atof(left.c_str());
		double b = atof(right.c_str());
		if (printNum)
		{
			std::cout << a << " " << b << " ";
		}
		double c = a/b;
		char outStr[20] {0};
		sprintf(outStr, "%1.9f", c);
		std::string val = strReplace(std::string(outStr), '.', ',');
		out << val;
		if (printNum)
			std::cout << val << "\n";
	}
	void decompressLine(bool printFlags)
	{
		int flags = 0;
		in.readBits((char*)&flags, 14);
		unsigned char extraFlagPos = 0;
		in.readBits((char*)&extraFlagPos, 4);
		int extraFlags = 0;
		in.readBits((char*)&extraFlags, extraFlagPos);
		if (printFlags)
		{
			for (int i = 0; i < 14; i++)
			{
				std::cout << (flags&(1<<i) ? "1" : "0");
			}
			std::cout << " " << (int)extraFlagPos << "\n";
			for (int i = 0; i < extraFlagPos; i++)
			{
				std::cout << (extraFlags&(1<<i) ? "1" : "0");
			}
		}
		if (flags&1)
		{
			lastGenus = decompressAlphabet();
		}
		std::string species = decompressAlphabet();
		out << lastGenus << " ";
		out << species << ";";
		extraFlagPos = 0;
		std::string parts[10];
		for (int i = 1; i < 10; i++)
		{
			if (flags&(1<<i))
			{
				parts[i] = decompressByte();
			}
			else if (extraFlags&(1<<extraFlagPos))
			{
				parts[i] = decompressNumber();
				extraFlagPos++;
			}
			else
			{
				parts[i] = decompressMisc();
				extraFlagPos++;
			}
			out << parts[i];
			if (i != 13)
			{
				out << ";";
			}
		}
		decompressPossibleFloat(flags, 10, extraFlags, extraFlagPos, parts[4], parts[3], printFlags);
		decompressPossibleFloat(flags, 11, extraFlags, extraFlagPos, parts[4], parts[5], printFlags);
		decompressPossibleFloat(flags, 12, extraFlags, extraFlagPos, parts[6], parts[3], printFlags);
		if (flags&(1<<13))
		{
			out << decompressByte();
		}
		else if (extraFlags&(1<<extraFlagPos))
		{
			out << decompressNumber();
		}
		else
		{
			out << decompressMisc();
		}
		out << "\n";
	};
	std::string decompressNumber()
	{
		std::string ret;
		unsigned char size = 0;
		in.readBits((char*)&size, 4);
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
		in.readBits((char*)&size, 6);
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
		unsigned char num = 0;
		in.readBits((char*)&num, 8);
		std::string ret;
		if (num > 99)
		{
			ret += '0'+num/100;
			num %= 100;
			if (num < 10)
			{
				ret += "0";
			}
		}
		if (num > 9)
		{
			ret += '0'+num/10;
			num %= 10;
		}
		ret += '0'+num;
		return ret;
	};
	std::string decompressMisc()
	{
		return "#DIV/0!";
	};
	BitReader& in;
	std::ostream& out;
	std::string lastGenus;
};

int main(int argc, char** argv)
{
	double a = 11/40.2;
	printf("%.9f\n", a);
	if (*argv[1] == 'c')
	{
		FILE* in = fopen(argv[2], "rb");
		FILE* out = fopen(argv[3], "wb");
		BitWriter writer {out};
		PaleoCompressor comp {writer, in};
		comp.compress();
		comp.printDiagnostics();
	}
	else
	{
		FILE* in = fopen(argv[2], "rb");
		std::ofstream out {argv[3]};
		BitReader reader {in};
		PaleoDecompressor decomp {reader, out};
		decomp.decompress();
	}
}
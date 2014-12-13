/***

Encodes the difference between two integers as 8-bit, 16-bit, or 32-bit binary number. 

The program will generate a list of 1,2, or 4 in 8-bit binary, which serves as decoding instruction

gcc ./numberdiff.c -o numberdiff.out
./numberdiff.out c compressed.out < curve1.dat instructionlist.out
./numberdiff.out d compressed.out > decompressed.out n instructionlist.out
or
./numberdiff.out d compressed.out > decompressed.out y instructionlist.out
 
y/n selects whether decimals which are 0 are printed, eg. y -> 0.20, n -> 0.2
***/

//Debug needed. Decompressor part has Segmentation fault (core dumped) problem. Could be an argument issue.

#include <stdio.h>
#include <stdint.h>

void compress(FILE* out, FILE* li)
{
	int oldN = 0;
	int newN = 0;
	int8_t l[3] = {1,2,4}; //Comment this line in the final version, length list will be a part of decompresser 
	int8_t diff;
	while (scanf("%d", &newN) != EOF)
	{
                //printf("%d",&newN);
		if (newN-oldN >= 32768 || newN-oldN <= -32767)
		{
			int32_t largeDiff = newN-oldN;
			fwrite(&largeDiff, 4, 1, out);
			fwrite(&l+2, 1, 1, li); //Comment this line in the final version, length list will be a part of decompresser
		}
		else if ((newN-oldN >= 16384 && newN-oldN < 32768 )|| (-32767<newN-oldN && newN-oldN <= -16383))
		{
			int16_t midDiff = newN-oldN;
			fwrite(&midDiff, 2, 1, out);
			fwrite(&l+1, 1, 1, li); //Comment this line in the final version, length list will be a part of decompresser
		}
		else
		{
			diff = newN-oldN;
			fwrite(&diff, 1, 1, out);
			fwrite(&l,1, 1, li); //Comment this line in the final version, length list will be a part of decompresser
		}
		oldN = newN;
	}
}

void decompress(FILE* in, FILE* li, int includeDecimal)
{

	int num = 0;
	int8_t length=1;
        fread(&length,1,1,li);

	while (!feof(in)&&!feof(li))
	{

		if (length==4) 
		{
			int32_t largeDiff=0;
			fread(&largeDiff, 4, 1, in);
			num += largeDiff;
		}
		else if (length==2)
		{
			int16_t largeDiff=0;
			fread(&largeDiff, 2, 1, in);
			num += largeDiff;
		}
		else
		{
			int8_t diff=0;
			fread(&diff, 1, 1, in);
			num += diff;
		}
		if (includeDecimal)
		{
			if (num < 0) putchar('-');
			printf("%d.%02d\n", abs(num/100), abs(num%100));
		}
		else
		{
			double printThis = num/100.0;
			printf("%g\n", printThis);
		}
		fread(&length,1,1,li);
		//fread(&diff, 2, 1, in);
	}
}

int main(int argc, char** argv)
{
	if (*argv[1] == 'c')
	{
		FILE* fp = fopen(argv[2], "wb");
		FILE* li = fopen(argv[3], "wb"); //length instructions
		compress(fp,li);
		fclose(fp);
		fclose(li);
	}
	else
	{
		FILE* fp = fopen(argv[2], "rb");
		FILE* li = fopen(argv[4], "rb"); //length instructions
		decompress(fp,li, *argv[3] == 'y');
		fclose(fp);
		fclose(li);
	}
}

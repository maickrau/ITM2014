/***

Encodes the difference between two numbers as 16-bit number. If the difference doesn't fit in 16 bits, use 32 bits.

gcc numberdiff.c -o numberdiff.out
./numberdiff.out c compressed.out < curve1.dat
./numberdiff.out d compressed.out > decompressed.out n
or
./numberdiff.out d compressed.out > decompressed.out y

y/n selects whether decimals which are 0 are printed, eg. y -> 0.20, n -> 0.2
***/

#include <stdio.h>
#include <stdint.h>

void compress(FILE* out)
{
	int oldN = 0;
	int newN = 0;
	int16_t diff;
	while (scanf("%d", &newN) != EOF)
	{
		if (newN-oldN >= 32768 || newN-oldN <= -32767)
		{
			diff = -32767;
			fwrite(&diff, 2, 1, out);
			int32_t largeDiff = newN-oldN;
			fwrite(&largeDiff, 4, 1, out);
		}
		else
		{
			diff = newN-oldN;
			fwrite(&diff, 2, 1, out);
		}
		oldN = newN;
	}
}

void decompress(FILE* in, int includeDecimal)
{
	int16_t diff;
	int num = 0;
	fread(&diff, 2, 1, in);
	while (!feof(in))
	{
		if (diff == -32767)
		{
			int32_t largeDiff = 0;
			fread(&largeDiff, 4, 1, in);
			num += largeDiff;
		}
		else
		{
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
		fread(&diff, 2, 1, in);
	}
}

int main(int argc, char** argv)
{
	if (*argv[1] == 'c')
	{
		FILE* fp = fopen(argv[2], "wb");
		compress(fp);
		fclose(fp);
	}
	else
	{
		FILE* fp = fopen(argv[2], "rb");
		decompress(fp, *argv[3] == 'y');
		fclose(fp);
	}
}

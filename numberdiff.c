/***

Encodes the difference between two numbers as 16-bit number

gcc numberdiff.c -o numberdiff.out
./numberdiff.out c compressed.out < curve1.dat
./numberdiff.out d compressed.out > decompressed.out

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
		diff = newN-oldN;
		fwrite(&diff, 2, 1, out);
		oldN = newN;
	}
}

void decompress(FILE* in)
{
	int16_t diff;
	int num = 0;
	fread(&diff, 2, 1, in);
	while (!feof(in))
	{
		num += diff;
		if (num < 0) putchar('-');
		printf("%d.%02d\n", abs(num/100), abs(num%100));
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
		decompress(fp);
		fclose(fp);
	}
}

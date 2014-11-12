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
	while (!feof(in))
	{
		fread(&diff, 2, 1, in);
		num += diff;
		printf("%d\n", num);
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

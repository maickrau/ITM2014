/***

Encodes/decodes numbers in text format into 2-byte format. Call "./shortnumber.out c filename" to encode and "./shortnumber.out d filename" to decode.

2-byte format is composed of byte pairs that represent numbers, with two control pairs:

-32768: add 65534 to the next number
-32767: subtract 65534 from the next number

Any other pair is mapped to a number in the usual way.

***/

#include <stdio.h>
#include <stdint.h>
#include <math.h>

void compress(FILE* stream)
{
	int number;
	int16_t tooBig = -32768;
	int16_t tooSmall = -32767;
	while (scanf("%d", &number) != EOF)
	{
		while (number > 32767)
		{
			fwrite(&tooBig, 2, 1, stream);
			number -= 65534;
		}
		while (number < -32766)
		{
			fwrite(&tooSmall, 2, 1, stream);
			number += 65534;
		}
		int16_t writeNumber = number;
		fwrite(&writeNumber, 2, 1, stream);
	}
}

void decompress(FILE* stream)
{
	int number = 0;
	while (1)
	{
		uint16_t a;
		fread(&a, 2, 1, stream);
		if (feof(stream))
		{
			return;
		}
		while ((int16_t)a == -32768)
		{
			number += 65534;
			fread(&a, 2, 1, stream);
		}
		while ((int16_t)a == -32767)
		{
			number -= 65534;
			fread(&a, 2, 1, stream);
		}
		number += (int16_t)a;
		printf("%d\n", number);
		number = 0;
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
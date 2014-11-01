/***

Encodes/decodes numbers in text format into byte format. Call "./smallnumber.out c filename" to encode and "./smallnumber.out d filename" to decode.

Byte format is composed of bytes that represent numbers, with two control bytes:

-128: add 254 to the next number
-127: subtract 254 from the next number

Any other byte is mapped to a number in the usual way.

***/

#include <stdio.h>

void compress(FILE* stream)
{
	int number;
	while (scanf("%d", &number) != EOF)
	{
		while (number > 127)
		{
			putc(-128, stream);
			number -= 254;
		}
		while (number < -126)
		{
			putc(-127, stream);
			number += 254;
		}
		putc((char)number, stream);
	}
}

void decompress(FILE* stream)
{
	int number = 0;
	int negative = 0;
	while (1)
	{
		int a = getc(stream);
		if (feof(stream))
		{
			if (number != 0)
			{
				printf("%d\n", number);
			}
			return;
		}
		while ((char)a == -128)
		{
			number += 254;
			a = getc(stream);
		}
		while ((char)a == -127)
		{
			number -= 254;
			a = getc(stream);
		}
		number += (char)a;
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
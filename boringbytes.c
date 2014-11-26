#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main()
{
	srand48(250);
	int i = 0;
	for(;i<2000;i++)
	{
		unsigned char a = lrand48()%256;
		fwrite(&a, 1, 1, stdout);
	}
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main()
{
	srand48(713);
	uint16_t num = lrand48();
	int i = 0;
	for(;i<1250;i++)
	{
		num += lrand48()%400+lrand48()%400-395;
		fwrite(&num, 2, 1, stdout);
	}
}

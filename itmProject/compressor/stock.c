#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
int main(){srand48(713);uint16_t n=lrand48();int i=0;for(;i<100;i++){n+=lrand48()%400+lrand48()%400-395;fwrite(&n,2,1,stdout);}}

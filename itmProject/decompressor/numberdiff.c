#include <stdio.h>
#include <stdint.h>
main(int a,char**b){FILE*d=fopen(b[2],"rb");int16_t c;a=0;fread(&c,2,1,d);while(!feof(d)){a+=c;if(a<0)putchar('-');printf("%d.%02d\n",abs(a/100),abs(a%100));fread(&c,2,1,d);}}
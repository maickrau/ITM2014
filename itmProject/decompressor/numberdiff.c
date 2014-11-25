
#include <stdio.h>
#include <stdint.h>
main(int h,char**i){FILE*fp=fopen(i[2],"rb");f=*i[3]=='y';int16_t c;int32_t b=0,d=0;fread(&c,2,1,e);while(!feof(e)){if(c==-32767){d=0;fread(&d,4,1,e);b+=d;}else{b+=c;}if(f){if(b<0)putchar('-');printf("%d.%02d\n",abs(b/100),abs(b%100));}else{double g=b/100.0;printf("%g\n",g);}fread(&c,2,1,e);}}
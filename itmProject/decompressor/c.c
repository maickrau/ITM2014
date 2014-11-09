#include <stdio.h>
#include <stdint.h>
main(int d,char**e){FILE*c=fopen(e[1],"rb");d=0;while(1){int16_t a;fread(&a,2,1,c);if(feof(c)){break;}while(a==-32768){d+=65534;fread(&a,2,1,c);}while(a==-32767){d-=65534;fread(&a,2,1,c);}d+=a;printf("%d\n",d);d=0;}}
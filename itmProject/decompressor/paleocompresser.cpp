#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
typedef std::string t;typedef unsigned char u;typedef double v;typedef int x;class q{public:q(FILE*s):d(s),z(0),y(0){};void o(char*a,x b){x c=0;while(c<b){if(z==0){y=fgetc(d);}if(c%8==0){*a=0;}*a|=(y&(1<<z)?1:0)<<c%8;z++;z%=8;c++;if(c%8==0){a++;}}};FILE*d;x z;x y;};t p(t y,char a,char b){for(x i=0;i<y.size();i++){if(y[i]==a){y[i]=b;}}return y;}class r{public:r(q&y,std::ostream&z):y(y),z(z),f(){};void n(x a,x b,t c,t d){if(a&(1<<b)){s(c,d);}else{z<<m();}z<<";";}void s(t d,t e){if(e=="0"){z<<"#DIV/0!";return;}d=p(d,',','.');e=p(e,',','.');v a=atof(d.c_str()),b,c;b=atof(e.c_str());c=a/b;char f[20]{0};sprintf(f,"%1.9f",c);t g=p(t(f),'.',',');z<<g;}void h(){x a=0;y.o((char*)&a,14);if(a&1){f=j();}t d=j();z<<f<<" ";z<<d<<";";if(a&2){z<<";";}else{k();}t b[10];for(x i=2;i<10;i++){if(a&(1<<i)){b[i]=l();}else{b[i]=m();}z<<b[i];if(i!=13){z<<";";}}n(a,10,b[4],b[3]);n(a,11,b[4],b[5]);n(a,12,b[6],b[3]);if(a&(1<<13)){z<<l();}else{z<<m();}z<<"\n";};void k(){x a=0;y.o((char*)&a,13);z<<a<<";";}t m(){t a;u b=0,c=0,d;y.o((char*)&b,3);if(b&4){y.o((char*)&c,1);if(c){b+=4;}}if(b==1){b=10;}else if(b==10){b=1;}if(b>=2)b++;for(x i=0;i<b;i++){d=0;y.o((char*)&d,3);if(d&4){c=0;y.o((char*)&c,1);if(c){d+=4;}}if(d==0){a+=',';}else{a+='0'+d-1;}}return a;};t j(){t a;u c=0,b;y.o((char*)&c,5);if(c>=1)c++;if(c>=25)c+=3;if(c>=30)c++;if(c>=31)c++;if(c==34)c=39;for(x i=0;i<c;i++){b=0;y.o((char*)&b,5);if(b==0){a+=' ';}else{a+='a'+b-1;}}return a;};t l(){u a=0;y.o((char*)&a,8);t b;if(a>99){b+='0'+a/100;a%=100;if(a<10){b+="0";}}if(a>9){b+='0'+a/10;a%=10;}b+='0'+a;return b;};q&y;std::ostream&z;t f;};x main(x a,char**b){FILE*y=fopen(b[2],"rb");std::ofstream z{b[3]};q c{y};r d{c,z};z<<";;max dm;dm;ww;wh;uw;ah;lobes;max dm;ww/dm;ww/wh;uw/dm;WER\n";for(a=0;a<4128;a++){d.h();}}
#include <math.h>
#include <stdio.h>
typedef double h;h j(h x,h a,h b,h c,h d,h e,h f,h g){h o=sin(x*a+b)*(c*(2000.0-x)+d*x)/2000.0+(e*(2000.0-x)+f*x)/2000.0;return pow(fabs(o),g)*(o>0?1:-1)*100;}main(){int k,l,i=0;while(scanf("%d",&k)!=EOF){i++;l=j(i,0.0333,1.57,67.13,-63,-49.66,0,1.5)+j(i,0.067,-2.5,350,-300,200,0,1);k=k+l;printf("%d\n",k);}}
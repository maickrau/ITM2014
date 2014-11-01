#include <math.h>
#include <stdio.h>
double a=3,b=1.57,c=-0.12,d=0.08,e=-0.06,f=0.04;int main(){int g,h;double i=0;while(scanf("%d",&g)!=EOF){i+=1;h=-sin(i/a+b)*(c*(199-i)+d*i)+(e*(199-i)+f*i);h*=h*(h>0?1:-1);g+=h;printf("%d\n",g);}}
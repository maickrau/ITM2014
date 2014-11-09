#include <stdio.h>
main(){int n;while(scanf("%d",&n)!=EOF){if(n<0)putchar('-');printf("%d.%02d\n",abs(n/100),abs(n%100));}}
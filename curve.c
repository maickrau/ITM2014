/***

Fits the numbers onto a hard-coded sine curve. Call "./curve.out c" to fit and "./curve.out d" to defit.

Fitting inputs values and outputs offsets from the curve.

Defitting inputs the offsets and outputs the original values.

***/

#include <math.h>
#include <stdio.h>

double a = 3;
double b = 1.57;
double c = -0.12;
double d = 0.08;
double e = -0.06;
double f = 0.04;

int main(int argc, char** argv)
{
	int number;
	double i = 0;
	while (scanf("%d", &number) != EOF)
	{
		i += 1;
		int curve = (-sin(i/a+b)*(c*(199-i)+d*i)+(e*(199-i)+f*i));
		curve = curve*curve*(curve > 0 ? 1 : -1);
		if (*argv[1] == 'c')
		{
			number = number-curve;
		}
		else
		{
			number = number+curve;
		}
		printf("%d\n", number);
	}
}
/***

Fits the numbers onto a hard-coded sine curve. Call "./curve.out c" to fit and "./curve.out d" to defit.

Fitting inputs values and outputs offsets from the curve.

Defitting inputs the offsets and outputs the original values.

***/

#include <math.h>
#include <stdio.h>

double a = 0.0333;
double b = 1.57;
double c = 67.13;
double d = -63;
double e = -49.66;
double f = 0;
double g = 1.5;

double a2 = 0.067;
double b2 = -2.5;
double c2 = 350;
double d2 = -300;
double e2 = 200;
double f2 = 0;
double g2 = 1;

double getCurve(double x, double a, double b, double c, double d, double e, double f, double g)
{
	double temp = sin(x*a+b)*(c*(2000.0-x)+d*x)/2000.0+(e*(2000.0-x)+f*x)/2000.0;
	return pow(fabs(temp), g)*(temp > 0 ? 1 : -1)*100;
}

int main(int argc, char** argv)
{
	int number;
	double i = 0;
	while (scanf("%d", &number) != EOF)
	{
		i += 1;
		int curve = getCurve(i, a, b, c, d, e, f, g)+getCurve(i, a2, b2, c2, d2, e2, f2, g2);
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
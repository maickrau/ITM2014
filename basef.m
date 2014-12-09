function[y] =  basef(x)

a0 = 1;
a1 = 1;
b1 = 1;
a2 = 1;
b2 = 1;
a3 = 1;
b3 = 1;
a4 = 1;
b4 = 1;
a5 = 1;
b5 = 1;
a6 = 1;
b6 = 1;
w = 1.00023;

y = a0 + a1*cos(x*w) + b1*sin(x*w) + a2*cos(2*x*w) + b2*sin(2*x*w) + a3*cos(3*x*w) + b3*sin(3*x*w) + a4*cos(4*x*w) + b4*sin(4*x*w) + a5*cos(5*x*w) + b5*sin(5*x*w) + a6*cos(6*x*w) + b6*sin(6*x*w);

function[y] =  typeCf(x)

a0=-24.79;
a1=0.5782;
b1=-2.287;
a2=-2.14;
b2=-2.114;
a3=-21.78;
b3=14.05;
a4=22.94;
b4=101.7;
a5=-122.3;
b5=-13.79;
a6=10.2;
b6=-21.91;
a7=-13.05;
b7=-13.82;
a8=6.49;
b8=-2.276;
w=0.001638;

y = a0 + a1*cos(x*w) + b1*sin(x*w) + a2*cos(2*x*w) + b2*sin(2*x*w) + a3*cos(3*x*w) + b3*sin(3*x*w) + a4*cos(4*x*w) + b4*sin(4*x*w) + a5*cos(5*x*w) + b5*sin(5*x*w) + a6*cos(6*x*w) + b6*sin(6*x*w) + a7*cos(7*x*w) + b7*sin(7*x*w) + a8*cos(8*x*w) + b8*sin(8*x*w);

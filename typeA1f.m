function[y] =  typeA1f(x)

a0=-36.91;
a1=92.33;
b1=-491.9;
a2=-4.905;
b2=-1.908;
a3=-71.85;
b3=115.5;
w=0.006634;

l2=1.8;   
m2=-1.8;    
l3=0;  
m3=0;
p=10000;

y = (a0+a1*cos(x*w)+b1*sin(x*w)+a2*cos(2*x*w)+b2*sin(2*x*w)+a3*cos(3*x*w)+ b3*sin(3*x*w))*(l2*(p-x)+m2*x)/p+(l3*(p-x)+m3*x)/p;

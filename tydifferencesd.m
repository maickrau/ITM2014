tydiff=load("tydiff5.txt");

signs=load("tydiffsign.txt");

diff=zeros(10000,1);

oldiff=ty;

f = fopen('ty.txt', 'w+');

medv=([50,25,12]);

for i=1:3

for j=1:10000;

	if signs(j,5-i)==0; 
		tydiff(j)=medv(4-i)-tydiff(j);
		
	
	else;
		tydiff(j)=medv(4-i)+tydiff(j);

	endif;

endfor;
end;

pre=0;

for t=1:10000;

	if signs(t,1)==0; 

		tydiff(t)= pre-tydiff(t);
			
	else;
		tydiff(t)= pre+tydiff(t);

	endif;

	pre=tydiff(t);

endfor;

tydiff(7442)=0;

for i=1:10000;

str=num2str(tydiff(i));

fprintf(f, '%#s', str);
fprintf(f, '\n');

endfor;

fclose(f);

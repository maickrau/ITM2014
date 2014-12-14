ty=load("ty.txt");

pre=0;
cur=0;

diff=zeros(10000,1);

signs=zeros(10000,4);

oldiff=ty;

f = fopen('tydiff5.txt', 'w+');

f1 = fopen('tydiffsign.txt', 'w+');

for t=1:10000

	cur=oldiff(t);

	if (cur-pre)<0

		signs(t,1)=0; 
	
	else
		signs(t,1)=1; 

	endif
	
	diff(t)= abs(cur-pre);

	pre=cur;

endfor

medv=([50,25,12]);

for i=1:3

for j=1:10000;

	if (diff(j)-medv(i))<0

		signs(j,i+1)=0; 
	
	else
		signs(j,i+1)=1; 

	endif

	diff(j)=abs(diff(j)-medv(i));

endfor
end


for i=1:10000

fprintf(f1, '%#d %#d %#d %#d', signs(i,1),signs(i,2),signs(i,3),signs(i,4)); 
fprintf(f1, '\n');

fprintf(f, '%#.2f', diff(i));
fprintf(f, '\n');

endfor

fclose(f);
fclose(f1);

%Needs matlab/octave function typeA1f.m and basef.m, where contain the models

%It only model parts of the data. Some other parts of the data are tough, so just let they be.


ty=load('ty.txt');

result=0;

fid = fopen('ty.out', 'w+');

for i = 1:10000;

	if (201<=i && i<=749)||(1151<=i && i<=1699)||(2101<=i && i<=2649)||(3051<=i && i<=3549)||(3951<=i && i<=4499); %typeA1
		result=ty(i)-(7*basef(i)+typeA1f(i-200)+30);

	elseif (1<=i && i<=200)||(750<=i && i<=1150)||(1700<=i && i<=2100)||(2650<=i && i<=3050)||(3550<=i && i<=3950)||(4500<=i && i<=4900)||(5400<=i && i<=5750)||(6500<=i && i<=6650)||(7450<=i && i<=7600)||(8420<=i && i<=8540)||(9360<=i && i<=9480); %Base
		result=ty(i)-(7*basef(i)+30);

	else; result=ty(i);

	endif;

	fprintf(fid, '%#.4f', result);
	fprintf(fid, '\n');

endfor;

fclose(fid);

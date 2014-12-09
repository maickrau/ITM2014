ty=load('ty.txt');

result=0;

fid = fopen('ty.out', 'w+');

for i = 1:10000;

	if (201<=i && i<=749)||(1151<=i && i<=1699) ||(2101<=i && i<=2649) ||(3051<=i && i<=3549) ||(3951<=i && i<=4499) ||(4901<=i && i<=5399); %typeA1
		result=ty(i)-(6*basef(i)+typeA1f(i-200)+30);

	elseif (7301<=i && i<=7499)||(7601<=i && i<=7799) ||(8221<=i && i<=8419)||(8541<=i && i<=8779) ||(9151<=i && i<=9359) ||(9481<=i && i<=9719); %typeA2
		result=ty(i)-(6*basef(i)+typeA2f(i)+30);

	elseif (6750<=i && i<=7300); %typeC
		result=ty(i)-(6*basef(i)-typeCf(i+600)+30);

        elseif (7720<=i && i<=8250); %typeC
		result=ty(i)-(6*basef(i)+typeCf(i)+30);

        elseif (8780<=i && i<=9150); %typeC
		result=ty(i)-(6*basef(i)+typeCf(i-100)+30);

	elseif (9720<=i && i<=10000); %typeC
		result=ty(i)-(6*basef(i)+typeCf(i)+30);

	else;
		result=ty(i)-(6*basef(i)+30);
	endif;

	fprintf(fid, '%#.2f', result);
	fprintf(fid, '\n');

endfor;

fclose(fid);

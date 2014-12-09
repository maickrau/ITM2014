tyin=load('ty.out');

result=0;

f = fopen('tyftest.txt', 'w+');

for i = 1:10000;

	if (201<=i && i<=749)||(1151<=i && i<=1699) ||(2101<=i && i<=2649) ||(3051<=i && i<=3549) ||(3951<=i && i<=4499) ||(4901<=i && i<=5399); %typeA1
		result=tyin(i)+(basef(i)+typeA1f(i-200)+50);

	elseif (7301<=i && i<=7499)||(7601<=i && i<=7799) ||(8221<=i && i<=8419)||(8541<=i && i<=8779) ||(9151<=i && i<=9359) ||(9481<=i && i<=9719); %typeA2
		result=tyin(i)+(basef(i)+typeA2f(i)+50);

        elseif (5751<=i && i<=6499)||(6651<=i && i<=7300) ||(7800<=i && i<=8220) ||(8780<=i && i<=9150) ||(9720<=i && i<=10000);  %typeC
		result=tyin(i)+(basef(i)+typeCf(i)+50);

	else;
		result=tyin(i)+(basef(i)+50);
	endif;

	fprintf(f, '%#.2f', result);
	fprintf(f, '\n');

endfor;

fclose(f);

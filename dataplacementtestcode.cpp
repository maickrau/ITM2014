#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;
int main(){
const float a = 2.97828;
const float b = 2.40659;
const float c = 0.089496;
const float d = -0.0184135;
const float e = -1.00154;
const float f = 0.674291;
const int x = 200;
float out = 0.0;
float offsets[x];
ifstream filein ("offsets.dat");
if(filein.is_open() )
{
int i=0;
string line;
while( getline (filein, line))
{
offsets[i] = atof (line.c_str());
i++;
}
filein.close();
}
else cout << "Offsets file no opened.";
ofstream fileout ("decompressed.dat"); //decompressed data
if (fileout.is_open() )
{
for(int i = 1; i<=x; ++i)
{
out = -sin(i*a+b)*(c*(200-i)+d*i)+(e*(200-i)+f*i)+offsets[i-1]; //function here
fileout << out << endl;
}
fileout.close();
}
else cout << "File problem";
return 0;
}

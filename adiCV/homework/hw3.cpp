#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
using namespace std;

#if 1
void swap(char* str[])
{
	int i;
	for(i=0;i<3;i++)
	{
		str[0][i]='c';
	}

	for(i=0;i<3;i++)
	{
		str[2][i]='a';
	}
	return;
}

#endif

int main(void)
{
	int i;
	char* str[]={"aaa","bbb","ccc"};
	cout << "before swap str = "<< endl;
	for(i=0;i<4;i++)
	{
		cout  << str[i] << endl;// cout 編譯會直接讀取位置上的內容值
	}
	
	swap(str);
	cout << "after swap str = " << endl;
	for(i=0;i<4;i++)
	{
		cout  << str[i] << endl;// cout 編譯會直接讀取位置上的內容值
	}

	return 0;
}

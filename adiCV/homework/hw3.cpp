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
		cout  << str[i] << endl;// cout �sĶ�|����Ū����m�W�����e��
	}
	
	swap(str);
	cout << "after swap str = " << endl;
	for(i=0;i<4;i++)
	{
		cout  << str[i] << endl;// cout �sĶ�|����Ū����m�W�����e��
	}

	return 0;
}

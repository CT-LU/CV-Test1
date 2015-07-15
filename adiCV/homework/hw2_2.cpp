#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
using namespace std;

void swap(char *tmp1)
{
	int i;
	char tmp2[16];
	size_t size;
	size = strlen(tmp1);
	//cout << "size = " << size << endl;
	for(i=0;i<size;i++)
	{
		tmp2[i]=tmp1[size-i-1];
	}
	for(i=0;i<size;i++)
	{
		tmp1[i]=tmp2[i];
	}
	return;
}

int main(void)
{
	char str[16];
    cout << "please enter a string : ";
	cin >> str;
	cout << "before swap, the string is " << str << endl;// cout 編譯會直接讀取位置上的內容值
	swap(str);
	cout << "after swap, the string is  " << str << endl;
	return 0;
}

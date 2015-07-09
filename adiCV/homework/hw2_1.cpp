#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
using namespace std;

void swap(char *tmp)
{
	//cout << "tmp = " << tmp << endl;
	//printf("*tmp >>> %p\n", tmp);
	tmp[0]='c';
	tmp[2]='a';	
	//tmp = "cba"; //錯誤, 右邊abc電腦編譯新的位置給tmp,並非改變tmp原本指的位置的字串
	//cout << "tmp = " << tmp << endl;
	//printf("%p\n", tmp);
	return;
}

int main(void)
{
	char str[16];

	strcpy(str, "abc");
	//size_t len;
	printf("%c\n", *str);// 印出str所指的內容（a）
	printf("%p\n", str);// 印出str所存的位置（字串a的位置）
	cout << "before swap str = " << str << endl;// cout 編譯會直接讀取位置上的內容值
	//cout << "str = " << *str << endl;// 印出str所存的位置的內容（第一個位置的字串a）
	//len = strlen(str);
	//cout << "length of str = " << len << endl;
	swap(str);
	cout << "after swap str = " << str << endl;
	return 0;
}

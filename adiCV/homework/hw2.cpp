#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
using namespace std;

void swap(char *tmp)
{
	//cout << "tmp = " << tmp << endl;
	tmp = "cba";
	//cout << "tmp = " << tmp << endl;
	return;
}

int main(void)
{
	char *str="abc";
	//size_t len;
	printf("%p\n", str);// 印出str所存的位置（字串a的位置）
	cout << "before swap str = " << str << endl;// cout 編譯會直接讀取位置上的內容值
	//cout << "str = " << *str << endl;// 印出str所存的位置的內容（第一個位置的字串a）
	//len = strlen(str);
	//cout << "length of str = " << len << endl;
	swap(str);
	cout << "after swap str = " << str << endl;
	return 0;
}

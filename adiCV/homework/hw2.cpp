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
	printf("%p\n", str);// �L�Xstr�Ҧs����m�]�r��a����m�^
	cout << "before swap str = " << str << endl;// cout �sĶ�|����Ū����m�W�����e��
	//cout << "str = " << *str << endl;// �L�Xstr�Ҧs����m�����e�]�Ĥ@�Ӧ�m���r��a�^
	//len = strlen(str);
	//cout << "length of str = " << len << endl;
	swap(str);
	cout << "after swap str = " << str << endl;
	return 0;
}

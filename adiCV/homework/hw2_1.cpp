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
	//tmp = "cba"; //���~, �k��abc�q���sĶ�s����m��tmp,�ëD����tmp�쥻������m���r��
	//cout << "tmp = " << tmp << endl;
	//printf("%p\n", tmp);
	return;
}

int main(void)
{
	char str[16];

	strcpy(str, "abc");
	//size_t len;
	printf("%c\n", *str);// �L�Xstr�ҫ������e�]a�^
	printf("%p\n", str);// �L�Xstr�Ҧs����m�]�r��a����m�^
	cout << "before swap str = " << str << endl;// cout �sĶ�|����Ū����m�W�����e��
	//cout << "str = " << *str << endl;// �L�Xstr�Ҧs����m�����e�]�Ĥ@�Ӧ�m���r��a�^
	//len = strlen(str);
	//cout << "length of str = " << len << endl;
	swap(str);
	cout << "after swap str = " << str << endl;
	return 0;
}

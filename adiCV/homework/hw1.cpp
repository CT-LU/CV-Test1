#include <iostream>
#include <cstdlib>
using namespace std;

void swap(int *p1, int *p2) //p1,p2 ���O���V a,b ����m
{
	int tmp =*p1;
	*p1=*p2;
	*p2=tmp;
	return;
}

int main(void)
{
	int a=5, b=6;
	cout << "a=" << a  << " b=" << b << endl;
	swap(&a,&b); //�ǤJa,b����m
	cout << "a=" << a  << " b=" << b << endl;
	return 0;


}

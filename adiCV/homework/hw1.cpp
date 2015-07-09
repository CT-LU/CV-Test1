#include <iostream>
#include <cstdlib>
using namespace std;

void swap(int *p1, int *p2) //p1,p2 分別指向 a,b 的位置
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
	swap(&a,&b); //傳入a,b的位置
	cout << "a=" << a  << " b=" << b << endl;
	return 0;


}

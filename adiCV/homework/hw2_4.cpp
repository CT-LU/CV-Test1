#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
using namespace std;

void swap(string& tmp1)
{
	int i;
	char* char_str1;
	char_str1 = new char[tmp1.size()];
	char* char_str2;
	char_str2 = new char[tmp1.size()];
	strcpy(char_str1, tmp1.c_str());

	for(i=0;i<tmp1.size();i++)
	{
		char_str2[i] = char_str1[tmp1.size()-i-1];
	}
	cout << "in swap, the string of 'char_str' is " <<  char_str2  << endl;
	tmp1 = char_str2;
	cout << "in swap, the string of 'tmp1' is " <<  tmp1  << endl;
	delete [] char_str1;
	delete [] char_str2;
	return;
}

int main(void)
{
	std::string s;
	cout << "please enter a string : ";
	std::getline(std::cin, s);
	
	cout << "before swap, the string is " << s << endl;
	swap(s);
	cout << "after swap, the string is  " << s << endl;
	return 0;
}

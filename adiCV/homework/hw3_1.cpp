#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
using namespace std;

#if 1
void swap(int** data, int data_height, int data_width)
{
	int i,j, tmp[data_height][data_width];
	for(i=0;i < data_height; i++){
		for(j=0;j < data_width; j++){
			tmp[i][j] = data[data_height-i-1][data_width-j-1];
			//cout << tmp[i][j] << " ";
		}
	}

	for(i=0;i < data_height; i++){
		for(j=0;j < data_width; j++){
			data[i][j] = tmp[i][j];
			//cout << data[i][j] << " ";
		}
	}
	cout << endl;
}

#endif

int	main(void) {
	int i,j;
	int data_height,data_width;
	int **data;

	cout << "please enter 2D metric (n, m): ";
	cin >> data_height >> data_width;

	data = new int*[data_height];
	for(i = 0; i < data_height; i++)
		data[i] = new int[data_width];

	for(i=0;i < data_height; i++){
		for(j=0;j < data_width; j++){
			cin >> data[i][j];
		}     
		cout << endl;
	}

	for(i=0;i < data_height; i++){
		for(j=0;j < data_width; j++){
			cout << data[i][j] << " ";
		}
	}
	cout << endl;	

	swap(data, data_height, data_width);

	for(i=0;i < data_height; i++){
		for(j=0;j < data_width; j++){
			cout << data[i][j] << " ";
		}
	}
	cout << endl;

#if 0	
	int tmp[data_height][data_width];
	for(i=0;i < data_height; i++){
		for(j=0;j < data_width; j++){
			tmp[i][j] = data[data_height-i-1][data_width-j-1];
			cout << tmp[i][j] << " ";
		}
	}
	cout << endl;
#endif

	for(i = 0; i < data_height; i++)
		delete [] data[i];
	delete [] data;
	return 0;
}



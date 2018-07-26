#ifndef BITMAP_H
#define BITMAP_H
#include <iostream>
#include <string.h>
using namespace std;
class Bitmap{
public:
	Bitmap(){
		size = 1024;
		memset(arr, 0, sizeof(int)*1024);
	}
	void set(int num){
		int offset1 = num/32;
		int offset2 = num%32;
		offset2  = (1 << (31-offset2));
		arr[offset1] = arr[offset1] | offset2;
	}
	void reset(int num){
		int offset1 = num/32;
		int offset2 = num%32;
		offset2  = (1 << (31-offset2));
		arr[offset1] = arr[offset1] & (~offset2);
	}
	bool exist(int num){
		int offset1 = num/32;
		int offset2 = num%32;
		offset2  = (1 << (31-offset2));
		return offset2 & arr[offset1];
	}
private:
	int arr[1024]; //1024*32
	int size;
};
#endif
#if 0
int main()
{
	Bitmap bmp;
	bmp.set(3);
	bmp.set(33);
	bmp.set(63);
	cout << bmp.exist(3) << bmp.exist(33) << bmp.exist(63) <<endl;
    bmp.reset(33);
	cout << bmp.exist(3) << bmp.exist(33) << bmp.exist(63) <<endl;
}
#endif

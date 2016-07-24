#include"FileCompress.h"

void test()
{
	FileCompress f;
	f.Compress("input.txt");

	f.unCompress("input.txt");
}


int main()
{
	test();
	system("pause");
	return 0;
}
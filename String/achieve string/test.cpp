#include"String.h"

int main()
{
	String str_1 = "hallo";
	String str_2("world!\n");
	String str_3 = str_2.SubStr(0, 5);
	char str[] = "hahaha";
	//str_1.Swap(str_2);
	//str_1 += 'h';
	//str_2.Append(str);
	//str_1.Resize(3, 'h');
	//str_2.Resize(10, 'h');
	//cout << str_2.Find('d') << endl;
	str_1.Insert(0, str);
	str_1.Erase(0, 7);
	cout << str_1.C_STR() << endl;
	cout << str_2.C_STR() << endl;
	cout << str_3.C_STR() << endl;
	system("pause");
	return 0;
}
#ifndef __BOOK_H__
#define __BOOK_H__


#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>
#include <stdlib.h>



struct data
{
	char name[20];
	char sex[10];
	short age;
	unsigned long int phone_number;
	char address[30];
};

enum choice
{
	exi,
	add,
	del,
	find,
	mod,
	show,
	emp,
	sort	
};







void print(void);
void show_m(struct data* s, int n);
int add_m(struct data* s, int n);
int find_f(char name_f[20], struct data* s, int n);
int mod_m(struct data* s, int f);
void del_d(struct data* s, int f, int n);
void sort_s(struct data* s,int n);
int struct_cmp(const struct data* s1, const struct data* s2);
void initial(FILE* pf, const struct data* s, int* n);






#endif // __BOOK_H__

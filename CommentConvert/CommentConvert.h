#ifndef __CommentConvert_H__
#define __CommentConvert_H__

#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>
#include <stdlib.h>
#include<io.h>





enum state
{
	nulstate,
	cstate,
	cppstate,
	endstate
};



void test(char *cn);
void CommentConvert(pfin, pfout);
void Donul(FILE *pfin, FILE *pfout, enum state* s);
void Doc(FILE *pfin, FILE *pfout, enum state* s);
void Docpp(FILE *pfin, FILE *pfout, enum state* s);



#endif // __CommentConvert_H__


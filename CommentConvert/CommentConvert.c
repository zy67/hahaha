#include "CommentConvert.h"



void CommentConvert(FILE * pfin, FILE * pfout)
{
	enum state s;
	s = nulstate;
	while (s != endstate)
	{
		switch (s)
		{
		case nulstate:
		{
			Donul(pfin, pfout, &s);
			break;
		}
		case cstate:
		{
			Doc(pfin, pfout, &s);
			break;
		}
		case cppstate:
		{
			Docpp(pfin, pfout, &s);
			break;
		}
		}
	}
}



void Donul(FILE *pfin, FILE *pfout, enum state* s)
{
	int frist = getc(pfin);
	int second;
	switch (frist)
	{
	case '/':
	{
		second = getc(pfin);
		switch(second)
		{

		case '*':
		{
			putc('/', pfout);
			putc('/', pfout);		
			*s = cstate;
			break;
		}
		case '/':
		{
			putc(frist, pfout);
			putc(frist, pfout);
			*s = cppstate;
			break;
		}
		default:
		{
			putc(frist, pfout);
			putc(frist, pfout);
			break;
		}

		}
		break;
	}
	case EOF:
	{
		*s = endstate;
		break;
	}
	default:
	{
		putc(frist, pfout);
		break;
	}
	}
}


void Doc(FILE *pfin, FILE *pfout, enum state* s)
{
	int frist = getc(pfin);
	int second;
	int thirdly;
	int fourth;
	switch (frist)
	{

	case '*':
	{
		second = getc(pfin);
		switch (second)
		{

		case '/':
		{
			thirdly = getc(pfin);
			if (thirdly == '\n')
			{
				putc(thirdly, pfout);
			}
			else if (thirdly == '/')
			{
				fourth = getc(pfin);
				if (fourth == '*')
				{
					putc('\n', pfout);
					ungetc('*', pfin); 
					ungetc('/', pfin);
				}
				else
				{
					ungetc(fourth, pfin);
					ungetc('/', pfin);
				}
			
			}
			else if (thirdly == EOF)
			{
				*s = endstate;
			}
			else
			{
				putc('\n', pfout);
				putc(thirdly, pfout);
			}
			*s = nulstate;
			break;
		}
		default:
		{
			ungetc(second, pfin);
			putc(frist, pfout);
		}

		}
	
		break;
	}
	case '\n':

	{
		putc(frist, pfout);
		putc('/', pfout);
		putc('/', pfout);
		break;
	}
	default:
	{
		putc(frist, pfout);
		break;
	}

	}
}

void Docpp(FILE *pfin, FILE *pfout, enum state* s)
{
	int frist = getc(pfin);
	switch (frist)
	{
		case '\n':
		{
			putc(frist, pfout);
			*s = nulstate;
			break;
		}
		case EOF:
		{
			*s = endstate;
			break;
		}
		default:
		{
			putc(frist, pfout);
			break;
		}
	}
}
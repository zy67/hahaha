#include "CommentConvert.h"



void test(char *cn)
{
	FILE * pfin = NULL;
	FILE * pfout = NULL;
	errno_t err1;
	errno_t err2;
	err1 = fopen_s(&pfin, cn, "r");
	if (pfin == NULL)
	{
		perror("open file");
		exit(EXIT_FAILURE);
	}
	err2 = fopen_s(&pfout, "D://比特科技//作业//Project22CommentConvert//haha//get.c", "w");
	if (pfout == NULL)
	{
		perror("open file");
		free(pfin);
		pfin = NULL;
		exit(EXIT_FAILURE);
	}
	CommentConvert(pfin, pfout);
	fclose(pfin);
	pfin = NULL;
	fclose(pfout);
	pfout = NULL;
}









int main()
{
	long handle;
	struct _finddata_t find;
	char *cn;
	const char *ch = "D://比特科技//作业//Project22CommentConvert//haha//*.c";
	handle = _findfirst(ch, &find);
	if (handle != -1)
	{
		cn = find.name;
		printf("%s", cn);
		/*test(cn);*/
	}
	else
		return 0;
	while (!(_findnext(handle, &find)))
	{
		cn = find.name;
		test(cn);
	}
	_findclose(handle);
	printf("转换完成\n");
	system("pause");
	return 0;
}
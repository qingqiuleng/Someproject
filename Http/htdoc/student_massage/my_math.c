#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void mymath(char* arg)
{
	char *data[3];
	data[2]=NULL;
	int i=1;
	char *end=arg+strlen(arg)-1;
	while(end>arg)
	{
		if(*end=='=')
		{
			data[i--]=end+1;
		}
		if(*end=='&')
		{
			*end='\0';
		}
		end--;
	}
	printf("<html>\n");//子进程直接将信息写到管道里面，然后父进程再从管道里读取并发送给socket
	printf("<h1>");
	printf("%s+%s=%d",data[0],data[1],atoi(data[0])+atoi(data[1]));
	printf("</h1>\n");
	printf("</html>\n");
}

int main()
{
	char method[1024];
	char arg[1024];
	int len=-1;
	char content_length[1024];
	if(getenv("REQUEST_METHOD"))
	{
		strcpy(method,getenv("REQUEST_METHOD"));
	}

	if(strcasecmp(method,"GET")==0)
	{
		if(getenv("QUERY_STRING"))
		{
			strcpy(arg,getenv("QUERY_STRING"));
		}
	}
	else
	{
		if(getenv("CONTENT_LENGTH"))
		{
			strcpy(content_length,getenv("CONTENT_LENGTH"));
			len=atoi(content_length);
		}
		int i=0;
		for(;i<len;i++)
		{
			read(0,&arg[i],i);
		}
		arg[i]='\0';

	}
	mymath(arg);
	return 0;
}







#include<stdio.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>

#define SIZE 256

void echo_errno(int arg)
{
	printf("%d is error\n");
}


int get_line(int sock,char* buf,int len)//逐行获取
{
	if(!buf||len<0)
		return -1;

	int i=0;
	int ret=0;
	char c='\0';
	while((i<len-1)&&c!='\n')
	{
		ret=recv(sock,&c,1,0);
		if(ret>0)//接收成功
		{
			if(c=='\r')
			{
				ret=recv(sock,&c,1,MSG_PEEK);//不把所读的字符从TCP中移除
				if(ret>0&&c=='\n')// \r\n
				{
					recv(sock,&c,1,0);
				}
				else// \r
				{
					c='\n';
				}
			}
			buf[i++]=c;
		}
		else//接收失败
		{
			c='\n';
		}	
	}
	buf[i]='\0';
	return i;
}

void clear_head(int sock)
{
	char buf[SIZE];
	int len=SIZE;
	int ret=-1;
	do{
		ret=get_line(sock,buf,len);

	}while(ret>0&&strcmp(buf,"\n")!=0);
}

void exe_cgi(int sock,const char*method,const char*path,const char* query_string)
{
	char buf[SIZE];
	int ret=-1;
	int content_length=-1;
	int cgi_input[2];
	int cgi_output[2];
	char method_env[SIZE];
	char query_string_env[SIZE];
	char content_length_env[SIZE];
	
	printf("i will fork new pid 1\n");
	if(strcasecmp(method,"GET")==0)
	{
		printf("clean head\n");
		clear_head(sock);
	}
	else
	{
		do{
			ret=get_line(sock,buf,sizeof(buf));//直接获取最后一行的请求正文

			printf("i will fork new pid 3\n");
			printf("buf:%s\n",buf);
			if(strncasecmp(buf,"Content-Length: ",16)==0)
			{
				
				printf("i will fork new pid 4\n");
				content_length=atoi(&buf[16]);
			}
		}while(ret>0&&(strcmp(buf,"\n")!=0));		
		if(content_length==-1)//POST方法的最下面一行是请求正文，为Content_Length:
		{
			printf("content_length:%d\n",content_length);
			echo_errno(sock);
			return;
		}
	}
	printf("clean over\n");	
	sprintf(buf,"HTTP/1.0/200 OK\r\n\r\n");//将这句话回写给server的状态行
	send(sock,buf,strlen(buf),0);
	
	
	if(pipe(cgi_output)<0)
	{
		echo_errno(sock);
		return;
	}
	if(pipe(cgi_input)<0)
	{
		echo_errno(sock);
		return;
	}
	

	pid_t id=fork();
	if(id<0)
	{
		perror("fork");
		return;
	}
	else if(id==0)//child
	{
	//	close(cgi_input[1]);//子进程将数据写到管道中，父进程从管道中读取数据
	//	close(cgi_output[0]);//父进程通过检测获取的信息并发给浏览器
		sprintf(method_env,"REQUEST_METHOD=%s",method);
		dup2(cgi_input[0],0);
		dup2(cgi_output[1],1);
		putenv(method_env);
		if((strcasecmp(method,"GET"))==0)//GET
		{
			sprintf(query_string_env,"QUERY_STRING=%s",query_string);
			putenv(query_string_env);
	}
		else//POST
		{
			sprintf(content_length_env,"CONTENT_LENGHT=%d",content_length);
			putenv(content_length_env);
		}
		execl(path,path,0);//将获取的环境变量写到已知的路径中
		exit(5);
	}
	else//father
	{
		close(cgi_input[0]);
		close(cgi_output[1]);

		char c='\0';
		int i=0;
		if(strcasecmp(method,"POST")==0)
		{
			for(;i<content_length;i++)
			{
				recv(sock,&c,1,0);
				write(cgi_input[1],&c,1);
			}	
		}
		printf("\n");
		int ret=0;
		while((ret=read(cgi_output[0],&c,1))>0)//因为html中的方法是经过重定向输出的，所以它直接输出到了管道里面，现在再通过从管道中 获取到每个字符，然后再经过send发送到相应的用户套接字上，管道起了一个中介作用
		{ 
			send(sock,&c,1,0);
		}
		wait(id,NULL,0);
	}	
}

void echo_www(int sock,const char* path,ssize_t size)
{
	int fd=open(path,O_RDONLY);
	if(fd<0)
	{
		echo_errno(sock);
		return;
	}	
	printf("get a new client  sock:%d,path:%s\n",sock,path);
	char state[SIZE];
	sprintf(state,"HTTP/1.0/200 OK\r\n\r\n");
	send(sock,state,strlen(state),0);
	if(sendfile(sock,fd,NULL,size)<0)
	{
		printf("sendfail\n");
		echo_errno(sock);
		return;
	}
	printf("sendover\n");
	close(fd);
}


void* accept_request(void* arg)//接收远端套接字
{
	int sock=(int)arg;
	char buf[SIZE];
	int len=sizeof(buf)/sizeof(buf[0]);
	char method[SIZE/10];
	char url[SIZE];
	char path[SIZE];
	int cgi=0;
	char* query_string=NULL;

	int ret=-1;
	ret=get_line(sock,buf,len);//一行一行的获取字符并将其存入到buf中
	if(ret<=0)
	{
		echo_errno(sock);
		return (void*)1;	
	}
	//printf("%s\n",buf);
	//获取方法（GET，POST）
	int i=0;
	int j=0;
	while((i<sizeof(method)-1)&&(j<sizeof(buf)/sizeof(buf[0]))&&!isspace(buf[i]))
	{
		method[i]=buf[i];
		i++;
		j++;
	}
	method[i]='\0';
	printf("method:%s\n",method);

	if(strcasecmp(method,"GET")!=0&&strcasecmp(method,"POST")!=0)//方法不正确
	{
		printf("method is error\n");
		echo_errno(sock);
		return (void*)2;
	}
	while(isspace(buf[j]))//跳过方法后面的空格
	{
		j++;
	}

	i=0;
	while((i<sizeof(url))&&(j<sizeof(buf)-1)&&!isspace(buf[j]))//将url统计出来（这里的url是截取了的，不包含协议）
	{
		url[i]=buf[j];
		i++;
		j++;	
	}
	url[i]='\0';
	printf("url:%s\n",url);
	if(strcasecmp(method,"POST")==0)//POST方法中url没有参数
	{
		cgi=1;
	}
	if(strcasecmp(method,"GET")==0)//GET方法
	{
		query_string=url;
		while(*query_string!='\0'&&*query_string!='?')	
		{
			query_string++;
			printf("haha\n");
		}
		if(*query_string=='?')
		{
			cgi=1;
			*query_string='\0';
			query_string++;
		}
	}
		sprintf(path,"htdoc%s",url);//填充path
		if(path[strlen(path)-1]=='/')
		{
			strcat(path,"my_s.html");
			printf("hehehe\n");
		}
		printf("path:%s\n",path);
		struct stat st;//描述文件属性的结构
		if(stat(path,&st)<0)// 如果失败则表示该路径错误
		{
			echo_errno(sock);
			return (void*)3;
		}
		else//描述正确
		{
			if(S_ISDIR(st.st_mode))//判断是否为一个目录		{
				strcat(path,"my_s.html");	
			
			else if((st.st_mode&S_IXUSR)||(st.st_mode&S_IXGRP)||st.st_mode&S_IXOTH)//判断是否为一个可执行文件
			{
				printf("i will\n");
				cgi=1;
			}
			else
			{}
		
		printf("%d\n",cgi);			
		if(cgi==1)
		{
			printf("path:%s\n",path);
			//printf("i will\n");
			exe_cgi(sock,method,path,query_string);
		}
		else
		{	
			printf("yoxi:%s\n",path);
			clear_head(sock);
			echo_www(sock,path,st.st_size);
		}					
		}		

	close(sock);
	return (void*)0;
}

void usage(char* argv)
{
	printf("usage [ip][port]%s\n",argv);
}

static int startup(char* ip,int port)//创建本地套接字
{
	int listen_socket=socket(AF_INET,SOCK_STREAM,0);
	if(listen_socket<0)
	{
		perror("socket");
		exit(2);
	}
	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_port=htons(port);
	local.sin_addr.s_addr=inet_addr(ip);

	//绑定
	if(bind(listen_socket,(struct sockaddr*)&local,sizeof(local))<0)
	{
		perror("bind");
		exit(2);
	}

	//监听
	if(listen(listen_socket,5)<0)
	{
		perror("listen");
		exit(3);
	}
	return listen_socket;
}

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		usage(argv[0]);
		exit(1);
	}

	struct sockaddr_in peer;
	socklen_t len=sizeof(peer);
	int listen_sock=startup(argv[1],atoi(argv[2]));	
	printf("listen_sock:%d\n",listen_sock);

	int done=0;
	while(!done)
	{
		//接收远端套接字
		int new_sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
		printf("new_sock:%d\n",new_sock);
		if(new_sock>0)
		{
			//多线程模式的
			pthread_t id;
			pthread_create(&id,NULL,accept_request,(void*)new_sock);
			pthread_detach(id);
		}
	}
	return 0;
}




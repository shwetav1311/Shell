/* @author Shweta Verma
Roll No 201506606 
v4
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "Parse.c"

#define IN_WORD 1
#define OUT_WORD 0

char chnge_dir[1024];


void getCurrentDir();
void changeDir(char *);
void parseInternal(char arr[],char tokens[][256],int *cnt,int *isPiped);
int checkIsSetter(char arr[]);
void export(char tokens[][256],int cntParam,int isSetter);
void sighandler(int signum) 
{ 
	//printf("got signal %d\n",signum );
	printf("\nMy_Shell:%s$ ",chnge_dir);
   	fflush( stdout ); 
}

int main()
{
	
	char arr[SIZE];
	char arr_dup[SIZE];
	getCurrentDir();
	printf("My_Shell:%s$ ",chnge_dir);
	int pid;
	int len;

	char tokens[256][256];
	int isPiped,cntParam=0;

	signal(SIGINT, SIG_IGN); 
  	signal(SIGINT, sighandler);


	while(strcmp(fgets(arr,SIZE,stdin),"exit\n")!=0)
	{
		len=strlen(arr);
		arr[len-1]='\0';

		parseInternal(arr,tokens,&cntParam,&isPiped);

		if(strcmp(tokens[0],"cd")==0 && !isPiped)
		{
			if(cntParam==1)
			{
				changeDir(NULL);
			}else
			{
				changeDir(tokens[1]);
			}
			insertIntoHistoryMain(arr);
			getCurrentDir();
		}else if(strcmp(tokens[0],"export")==0 && !isPiped)
		{
			export(tokens,cntParam,0);
			insertIntoHistoryMain(arr);

		}else if ( checkIsSetter(tokens[0]) && !isPiped)
		{
			export(tokens,cntParam,1);
			insertIntoHistoryMain(arr);
		}else
		{
			pid=fork();
			if(pid==0)
			{
				parse(arr);
				exit(0);
			}else
			{
				wait(0);
			}
		}
		
		// getCurrentDir();
		printf("My_Shell:%s$ ",chnge_dir);
	}

	printf("Bye..\n");


}



int checkIsSetter(char arr[])
{
	char *ch=arr;

	while(*ch!='\0')
	{
		if(*ch=='=')
		{
			return 1;
		}
		ch++;
	}

	return 0;
}

void export(char tokens[][256],int cntParam,int isSetter)
{
	char *ch1,*ch2;
	int i;
	if(isSetter)
	{
		i=0;
	}else
	{
		i=1;
	}

	for(;i<cntParam;i++)
	{
		//printf("%s\n",tokens[i]);
		ch2=NULL;
		ch1=tokens[i];
		while(*ch1!='\0')
		{	
			if(*ch1=='=')
			{
				*ch1='\0';
				ch1++;
				ch2=ch1;
				ch1=tokens[i];
				break;
			}
			ch1++;
		}

		if(ch2==NULL)
		{
			printf("%s not a valid identifier\n",tokens[i]);
		}else{

			if(setenv(ch1,ch2,1) < 0 )
			{
				printf("Failed to export\n");
			}
		}
	}
}

void parseInternal(char arr[],char tokens[][256],int *cnt,int *isPiped)
{

	int l=0,j=0;
	int word_status=OUT_WORD;
	char buff[256];
	char *ch=arr;
	*isPiped=0;
	while(*ch != '\0')
	{
		if(*ch==' ' || *ch=='\t')
		{
			if(word_status==IN_WORD)
			{
				word_status=OUT_WORD;
				buff[l++]='\0';
				strcpy(tokens[j++],buff);
				l=0;
			}
		}else if(*ch=='|')
		{
			*isPiped = 1;
			return;
		}
		else 
		{
			word_status=IN_WORD;
			buff[l++]=*ch;
		}

		ch++;
	
	}
	
	if(word_status==IN_WORD)  
	{
		buff[l++]='\0';
		strcpy(tokens[j++],buff);
		
	}	

	*cnt=j;
}
void getCurrentDir()
{
	if(getcwd(chnge_dir,sizeof(chnge_dir))!=NULL)
	{
		//printf("%s\n",chnge_dir);
	}else
	{
		perror("getcwd() error");
	}
}



void changeDir(char *dirName)
{
	if(dirName==NULL)
	{
		chdir(getenv("HOME"));
		return;
	}

	if(chdir(dirName)!= -1)
	{
		getCurrentDir();
		//printf("changed to%s\n",chnge_dir);
	}else
	{
		printf("%s :Path not found\n",dirName);
	}
	
}
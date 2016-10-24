/* @author Shweta Verma
Roll No 201506606 
v4
*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define IN_WORD 1
#define OUT_WORD 0
#define HISTORY_SIZE 100

#define SIZE 512

void tokenize(char arr[]);
void execute();
void changeDir(char *dirName);
void getPwd(char *arr);
void echo(char *arr[]);
void loadHistory();
void insertIntoHistory(char *arr);
void searchHistory(char mid[],char arr[]);
void getLineFromHistory(char arr[],int num);
void history(char *arr[]);


char history_cmds[200][512];
int history_cnt=0;
char historyFile[256];
int num_lines;

struct Command
{
	char *arr[SIZE];
	char cmd_arr[SIZE][256];
	int isInRedirect;
	char inFilename[SIZE];
	int isOutRedirect;
	char outFilename[SIZE];
};

struct Command commands[500];
int command_cnt=0;
int cntPipe=0;

int parse(char argv[])
{

	
	sprintf(historyFile,"%s/my_history",getenv("HOME"));
	loadHistory();
	char buff[SIZE];
	char *ch = argv;
	int cnt=0;
	int w_status=OUT_WORD,q_status = OUT_WORD, l=0,i;

	/* 	1. Calculate number if pipes
		2. Extract command
		3. Maintain last command
	*/

		//count the number of pipes
	while(*ch != '\0')
	{
		
		if(*ch=='"' || *ch=='\'')
		{
			if(q_status==OUT_WORD)
			{
				q_status=IN_WORD;
			}else
			{
				q_status=OUT_WORD;
			}
		}
		else if(*ch=='|')
		{
			if(q_status!=IN_WORD)
				cntPipe++;
		}
		ch++;
	
	}
	
	ch = argv;
	int isBanged=0;
	char *ch2,*ch3;
	char temp_buff[512];
	char mid[512],first[1024];
	strcpy(mid,"\0");
	while(*ch != '\0')
	{
		if(*ch=='!')
		{
			isBanged=1;
			ch2=ch;  //string after !
			ch2++;
			*ch='\0';  //string before !
			l=0;
			strcpy(first,argv);
			while(*ch2 != '\0' && *ch2!=' ')
			{
				temp_buff[l++]=*ch2;
				ch2++;	
			}
			//ch2 now points to after bang
			temp_buff[l]='\0';
			ch3=temp_buff;
			if(*ch3 == '!')
			{
				//printf("Found the second bang\n");
				 getLineFromHistory(mid,history_cnt-1);

			}else if(*ch3 >= 48 && *ch3<=57)
			{
				char num[5];
				l=0;
				while(*ch3 >= 48 && *ch3<=57)
				{
					num[l++]= *ch3;
					ch3++;

				}
				num[l]='\0';
				getLineFromHistory(mid,atoi(num)-1);

			}else if ((*ch3 >= 65 && *ch3<=90) || (*ch3 >= 97 && *ch3<=122))
			{
				char arr[256];
				l=0;
				while((*ch3 >= 65 && *ch3<=90) || (*ch3 >= 97 && *ch3<=122))
				{
					arr[l++]= *ch3;
					ch3++;

				}
				arr[l]='\0';

				searchHistory(mid,arr);

			}else if (*ch3=='-')
			{
				char num[5];
				l=0;
				ch3++;
				while(*ch3 >= 48 && *ch3<=57)
				{
					num[l++]= *ch3;
					ch3++;

				}	
				num[l]='\0';
				getLineFromHistory(mid,history_cnt-atoi(num));
			}

			break;

		}
		ch++;
	
	}


	
	char newCommand[1024];

	if(isBanged)
	{
		if(strcmp(mid,"-1")==0)
		{
			printf("My_Shell:event not found\n");
			return;
		}else if(strlen(mid)==0)
		{
			if(cntPipe==0)
			{
				return;
			}else
			{
				printf("My_Shell: syntax error near unexpected token `!'\n");
				return;
			}
			
			
		}
		// printf("before bang:%s\n",first);
		// printf("replace bang:%s\n",mid);
		// printf("after bang bang:%s\n",ch2);
		
		strcpy(newCommand,first);
		strcat(newCommand,mid);
		strcat(newCommand,ch2);
		printf("%s\n",newCommand);
		strcpy(argv,newCommand);

	}

	insertIntoHistory(argv);
	loadHistory();
	

	ch = argv;
	l=0;

	while(*ch != '\0')
	{
		
		if(*ch=='"' || *ch=='\'')
		{
			if(q_status==OUT_WORD)
			{
				q_status=IN_WORD;
			}else
			{
				q_status=OUT_WORD;
			}

		}
		else if(*ch=='|')
		{
			if(q_status!=IN_WORD)
			{
				buff[l]='\0';
				l=0;
				tokenize(buff);
				ch++;
				continue;
			}
		}	
		buff[l++] = *ch;
		ch++;
	
	}


	buff[l++]='\0';
	tokenize(buff);
	execute();
	
	return 0;
}


void searchHistory(char mid[],char arr[])
{
	int i=0;

	for(i=history_cnt-1;i>=0;i--)
	{
		char *ch = arr;
		char *ch1 = history_cmds[i];
		if(*ch==*ch1)
		{
			while(*ch!='\0' && *ch1!='\0' && *ch==*ch1)
			{
				ch++;ch1++;
			}

			if(*ch=='\0')
			{
				strcpy(mid,history_cmds[i]);
				break;
			}

		}
	}

	if(i<0)
	{
		strcpy(mid,"-1");
	}
}


void getLineFromHistory(char arr[],int num)
{
//	printf("line from history %d\n",num );
	if(num>history_cnt || num < 0)
	{
		strcpy(arr,"-1");
	}else
	{
		strcpy(arr,history_cmds[num]);
	}
	
}

void loadHistory()
{
	
	FILE *fp = fopen(historyFile, "a+");
	char buff[256];
	int len=0,j=0;history_cnt=0;

	while (fgets(buff,sizeof(buff),fp) != NULL ) 
	{
			len = strlen(buff);
	  		buff[len-1]='\0';
			strcpy(history_cmds[history_cnt++],buff);	
    }

	/*
	while ( fgets(buff,sizeof(buff),fp) != NULL ) 
	{
       num_lines++;
    }
    fclose(fp);
    fp = fopen(historyFile, "a+");
    if(num_lines<HISTORY_SIZE)
	{
		while (fgets(buff,sizeof(buff),fp) != NULL ) 
		{
       		len = strlen(buff);
     	  	buff[len-1]='\0';
       		strcpy(history_cmds[history_cnt++],buff);
	    }
	}else
	{
		while (fgets(buff,sizeof(buff),fp) != NULL ) 
		{
       		if(j>=(num_lines-HISTORY_SIZE))
       		{
       			len = strlen(buff);
     	  		buff[len-1]='\0';
       			strcpy(history_cmds[history_cnt++],buff);
       		}
       		j++; 		
	    }
	}*/

	fclose(fp);
   
}

void insertIntoHistory(char *arr)
{
	if(strcmp(history_cmds[history_cnt-1],arr)!=0)
	{
			//printf("%d history_cnt\n",history_cnt );
			if(history_cnt>=HISTORY_SIZE)
			{
				//printf("Its greater\n");
				int i;
				FILE * fp = fopen(historyFile, "w");
				fp = fopen(historyFile, "a+");

				for(i=1;i<history_cnt;i++)
				{
					fprintf(fp,"%s\n",history_cmds[i]);
				}
				fprintf(fp,"%s\n",arr);
				fclose(fp);

			}else
			{
					FILE * fp = fopen(historyFile, "a+");
					fprintf(fp,"%s\n",arr);
					fclose(fp);
			
			}
	}
	
}

void insertIntoHistoryMain(char *arr)
{
	sprintf(historyFile,"%s/my_history",getenv("HOME"));
	loadHistory();
	if(strcmp(history_cmds[history_cnt-1],arr)!=0)
	{
			//printf("%d history_cnt\n",history_cnt );
			if(history_cnt>=HISTORY_SIZE)
			{
				//printf("Its greater\n");
				int i;
				FILE * fp = fopen(historyFile, "w");
				fp = fopen(historyFile, "a+");

				for(i=1;i<history_cnt;i++)
				{
					fprintf(fp,"%s\n",history_cmds[i]);
				}
				fprintf(fp,"%s\n",arr);
				fclose(fp);

			}else
			{
					FILE * fp = fopen(historyFile, "a+");
					fprintf(fp,"%s\n",arr);
					fclose(fp);
			
			}
	}
	
}


void tokenize(char arr[])
{
	int i;
	struct Command *tempCommand;

	tempCommand=malloc(sizeof(struct Command));

	char *ch = arr;
	int cnt=0; //token count;
	int w_status=OUT_WORD,q_status=OUT_WORD;
	char buff[256];
	int l=0,j;

	tempCommand->isInRedirect=0;
	tempCommand->isOutRedirect=0;

	while(*ch!='\0')
	{
		//printf("%c\n",*ch );

		if(*ch=='"')
		{
			if(w_status==IN_WORD)
			{
				ch++;
				while(*ch!='"')
				{
					buff[l++]=*ch;
					ch++;
				}
			}else
			{
				l=0;
				ch++;
				while(*ch!='"')
				{
					buff[l++]=*ch;
					ch++;
				}
				buff[l]='\0';	
				strcpy(tempCommand->cmd_arr[cnt],buff);
				tempCommand->arr[cnt]=tempCommand->cmd_arr[cnt];
				cnt++;l=0;
			}
		
		}else if(*ch=='\'')
		{
			if(w_status==IN_WORD)
			{
				ch++;
				while(*ch!='\'')
				{
					buff[l++]=*ch;
					ch++;
				}
			}else
			{
				l=0;
				ch++;
				while(*ch!='\'')
				{
					buff[l++]=*ch;
					ch++;
				}
				buff[l]='\0';	
						
				strcpy(tempCommand->cmd_arr[cnt],buff);
				tempCommand->arr[cnt]=tempCommand->cmd_arr[cnt];
				l=0;cnt++;
			}
		
		}else if(*ch==' ' || *ch=='\t')
		{
			
			if(w_status==IN_WORD)
			{
				w_status=OUT_WORD;
				buff[l]='\0';
				strcpy(tempCommand->cmd_arr[cnt],buff);
				tempCommand->arr[cnt]=tempCommand->cmd_arr[cnt];
				l=0;cnt++;
			}
		}else if(*ch=='<')
		{
			
			if(w_status==IN_WORD)
			{
				w_status=OUT_WORD;
				buff[l]='\0';
				strcpy(tempCommand->cmd_arr[cnt],buff);
				tempCommand->arr[cnt]=tempCommand->cmd_arr[cnt];
				l=0;cnt++;
			}

			tempCommand->isInRedirect = 1;

			ch++;
			while(*ch==' ' || *ch=='\t')
			{
				ch++;
			}
			l=0;
			while(*ch!=' ' && *ch!='>' && *ch!='\0')
			{
				buff[l++]=*ch;
				ch++;
			}
			buff[l]='\0';
			strcpy(tempCommand->inFilename,buff);
			ch--;

		}else if(*ch=='>')
		{
			if(w_status==IN_WORD)
			{
				w_status=OUT_WORD;
				buff[l]='\0';
				strcpy(tempCommand->cmd_arr[cnt],buff);

				tempCommand->arr[cnt]=tempCommand->cmd_arr[cnt];
				l=0;cnt++;
			}

			tempCommand->isOutRedirect = 1;
			ch++;

			while(*ch==' ' || *ch=='\t')
			{
				ch++;
			}
			l=0;
			while(*ch!=' ' && *ch!='<' && *ch!='\0')
			{
				buff[l++]=*ch;
				ch++;
			}
			buff[l]='\0';
			strcpy(tempCommand->outFilename,buff);
			ch--;
		}
		else
		{
			w_status = IN_WORD;
			buff[l++]=*ch;
		}

		ch++;
	}

	if(w_status==IN_WORD)
	{
			buff[l]='\0';
			strcpy(tempCommand->cmd_arr[cnt],buff);
			tempCommand->arr[cnt]=tempCommand->cmd_arr[cnt];
			l=0;cnt++;
	}

	// printf("tempcommand values\n");
	// printf("%d\n",tempCommand->isInRedirect);
	// printf("%d\n",tempCommand->isOutRedirect);
	// printf("%s\n",tempCommand->inFilename);
	// printf("%s\n",tempCommand->outFilename);

	// for(j=0;j<cnt;j++)
	// {
	// 	printf("****%s\n",tempCommand->arr[j]);
	// }
	//printf("Last is *************%s************\n",tempCommand->arr[cnt-1]);
	tempCommand->arr[cnt]=NULL;

	commands[command_cnt++]=*tempCommand;

	
}


void execute()
{

	int i,pid;
	int inFile,outFile;
	int saved_stdout = dup(1);
	
	for(i=0;i<command_cnt;i++)
	{
		int fd[2];
		if(pipe(fd)<0)
		{
			printf("Failed to create pipe\n");
		}


		if(i==command_cnt-1)
		{
			if(strcmp(commands[i].arr[0],"cd")==0 && cntPipe == 0)
			{
				changeDir(commands[i].arr[1]);
				break;
			}

			if(commands[i].isInRedirect)
			{
				inFile = open(commands[i].inFilename,O_RDWR);
				if(inFile<0)
				{
					printf("%s : No such file or directory\n",commands[i].inFilename);
					exit(0);
				}
				dup2(inFile,0);
	 
			}

			if(commands[i].isOutRedirect)
			{
				outFile = open(commands[i].outFilename,O_CREAT|O_WRONLY|O_TRUNC,0666);
				dup2(outFile,1);
			}

			if(strcmp(commands[i].arr[0],"echo")==0)
			{
				echo(commands[i].arr);

			}else if(strcmp(commands[i].arr[0],"pwd")==0)
			{
				getPwd(commands[i].arr[0]);

			}else if(strcmp(commands[i].arr[0],"history")==0)
			{
				history(commands[i].arr);

			}else
			{
				if(execvp(commands[i].arr[0],commands[i].arr)<0)
				{
					
					printf("My_Shell: %s: command not found\n", commands[i].arr[0]);
				}
	
			}

			close(inFile);
			close(outFile);
			break;
		}


		pid = fork();

		if(pid==0)
		{
			
			if(commands[i].isInRedirect)
			{
				inFile = open(commands[i].inFilename,O_RDWR);
				if(inFile<0)
				{
					printf("%s : No such file or directory\n",commands[i].inFilename);
					exit(0);
				}
				dup2(inFile,0);
	 
			}
			
			if(commands[i].isOutRedirect)
			{
				outFile = open(commands[i].outFilename,O_CREAT|O_WRONLY|O_TRUNC,0666);
				dup2(outFile,1);
			}else
			{
				dup2(fd[1],1);   //stdin now points to read end of pipe
			}
			
			close(fd[0]);    //close the write end

			if(strcmp(commands[i].arr[0],"echo")==0)
			{
				echo(commands[i].arr);

			}else if(strcmp(commands[i].arr[0],"pwd")==0)
			{
				getPwd(commands[i].arr[0]);

			}else if(strcmp(commands[i].arr[0],"history")==0)
			{
				history(commands[i].arr);

			}
			else
			{
				if(execvp(commands[i].arr[0],commands[i].arr)<0)
				{
					dup2(saved_stdout, 1);
					close(saved_stdout);
					printf("My_Shell: %s: command not found\n", commands[i].arr[0]);
					exit(0);
				}
			}
			close(inFile);
			close(outFile);
			exit(0);

		}else
		{
			
			wait(0);
			dup2(fd[0],0);  //redirected stdin to read end of pipe
			close(fd[1]);   //closed the write end
		}
	}
	
}


void echo(char *arr[])
{
	int i=1;
	char *ch;
	while(arr[i]!=NULL)
	{
		ch=arr[i];
		if(*ch=='$')
		{
			ch++;
			if(getenv(ch))
			{
				printf("%s ",getenv(ch));
			}
			
		}else
		{
			printf("%s ",arr[i]);
		}
		i++;
	}
	printf("\n");
}

void getPwd(char *arr)
{
	printf("%s\n",getcwd(arr,100));
}

void history(char *arr[])
{
	int i=1,j;
	int param;
	while(arr[i]!=NULL)
	{
		i++;
	}

	if(i==1)
	{
		for(j=0;j<history_cnt;j++)
		{
			printf(" %-5d%s\n",j+1,history_cmds[j]);
		}

	}else if(i==2)
	{
		param=atoi(arr[1]);
		for(j=0;j<history_cnt;j++)
		{
			if(j>=history_cnt-param)
			{
				printf(" %-5d%s\n",j+1,history_cmds[j]);
			}

		}
	}
}
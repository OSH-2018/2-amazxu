#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>

int dic(char *args[]){  //为管道的查询做准备
    int i;
    extern char **environ;              //环境变量指针
    char **env = environ;
    char temp1[100];
    char temp2[100];

    /* 没有输入命令 */
    if (!args[0])
        return 0;

    /* 内建命令 */
    if (strcmp(args[0], "cd") == 0) {
        if (args[1])
            if (chdir(args[1])== -1)
                perror("chdir");                //失败返回 -1
        return 0;
    }
    if (strcmp(args[0], "pwd") == 0) {
        char wd[4096];
        puts(getcwd(wd, 4096));
        return 0;
    }
    if (strcmp(args[0], "exit") == 0)
        return -1;

    if (strcmp(args[0], "env") == 0) {
        while(*env){
            printf("%s\n",*env);
            env++;
        }
        return 0;
    }

    if (strcmp(args[0], "export") == 0){
        if(args[1] == '\0'){               //失败
            printf("please input the right variable\n");               //打印提示信息
        }
        else{
            strcpy(temp1,args[1]);
            for(i=0;args[1][i]!='=';i++);
            temp1[i]='\0';
            if(getenv(temp1)==NULL){                //如果系统中没有这个变量名
                printf("please input the right variable\n");
                return 0;
            }
            else{
            strcpy(temp2,args[1]+i+1);
            setenv(temp1,temp2,1);
            return 0;
            }
        }
        return 0;
    }


    /* 外部命令 */
    pid_t pid = fork();
    if (pid == 0) {
        /* 子进程 */
        execvp(args[0], args);
        /* execvp失败 */
        return 255;
    }
    /* 父进程 */
    wait(NULL);

}

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    while (1) {

        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);

        /* 清理结尾的换行符，换成‘\0’*/
        int i;
        for (i = 0; cmd[i] != '\n'; i++);
        cmd[i] = '\0';

        /* 拆解命令行 */
        args[0] = cmd;
        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    *args[i+1] = '\0';
                    args[i+1]++;
                    break;
                }
        args[i] = NULL;



        /* 管道 */
        int pipenum=0;
        for(i=0;args[i]!=NULL;i++)
            if (strcmp(args[i], "|") == 0){
                pipenum++;
                args[i]=NULL;
            }

        if(pipenum==0) {
            int m;
            m=dic(args);

            //ls
            //  wait(NULL);
            if(m==-1)
                break;
            else
                continue;
        }

        int pipecopy[2];
        pipecopy[0]=dup(STDIN_FILENO);
        pipecopy[1]=dup(STDOUT_FILENO);

        //n个管道
        int pipetemp[2];

        for(i=0;i<pipenum;i++) {
            pid_t fpid;
            //申请管道失败
            if (pipe(pipetemp) == -1) {
                perror("pipe");
                exit(-1);
            }
            fpid = fork();
            //申请进程失败
            if (fpid == -1) {
                perror("fork");
                exit(-1);
            }
            //子进程
            if(fpid==0){
                close(pipetemp[0]);
                dup2(pipetemp[1],STDOUT_FILENO);
                int x;
                x=dic(args+2*i);

                if(x==-1)
                    break;
                close(pipetemp[0]);
                _exit(0);
            }

                close(pipetemp[1]);
                dup2(pipetemp[0],STDIN_FILENO);
                int y;
                y=dic(args+2*pipenum);
                if(y==-1)
                    break;
                pipenum--;
                close(pipetemp[0]);         //把管道全部关闭
                wait(NULL);
            }
         //   waitpid(fpid, NULL, 0);

        dup2(pipecopy[0],STDIN_FILENO);
        dup2(pipecopy[1],STDOUT_FILENO);





    }








    }

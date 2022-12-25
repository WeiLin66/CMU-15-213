# Shell Lab實驗紀錄

## 簡介

本實驗要求我們編寫一個供使用者交互的終端介面，類似於Linux中的shell終端。實驗中我們需要先解析輸入指令(這些指令可能包含多個參數)，且指令必須支持進程背景執行。

另一個重點是該實驗模擬Unix系統中的作業控制(Job Control)，對創建的進程進行監控與管理(增、刪、改、查)。同時為了避免交互過程產生殭屍進程(Zombie)，我們需要編寫特殊的信號處理函式，並且該終端也要可以被Ctrl + C，Ctrl + Z退出程序和暫停程序。

細節部分可以參考資料夾中的`tshlab.pdf`中的`The tsh Specification`；程式編寫環節方面可以參考資料夾中的`shlab.pdf`整份文件

## 主結構

### 主程式架構

![image-20221225204937776](https://raw.githubusercontent.com/WeiLin66/pictures/main/202212252157371.png)



## 目標編寫函式

本次實驗需要編寫7個函式，分別是:

1. `eval`: 解析使用者輸入的指令
2. `builtin_cmd`: 執行原生的指令
   1. `quit`: 直接結束shell
   2. `jobs`: 列出當前執行中和中止的背景進程
   3. `bg`: 將中止的背景進程變成運行中的背景進程
   4. `fg`: 將中止的背景進程轉換成運行中的前臺進程
3. `do_bgfg`: 實現`bg`, `fg`指令
4. `waitfg`: 等待前臺進程結束
5. `sigchld_handler`: `SIGCHLD`的信號處理函式
6. `sigint_handler`: `SIGINT`的信號處理函式
7. `sigstp_handler`: `SIGTSTOP`的信號處理函式

> 該實驗的作業控制就是存放進程的PID, JID(作業編號),進程狀態，在編寫函式過程中要考慮到操作是否會變動的進程狀態，進而需要調用lab提供的Helper函式，例如`addjob()`

### eval

編寫`eval`的重點在創建子進程部分，也就是`fork`後使用`execve`加載指令bin檔這部分。由於`fork`指令的特性，我們無法保證父進程部分程式碼會先執行，有可能在添加作業前，`SIGCHLD`就已經抵達，所以在`fork`前要先對`SIGCHLD`進行屏蔽

```c
void eval(char *cmdline) {

    /* error handler */
    if(cmdline == NULL){

        return;
    }

    char *argv[MAXARGS] = {0};
    char buffer[MAXLINE] = {0};
    int bg;
    pid_t pid;
    sigset_t mask_all, mask_one, prev_one;

    /* 創建遮罩用來阻擋信號SIGCHLD */
    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);

    strcpy(buffer, cmdline);
    
    /* 判斷該指令是否為後臺進程 */
    bg = parseline(buffer, argv);

    /* if command is a empty command, then return */
    if(argv[0] == NULL){

        return;
    }

    /* 是否為built-in command */
    if(!builtin_cmd(argv)){

        /* 為了避免race condition，確保邏輯是先添加進程再觸發SIGCHLD */
        Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);

        /* fork a child process */
        if((pid = Fork()) == 0){

            /* 子進程會繼承父進程的信號向量表，所以需要unblock SIGCHLD，避免加載的進程無法觸發SIGCHLD */
            Sigprocmask(SIG_SETMASK, &prev_one, NULL); // unblock SIGCHLD

            setpgid(0, getpid()); // set process group id 

            if(execve(argv[0], argv, environ) < 0){

                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        /* 該遮罩確保添加作業過程不會被其他信號中斷 */
        Sigprocmask(SIG_BLOCK, &mask_all, NULL); // block all signals
        addjob(jobs, pid, (bg ? BG : FG), buffer); // add job
        /* 回復成原先狀態，允許接收SIGCHLD信號 */
        Sigprocmask(SIG_SETMASK, &prev_one, NULL); // unblock all signals

        /* 前臺進程需要等待子進程結束 */
        if(!bg){

            waitfg(pid);
        }
        /* 若為後臺進程則打印進程相關信息後退出 */
        else{

            #if (DEBUG_LOG)
            
            int i=1;
            strcpy(debug_log, argv[0]);

            while(argv[i]){

                strcat(debug_log, " ");
                strcat(debug_log, argv[i++]);
            }

            printf("[%d] (%d) %s &\n", getjobpid(jobs, pid)->jid, pid, debug_log);
            #endif
        }
    }

    return;
}
```

### builtin_cmd

`builtin_cmd`較為簡單，只需要依照相應的指令執行就可以了，重點在於`bg`, `fg`這兩個指令。若為built-in command則反為1，否則返回0

```c
int builtin_cmd(char **argv) {

    int ret = 1;

    /* 退出程序 */
    if(!strcmp(argv[0], "quit")){

        exit(0);
    }
    /* 列出後臺進程信息 */
    else if(!strcmp(argv[0], "jobs")){

        listjobs(jobs);
        // listbgjobs(jobs); // list only back ground job
    }
    /* restarts job by sending it a SIGCONT signal, and then runs it in the
       background. The job argument can be either a PID or a JID. */
    else if(!strcmp(argv[0], "bg")){

        do_bgfg(argv);
    }
    /* restarts job by sending it a SIGCONT signal, and then runs it in the
       foreground. The job argument can be either a PID or a JID. */
    else if(!strcmp(argv[0], "fg")){

        do_bgfg(argv);
    }
     /* not a builtin command (include invalid syntax) */
    else{

        ret = 0;
    }

    return ret;    
}

```

### do_bgfg

該函式是針對中止狀態的背景進程的操作。首先需要發送信號`SIGCONT`將進程狀態更改成運行中，若運行後的進程是前臺進程就等待，反之就退出就好了(記得要調用作業控制函式去修改全局作業變數的狀態!)

`fg`, `bg`後面可以接上PID, 或`JID`(作業編號)，例如`fg 1000`，若是使用作業編號要加上特殊符號%，例如`fg %1`

```c
void do_bgfg(char **argv) {

    /* error handler */
    if(argv == NULL){

        return;
    }

    /* 判斷是執行fg還是bg */
    u_int8_t fgbg = 1; // define 0 for front ground; 1 for back ground
    fgbg = strcmp(argv[0], "bg") == 0;

    if(argv[1] == NULL){
            
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return;
    }

    int target_jid;
    pid_t target_pid;
    struct job_t* job_ptr;

    /* 參數是針對jid */
    if(argv[1][0] == '%'){

        target_jid = atoi(argv[1]+1);

        if(!target_jid){
            
            printf("%s: argument must be a PID or %%jobid\n", argv[0]);
            return;
        }

        if((job_ptr = getjobJid(jobs, target_jid)) == NULL){

            printf("%%%d: No such job\n", target_jid);
            return;
        }
    }
    /* 參數是針對pid */
    else{

        target_pid = atoi(argv[1]);

        if(!target_pid){
            
            printf("%s: argument must be a PID or %%jobid\n", argv[0]);
            return;
        }

        if((job_ptr = getjobpid(jobs, target_pid)) == NULL){

            printf("(%d): : No such process\n", target_pid);
            return;
        }
    }

    /* 發送信號給指定進程 */
    Kill(-(job_ptr->pid), SIGCONT); // Continue if stopped

    switch (fgbg){

        /* 若為前臺進程，則需要等待進程結束 */
        case FRONT_GROUND:
            job_ptr->state = FG;
            waitfg(job_ptr->pid);
        break;

        /* 後臺進程打印信息後退出即可 */
        case BACK_GROUND:
            job_ptr->state = BG;

            #if (DEBUG_LOG)
                printf("[%d] (%d) %s", job_ptr->jid, job_ptr->pid, job_ptr->cmdline);
            #endif
        break;

        case UNKNOWED:
            #if (DEBUG_LOG)
                printf("fatal errror! fgbg is neither 0 nor 1\n");
            #endif
            exit(1);
        break;
    }
}
```

### waitfg

用來等待前臺進程結束，循環等待子進程結束，然後觸發`SIGCHLD`信號處理函式後回收子進程

```c
void waitfg(pid_t pid){

    if(pid <= 0){
        
        return;
    }

    struct job_t* fg = getjobpid(jobs, pid);

    if(fg == NULL){

        return;
    }

    sigset_t mask;

    /* 阻擋除了SIGCHLD以外的所有信號 */
    sigfillset(&mask);
    sigdelset(&mask, SIGCHLD);
    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGTSTP);

    /* 等待SIGCHLD信號 */
    for(; fgpid(fg) != 0; ){

        Sigsuspend(&mask); // suspend until SIGCHLD arrives
    }
}
```

### sigchld_handler

回收前後臺進程並打印子進程結束資訊，注意該函式不會阻塞等待子進程。為了確保執行信號處理函式過程中不被其他信號打斷，可以先行阻擋其他信號

```c
void sigchld_handler(int sig) {

    sigset_t mask, prev;

    int stat, olderr = errno; // save original errno
    pid_t pid;

    sigfillset(&mask);

    /* 阻擋其他信號 */
    Sigprocmask(SIG_BLOCK, &mask, &prev);

    while((pid = waitpid(-1, &stat, WNOHANG | WUNTRACED)) > 0){

        /* 進程正常退出 */
        if(WIFEXITED(stat)){

            #if (DEBUG_LOG)
            // sprintf(debug_log, "child process [pid: %d] is terminated normally\n", pid);
            // Sio_print(debug_log);
            #endif

            /* 刪除作業 */
            deletejob(jobs, pid);
        }
        /* 子進程被信號中止 */
        else if(WIFSIGNALED(stat)){

            #if (DEBUG_LOG)
            sprintf(debug_log, "Job [%d] (%d) terminated by signal %d\n", getjobpid(jobs, pid)->jid, pid, WTERMSIG(stat));
            Sio_print(debug_log);
            #endif

            /* 刪除作業 */
            deletejob(jobs, pid);
        }
        /* 子進程被信號中止 */
        else if(WIFSTOPPED(stat)){

            #if (DEBUG_LOG)
            sprintf(debug_log, "Job [%d] (%d) stopped by signal %d\n", 
                       getjobpid(jobs, pid)->jid, pid, WSTOPSIG(stat));
            Sio_print(debug_log);
            #endif

            struct job_t* target = getjobpid(jobs, pid);
            target->state = ST;
        }
        /* 中止的子進程收到信號SIGCONT */
        else if(WIFCONTINUED(stat)){

            #if (DEBUG_LOG)
            sprintf(debug_log, "child JID: %d, PID: %d is resumed by signal SIGCONT\n", 
                       pid2jid(pid), pid);
            Sio_print(debug_log);
            #endif
        }
    }

    /* 解鎖 */
    Sigprocmask(SIG_SETMASK, &prev, NULL);

    errno = olderr; // restore erron
    
    return;
}
```

### sigint_handler

ctrl + c的信號處理函式，直接發送一個`SIGINT`信號給前臺進程組為`pid`的所有進程

```c
void sigint_handler(int sig) {

    pid_t pid = fgpid(jobs);

    if(pid == 0){

        Sio_print("No front ground job!\n");
        return ;
    }

    Kill(-pid, SIGINT); // close front ground process group 

    return;
}
```

### sigstp_handler

ctrl + z的信號處理函式，直接發送一個`SIGTSTP`信號給前臺進程組為`pid`的所有進程

```c
void sigtstp_handler(int sig) {

    pid_t pid = fgpid(jobs);

    if(pid == 0){

        Sio_print("No front ground job!\n");
        return ;
    }

    Kill(-pid, SIGTSTP); // stop front ground process group 

    return;
}
```

## 如何進行驗證

在檔案資料夾中可以發現trace01~16文件，該文件可以自動化測試指令。在編寫完上述函式後首先可以先自行測試編譯完成後的`tsh`檔是否符合預期，簡單的執行`./tsh`進行調適會比直接使用trace文件測試來的方便，也較好debug

使用`make`編譯整個文件後會生成`tsh`和`tshref`兩個可執行檔，我們可以使用指令`./sdriver.pl -t traceXX.txt -s ./tsh -a "-p"`來進行驗證，例如:

```shell
./sdriver.pl -t trace08.txt -s ./tsh -a "-p"
./sdriver.pl -t trace08.txt -s ./tshref -a "-p"
```

tsh的執行結果要和tshref一模一樣(除了進程PID以外)

![image-20221225215923027](https://raw.githubusercontent.com/WeiLin66/pictures/main/202212252200063.png)






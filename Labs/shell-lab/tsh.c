/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024         /* max line size */
#define MAXARGS    128          /* max args on a command line */
#define MAXJOBS    16           /* max jobs at any point in time */
#define MAXJID     1 << 16      /* max job ID */

#define DEBUG_ON    1
#define DEBUG_OFF   0
#define DEBUG_LOG   DEBUG_OFF

/* Job states */
#define UNDEF   0    /* undefined */
#define FG      1    /* running in foreground */
#define BG      2    /* running in background */
#define ST      3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

typedef enum{
    FRONT_GROUND = 0,
    BACK_GROUND = 1,
    UNKNOWED = 2
}Job_Status;

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};

struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listbgjobs(struct job_t* jobs); // add by Zach
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) {

    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {

        switch (c) {

            case 'h':             /* print help message */
                usage();
    	    break;

            case 'v':             /* emit additional diagnostic info */
                verbose = 1;
    	    break;

            case 'p':             /* don't print a prompt */
                emit_prompt = 0;  /* handy for automatic testing */
    	    break;

    	    default:
                usage();
	   }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

    	/* Read command line */
    	if (emit_prompt) {

    	    printf("%s", prompt);
    	    fflush(stdout);
    	}

    	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)){

    	    app_error("fgets error");
        }

    	if (feof(stdin)) { /* End of file (ctrl-d) */

    	    fflush(stdout);
    	    exit(0);
    	}

    	/* Evaluate the command line */
    	eval(cmdline);
    	fflush(stdout);
    	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) {

    if(cmdline == NULL){

        return;
    }

    char *argv[MAXARGS] = {0};
    char buffer[MAXLINE] = {0};
    int bg;
    pid_t pid;
    sigset_t mask_all, mask_one, prev_one;

    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);

    strncpy(buffer, cmdline, strlen(cmdline));
    bg = parseline(buffer, argv);

    /* empty command */
    if(argv[0] == NULL){

        return;
    }

    if(!builtin_cmd(argv)){

        /* avoid race condition here */
        Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);

        if((pid = Fork()) == 0){

            Sigprocmask(SIG_SETMASK, &prev_one); // unblock SIGCHLD

            if(execve(argv[0], argv, environ) < 0){

                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        /* add job */
        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        addjob(jobs, pid, (bg ? BG : FG), buffer);
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);

        /* wait for front ground job */
        if(!bg){

            int status;

            if(Waitpid(pid, &status, 0) < 0){

                unix_error("waitfg: waitpid error");
            }else{

                printf("%d %s", pid, cmdline);
            }
        }
    }

    return;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) {

    if(cmdline == NULL || argv == NULL){

        return;
    }

    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */

    while (*buf && (*buf == ' ')){ /* ignore leading spaces */
    
    	buf++;
    }

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {

        buf++;
        delim = strchr(buf, '\'');
    }else {

        delim = strchr(buf, ' ');
    }

    while (delim) {

        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;

        while (*buf && (*buf == ' ')){ /* ignore spaces */

            buf++;
        }

        if (*buf == '\'') {
            
            buf++;
            delim = strchr(buf, '\'');
        }else {
            
            delim = strchr(buf, ' ');
        }
    }

    argv[argc] = NULL;
    
    if (argc == 0){  /* ignore blank line */
	
        return 1;
    }

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	   
       argv[--argc] = NULL;
    }

    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute it immediately.
 */
int builtin_cmd(char **argv) {

    if(argv == NULL){

        return;
    }

    int ret = 1;

    /* quit the shell process */
    if(!strcmp(argv[0], "quit")){

        exit(0);
    }
    /* lists all background jobs */
    else if(!strcmp(argv[0], "jobs")){

        listbgjobs(jobs); // list only back ground job
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

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 *  bg/fg %5 denotes jid 5, wheara bg/fg 5 denotes pid
 */
void do_bgfg(char **argv) {

    if(argv == NULL){

        return;
    }

    if(argv[1] == NULL){

        return;
    }

    int target_jid;
    pid_t target_pid;
    u_int8_t fgbg = 1; // define 0 for front ground; 1 for back ground
    struct job_t* job_ptr;

    fgbg = strcmp(argv[0], "bg") == 0;

    /* argument is jid */
    if(argv[1][0] == '%'){

        target_jid = atoi(argv[1]);
        job_ptr = getjobjid(jobs, target_jid);
    }
    /* argument is pid */
    else{

        target_pid = atoi(argv[1]);
        job_ptr = getjobpid(jobs, target_pid);
    }

    if(job_ptr != NULL && job_ptr->state == ST){

        #if (DEBUG_LOG)
            printf("process jid: %u is currently in ST mode!\n");
        #endif

        Kill(-(job_ptr->pid), SIGCONT); // Continue if stopped

        switch (fgbg){

            /* turn into fg and conduct a Wait() */
            case FRONT_GROUND:
                job_ptr->state = FG;
                waitfg(job_ptr->pid);
            break;

            /* turn into bg */
            case BACK_GROUND:
                job_ptr->state = BG;
            break;

            case UNKNOWED:
                #if (DEBUG_LOG)
                    printf("fatal errror! fgbg is neither 0 nor 1\n");
                #endif
                exit(1);
            break;
        }
    }else{

        #if (DEBUG_LOG)
            printf("process jid: %u is not found or is not in the ST mode!\n");
        #endif
    }

}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid){

    if(pid <= 0){
        
        return;
    }

    struct job_t* fg = getjobpid(jobs, pid);

    if(fg == NULL){

        return;
    }

    /* wait until current job pid is not a front ground job anymore */
    for(; fgpid(fg) != 0; ){

        sleep(1000); // wait for SIGCHLD
    }
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) {

    sigset_t mask, prev;

    int stat, olderr = errno; // save original errno

    pid_t pid;

    sigfillset(&mask);

    /* prevent handler being interrupted by other signal */
    Sigprocmask(SIG_BLOCK, &mask, &prev);

    /**
     * @brief waitpid option could be
     * 0: block waiting
     * WNOHANG: return immediately if no child has exited.
     * WUNTRACED: return if a child has stopped
     * WCONTINUED: return if a stopped child has been resumed by delivery of SIGCONT
     */
    /* wait for all zombies */
    while((pid = Waitpid(-1, &stat, WNOHANG | WUNTRACED)) > 0){

        if(WIFEXITED(stat)){

            #if (DEBUG_LOG)
            Sio_error("child %lu is terminated normally\n", pid);
            #endif

            /* delete child process */
            deletejob(jobs, pid);
        }else if(WIFSIGNALED(stat)){

            #if (DEBUG_LOG)
            Sio_error("child %lu is terminated by signal\n", pid);
            #endif

            /* child proces is terminated by signal */
            deletejob(jobs, pid);
        }else if(WIFSTOPPED(stat)){

            #if (DEBUG_LOG)
            Sio_error("child JID: %lu, PID: %lu is stopped by signal %lu\n", 
                       pid2jid(pid), pid, WSTOPSIG(stat));
            #endif

            struct job_t* target = getjobpid(jobs, pid);
            target->state = ST;
        }else if(WIFCONTINUED(stat)){

            #if (DEBUG_LOG)
            Sio_error("child JID: %lu, PID: %lu is resumed by signal SIGCONT\n", 
                       pid2jid(pid), pid);
            #endif
        }
    }

    /* unblock all signals */
    Sigprocmask(SIG_SETMASK, &prev, NULL);

    /* quit waitpid successfully? */
    if(errno != ECHILD){

        Sio_error("waitpid error\n");
    }

    errno = olderr; // restore erron
    
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) {

    pid_t pid = fgpid(jobs);

    if(pid == 0){

        return ;
    }

    Kill(-pid, SIGINT); // close front ground process group 

    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) {

    pid_t pid = fgpid(jobs);

    if(pid == 0){

        return ;
    }

    Kill(-pid, SIGSTOP); // close front ground process group 

    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    
    int i;

    for (i = 0; i < MAXJOBS; i++){

        clearjob(&jobs[i]);    
    }
	
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) {

    int i, max=0;

    for (i = 0; i < MAXJOBS; i++){
    
        if (jobs[i].jid > max){
         
            max = jobs[i].jid;
        }
    }
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) {

    int i;
    
    if (pid < 1){
	   
       return 0;
    }

    for (i = 0; i < MAXJOBS; i++) {

    	if (jobs[i].pid == 0) {
    	    
            jobs[i].pid = pid;
    	    jobs[i].state = state;
    	    jobs[i].jid = nextjid++;
    	    
            if (nextjid > MAXJOBS){
    		  
              nextjid = 1;
            }

    	    strcpy(jobs[i].cmdline, cmdline);
      	    
            if(verbose){
    	    
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }

            return 1;
    	}
    }

    printf("Tried to create too many jobs\n");
    
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) {

    int i;

    if (pid < 1){
	
        return 0;
    }

    for (i = 0; i < MAXJOBS; i++) {
    
    	if (jobs[i].pid == pid) {
    	    
            clearjob(&jobs[i]);
    	    nextjid = maxjid(jobs)+1;
    	    
            return 1;
    	}
    }

    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    
    int i;

    for (i = 0; i < MAXJOBS; i++){

    	if (jobs[i].state == FG){
    
    	    return jobs[i].pid;
        }
    }

    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    
    int i;

    if (pid < 1){
    
    	return NULL;
    }

    for (i = 0; i < MAXJOBS; i++){
    
    	if (jobs[i].pid == pid){

	       return &jobs[i];
        }
    }

    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobJid(struct job_t *jobs, int jid) {

    int i;

    if (jid < 1){
    
    	return NULL;
    }

    for (i = 0; i < MAXJOBS; i++){
    	
        if (jobs[i].jid == jid){

    	    return &jobs[i];
        }
    }

    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) {

    int i;

    if (pid < 1){
    
    	return 0;
    }

    for (i = 0; i < MAXJOBS; i++){
    	
        if (jobs[i].pid == pid) {

            return jobs[i].jid;
        }
    }

    return 0;
}

void listbgjobs(struct job_t* jobs){

    if(jobs == NULL){

        return;
    }

    int i;

    for(int i=0; i<MAXJOBS; i++){

        if(jobs[i].pid <= 0){

            continue;
        }

        if(jobs[i].state != BG){

            continue;
        }

       printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
    }
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) {

    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
    	
        if (jobs[i].pid != 0) {
    	    
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
    	    
            switch (jobs[i].state) {
        		
                case BG: 
        		    printf("Running ");
    		    break;

        		case FG: 
        		    printf("Foreground ");
    		    break;

        		case ST: 
        		    printf("Stopped ");
    		    break;

        	    default:
        		    printf("listjobs: Internal error: job[%d].state=%d ",i, jobs[i].state);
    	    }

    	    printf("%s", jobs[i].cmdline);
	    }
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) {

    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg){

    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg){

    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) {

    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0){
	
        unix_error("Signal error");
    }

    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) {
    
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}




/*
 *  Trade secret of  R&D.
 *  Copyright (c) 2010  R&D. (unpublished)
 *
 *  All rights reserved.  This notice is intended as a precaution against
 *  inadvertent publication and does not imply publication or any waiver
 *  of confidentiality.  The year included in the foregoing notice is the
 *  year of creation of the work.
 *
 */
#include <iostream>
#include<signal.h>
#include <cstdlib>
#include<unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/wait.h>
using namespace std;
int gArgc;
char ** gArgv;
int childPid;
int64_t start_daemon()
{
    int fd;
    switch(fork())
    {
    case -1:
        return -1;
    case 0:
        break;
    default:
        exit(0);
    }
    if(setsid() == -1)
        return -1;
    umask(0);

    fd = open("/dev/null",O_RDWR);
    if(fd == -1)
        return -1;
    if(dup2(fd, STDIN_FILENO) == -1)
        return -1;
    if(dup2(fd, STDERR_FILENO) == -1)
        return -1;
    if(dup2(fd, STDOUT_FILENO) == -1)
        return -1;

    if(fd > STDERR_FILENO)
    {
        if(close(fd) == -1)
            return -1;
    }
    return 0;
}
void HandleSigtermSignal(int sigNum, siginfo_t * info ,void * context)
{
}
void SigChildHandler (int sigNum, siginfo_t * info ,void * context)
{
    if(sigNum == SIGCHLD)
    {
        waitpid(childPid,NULL,0);
    }
}
void child_main()
{
    cout<<"child started"<<endl;
    while(true)
    {
        sleep(1);
    }
}
void do_child_process()
{
    sigset_t blockMask, origMask;
    sigemptyset(&blockMask);
    sigaddset(&blockMask,SIGCHLD);
    sigprocmask(SIG_BLOCK, & blockMask, & origMask);

    struct sigaction sigtermSig;
    sigtermSig.sa_sigaction = HandleSigtermSignal;
    sigtermSig.sa_flags = SA_SIGINFO;
    if(sigaction(SIGTERM, & sigtermSig, NULL) < 0)
    {
        exit(5);
    }
    child_main();
}
void start_child_process()
{
    childPid = fork();
    switch(childPid)
    {
    case 0:
        do_child_process();
        _exit(1);
    default:
        break;
    }
}
int daemon_process_cycle()
{
    sigset_t set;
    struct sigaction child_act;
    child_act.sa_sigaction = SigChildHandler;
    child_act.sa_flags = SA_SIGINFO;
    if(sigaction(SIGCHLD,&child_act,NULL) < 0)
        return -1;
    sigemptyset(&set);
    start_child_process();
    for(;;)
    {
        sigsuspend(&set);
        start_child_process();
    }
    return 0;
}
int main(int argc, char ** argv)
{
    gArgc = argc;
    gArgv = argv;

    if(start_daemon() == -1)
        exit(1);

    if(daemon_process_cycle() == -1)
        exit(2);
    return 0;
}

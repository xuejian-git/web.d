### 什么是 Linux 下的守护进程

Linux daemon 是运行于后台常驻内存的一种特殊进程，周期性的执行或者等待 trigger 执行某个任务，与用户交互断开，独立于控制终端。
一个守护进程的父进程是 init 进程，它是一个孤儿进程，没有控制终端，所以任何输出，无论是向标准输出设备 stdout 还是标准出错设备 stderr 的
输出都被丢到了 /dev/null 中。守护进程一般用作服务器进程，如 httpd，syslogd 等。

### 进程、进程组、会话、控制终端之间的关系

- 进程组：由一个或者多个进程组成，进程组号(GID)，就是这些进程中的进程组长的 PID

- 会话：又叫会话期，它包括了期间所有的进程组，一般一个会话开始于用户 login，一般 login 的是 shell 的终端，所以 shell 是此次
会话的shoul 首进程，会话一般结束于 logout，对于非进程组长，它可以调用 setid() 创建一个新的会话

- 控制终端：一般指 shell 的终端，它在会话期可有可没有

### 创建守护进程的过程

- 成为后台进程

用 fork 创建进程，父进程退出，子进程称为孤儿进程被 init 进程接管，子进程变为后台进程

- 脱离父进程的控制终端，登录会话和进程组

调用 setid() 让子进程称为成为新会话的组长，脱离父进程的会话期，setid() 在调用者是某进程组的组长时会失败，但是前提保证调用者不是组长即可
之后子进程变为新会话期的组长

- 进制进程重新开启控制终端

因为会话组的组长有权限重新打开控制终端，所以这里第二次 fork 将子进程结束，留着孙进程，孙进程不是会话组的组长所以没有权利再
打开控制终端，这样整个程序就与控制终端隔离了

- 关闭文件描述符

进程从创建它的父进程那里继承了打开的文件描述符。如不关闭，将会浪费系统资源，造成进程所在的文件系统无法卸下以及引起无法预料的错误

- 重定向 0，1，2 标准文件描述符

将三个标准文件描述符定向到 /dev/null 中

- 改变工作目录和文件掩码

进程活动时，其工作目录所在的文件系统不能卸下(比如工作目录在一个 NFS 中,运行一个 daemon 会导致 umount 无法成功)。一般需要将工作目录改变
到根目录。对于需要转储核心，写运行日志的进程将工作目录改变到特定目录如 chdir("/tmp")，进程从创建它的父进程那里继承
了文件创建掩模。它可能修改守护进程所创建的文件的存取位。为防止这一点，将文件创建掩模清除：umask(0);

*后三步可以先做，因为这些修改将会被子进程继承下来*

### 一个实例

```cpp
#define ERROR_EXIT(m)\
    do\
{\
    perror(m);\
    exit(EXIT_FAILURE);\
}\
while(0)

int main(int argc, char **argv)
{
    time_t t;
    int fd, i;
    mydaemon();
    fd = open("./mydaemon.log", O_RDWR|O_CREAT, 0644);
    if(fd < 0)
        ERROR_EXIT("open /mydaemon.log failed!");
    for(i=0; i<3; i++) {
        t = time(0);
        char *buf = asctime(localtime(&t));
        write(fd, buf, strlen(buf));
        sleep(10);
    }
    close(fd);
    return 0;                                
}

void mydaemon(int nochdir, int noclose) {
    pid_t pid;
    int fd, i, nfiles;
    struct rlimit rl;
    pid = fork();
    if(pid < 0)
        ERROR_EXIT("First fork failed!");
    if(pid > 0)
        exit(EXIT_SUCCESS);// father exit
    if(setsid() == -1)
        ERROR_EXIT("setsid failed!");
    pid = fork();
    if(pid < 0)
        ERROR_EXIT("Second fork failed!");
    if(pid > 0)// father exit
        exit(EXIT_SUCCESS);
#ifdef RLIMIT_NOFILE
    /* 关闭从父进程继承来的文件描述符 */
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
        ERROR_EXIT("getrlimit failed!");
    nfiles = rl.rlim_cur = rl.rlim_max;
    setrlimit(RLIMIT_NOFILE, &rl);
    for(i=3; i<nfiles; i++)
        close(i);
#endif
    /* 重定向标准的3个文件描述符 */
    if(!noclose) {
        if(fd = open("/dev/null", O_RDWR) < 0)
            ERROR_EXIT("open /dev/null failed!");
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if(fd > 2)
            close(fd);
    }
    /* 改变工作目录和文件掩码常量 */
    if(!nochdir)
        chdir("/");
    umask(0);                                                                
}
```

enum procinfo_state { PROC_UNUSED, PROC_USED, PROC_SLEEPING, PROC_RUNNABLE, PROC_RUNNING, PROC_ZOMBIE };

struct procinfo
{
    int pid;
    int ppid;
    enum procinfo_state state;
    char name[16];
    char pname[16];
};

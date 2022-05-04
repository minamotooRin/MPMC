#ifndef COMMON_H
#define COMMON_H

// For compile
typedef int PID;
typedef int UINT8;
// int ASEND(int i, UINT8* u, size_t s, PID p);

enum class TASK_RESULT
{
    SUCCESS = 0,
    FAILURE_STOP,
    FAILURE_MEMORY,
    FAILURE_TASK_NOT_FOUND,
    FAILURE_UNKNOW,

    TASK_RESULT_NUM
};

enum class PROCESS_RESULT
{
    SUCCESS = 0,
    FAILURE_MEMORY,
    FAILURE_UNKNOW,

    PROCESS_RESULT_NUM
};

#endif
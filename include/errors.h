#ifndef ERRORS_H
#define ERRORS_H

typedef struct SendErrorInfo SendErrorInfo;
typedef struct RecvErrorInfo RecvErrorInfo;
typedef struct PthreadMutexLockErrorInfo PthreadMutexLockErrorInfo;

typedef enum {
    ERROR_RECOVERABLE,
    ERROR_FATAL
} ErrorCategory;

struct SendErrorInfo{
    int error_code;
    ErrorCategory category;
};
struct PthreadMutexLockErrorInfo{
    int error_code;
    ErrorCategory category;

};

struct RecvErrorInfo{
    int error_code;
    ErrorCategory category;
};
void log_pthread_create_err(int err_code);
SendErrorInfo* categorize_send_error(int err_no);
RecvErrorInfo* categorize_recv_error(int err_no);
PthreadMutexLockErrorInfo* categorize_mtx_lck_error(int err_no);
#endif

#ifndef ERRORS_H


typedef struct SendErrorInfo SendErrorInfo;
typedef struct RecvErrorInfo RecvErrorInfo;

typedef enum {
    SEND_ERROR_RECOVERABLE,
    SEND_ERROR_FATAL
} SendErrorCategory;

struct SendErrorInfo{
    int error_code;
    SendErrorCategory category;
};

typedef enum {
    RECV_ERROR_RECOVERABLE,
    RECV_ERROR_FATAL
} RecvErrorCategory;

struct RecvErrorInfo{
    int error_code;
    RecvErrorCategory category;
};
void log_pthread_create_err(int err_code);
SendErrorInfo* categorize_send_error(int err_no);
RecvErrorInfo* categorize_recv_error(int err_no);
#endif

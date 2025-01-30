#ifndef ERRORS_H


typedef struct SendErrorInfo SendErrorInfo;

typedef enum {
    SEND_ERROR_RECOVERABLE,
    SEND_ERROR_FATAL
} SendErrorCategory;

struct SendErrorInfo{
    int error_code;
    SendErrorCategory category;
};

void log_pthread_create_err(int err_code);
SendErrorInfo* categorize_send_error(int err_no);
#endif

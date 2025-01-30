#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include "../include/errors.h"
#include "../include/util.h"

SendErrorInfo*
categorize_send_error(int err_no){
    SendErrorInfo* err = (SendErrorInfo*) calloc(1,sizeof(SendErrorInfo));
    err->error_code = err_no;
    switch (err_no){
        case EINTR:
        case ENOBUFS:
        case EMSGSIZE:
        case EWOULDBLOCK:
            err->category = SEND_ERROR_RECOVERABLE;
            break;
        case ECONNRESET:
        case EPIPE:
        case ENOTCONN:
        case EBADF:
        case ENOTSOCK:
        case EFAULT:
        case EINVAL:
            err->category = SEND_ERROR_FATAL;
            break;
        default:
            err->category = SEND_ERROR_FATAL;
            break;
        }
    return err;
}
void
log_pthread_create_err(int err_code){
    switch(err_code) {
        case EAGAIN:
            log_error("System lacks resources to create thread");
            break;
        case EINVAL:
            log_error("Invalid thread attributes.");
            break;
        case ENOMEM:
            log_error("Insufficient memory to create thread");
            break;
        default:
            log_error("Unknown error creating thread");
    }
}
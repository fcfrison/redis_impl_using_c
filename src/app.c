#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/protocol.h"
#include "../include/util.h"
#include "../include/cmd_handler.h"
#include "../include/server.h"
#include "../include/errors.h"
#include "../include/queue.h"
void*
app_code(void* arg){
    int* fd_ptr = (int*)arg;
    int  fd     = *fd_ptr;
    int  rtn_v;
    char buff, *rtn_s;

    ssize_t rtn;
    while(1){
        rtn = recv_resp_fm_clnt(&buff, 1, fd, 3, .500);
        if(rtn==-1){
            printf("Invalid data received: %s...\n", strerror(errno));
		    break;
        }
        if(!rtn){
            break;
        }
        switch (buff){
            case '*':
                ArrayNode* array = parse_array(fd);
                if(array){
                    //print_array(array,0);
                    rtn_s = parse_command(array);
                }
                if(array && rtn_s){
                    rtn_v = send_resp_to_clnt(rtn_s,strlen(rtn_s)+1, fd,3,500);
                    if(rtn_v==-1){
                        continue;
                    }
                    free(rtn_s);
                    delete_array(array,1);
                }
                break;
            
            default:
                break;
        }

	}
    printf("Closing fd: %d\n",fd);
    close(fd);
    return NULL;
}
int main(){
    start_server(&app_code, 10, 5, 100,10);
}

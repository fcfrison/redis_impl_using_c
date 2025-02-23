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
#include "../include/simple_map.h"
#include "../include/app.h"

SimpleMap* sm;
SimpleMap* config_dict;
void*
app_code(void* arg){
    int* fd_ptr = (int*)arg;
    int  fd     = *fd_ptr;
    int  rtn_v;
    char buff, *rtn_s, *msg;
    CmdParserArgs args = {sm, config_dict};
    ssize_t rtn;
    while(1){
        rtn_s = NULL;
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
                ArrayNode* array = lexer(fd);
                if(!array){
                   msg = "An error occured while parsing the command.";
                    send_resp_to_clnt(msg,strlen(msg)+1, fd,3,500);
                    return NULL;
                }
                if(array){
                    print_array(array,0);
                    rtn_s = parse_command(array, &args);
                }
                if(!rtn_s){
                    msg = "An error occured while parsing the command.";
                    rtn_v = send_resp_to_clnt(msg,strlen(msg)+1, fd,3,500);
                    if(rtn_v==-1){
                        continue;
                    }
                    delete_array(array,1);
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
    return NULL;
}

int main(int argc, char** argv){
    setup(argc, argv, &sm, &config_dict);
    start_server(&app_code, 10, 500, 2, 0);
};
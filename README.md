# Redis implementation
https://build-your-own.org/redis/
https://app.codecrafters.io/courses/redis/

### VsCode debug config
https://dev.to/talhabalaj/setup-visual-studio-code-for-multi-file-c-projects-1jpi

### makefile tutorial
https://opensource.com/article/18/8/what-how-makefile

### socket error handling
https://thelinuxcode.com/catch-socket-errors-c/
https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-send-send-data-socket
#### errno is thread safe
https://stackoverflow.com/questions/1694164/is-errno-thread-safe

### multithreading in C lang
https://tutorial.codeswithpankaj.com/c-programming/thread

### Function pointers in C
https://www.geeksforgeeks.org/function-pointer-in-c/

### recv - set timeout
https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout

## Project design considerations
### SET command parser
The SET command validation accepts requests that respect the syntax:
```
SET key value [NX | XX] [GET] [EX seconds | PX milliseconds | EXAT unix-time-seconds | PXAT unix-time-milliseconds | KEEPTTL] 
```
It's important to mention that the order of the options must be observed. Otherwise, the parser will consider it as invalid. The KEEPTTL option, at the moment, is not a valid one. It's also important to mention that the client must send the request as an array of bulkstrings (again, at least for the present moment). For instance:
```
SET anotherkey "will expire in a minute" EX 60
```
Is serialized as:
```
*4\r\n$3\r\nSET\r\n$9\r\nanotherkey\r\n$22\r\nwill expire in a minute\r\n$2\r\nEX\r\n$2\r\n60\r\n
```

#ifndef __dbg_h_
#define __dbg_h_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define RESET "\x1b[0m"
#define LIGHT_RED "\x1b[38;5;9m"
#define LIGHT_BLUE "\x1b[38;5;38m"
#define LIGHT_GREEN "\x1b[38;5;84m"
#define LIGHT_YELLOW "\x1b[38;5;192m"

#define clean_errno() (errno == 0 ? "None": strerror(errno))

#define debug(M, ...) fprintf(stdout, "%s[DEBUG]%s %s:%d: " M "\n", LIGHT_BLUE, RESET, __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stdout, "%s[INFO]%s %s:%d: " M "\n", LIGHT_GREEN, RESET, __FILE__, __LINE__, ##__VA_ARGS__) 
#define log_warn(M, ...) fprintf(stderr, "%s[WARN]%s %s:%d:  error: %s " M "\n", LIGHT_YELLOW, RESET, __FILE__, __LINE__, clean_errno() ##__VA_ARGS__)
#define log_err(M, ...) fprintf(stderr, "%s[ERROR]%s %s:%d: error: %s " M "\n", LIGHT_RED, RESET, __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); }
#define check_mem(A) check((A), "Out of memory!")

#endif //__dbg_h_
 

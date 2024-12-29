#ifndef __conn_h_
#define __conn_h_

#include <time.h>

struct Connection;

void init_database(struct Connection *conn, const char *filename);
void merge_database(struct Connection *conn);
void mem_to_disk_transfer(struct Connection *conn);
void set(struct Connection *conn, char *key, char *value, time_t duration);
char* get(struct Connection *conn, char *key);
void del(struct Connection *conn, char *key);

#endif //__conn_h_

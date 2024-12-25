#ifndef __conn_h_
#define __conn_h_

struct Connection;

void init_database(struct Connection *conn, const char *filename);
void merge_database(struct Connection *conn);
void mem_to_disk_transfer(struct Connection *conn);

#endif //__conn_h_

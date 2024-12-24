#ifndef __conn_h_
#define __conn_h_

#include <sqlite3.h>

struct Connection {
  sqlite3 *diskdb;
  sqlite3 *memdb;
};

void init_database(struct Connection *conn, const char *filename);
void merge_database(struct Connection *conn);

#endif //__conn_h_

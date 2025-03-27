#ifndef __conn_h_
#define __conn_h_

#include <time.h>
#include <stdbool.h>

struct Connection;

struct Values {
  const char* key;
  const char* value;
  const char* expires_on;
  const char* created_on;
};

void init_database(struct Connection *conn, const char *filename);
void merge_database(struct Connection *conn);
void mem_to_disk_transfer(struct Connection *conn);
void set(struct Connection *conn, char *key, char *value, time_t duration);
char* get(struct Connection *conn, char *key);
void del(struct Connection *conn, char *key);
void create_db_file(const char *filename);
void init_disk_table(struct Connection *conn);
bool file_exist(const char *filename);

#endif //__conn_h_

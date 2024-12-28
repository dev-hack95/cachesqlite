#include <stdio.h>
#include <stdlib.h>
#include "../include/dbg.h"
#include "../include/conn.h"
#include <sqlite3.h>
#include <string.h>
#include <time.h>

#define QUERY_MAX_SIZE 1024

struct Connection {
  sqlite3 *diskdb;
  sqlite3 *memdb;
  char *filename;
};


void init_database(struct Connection *conn,  const char *filename) {
  conn->filename = calloc(strlen(filename) + 1, sizeof(char));
  //log_info("%s", filename);
  strncpy(conn->filename, filename, strlen(filename) + 1);
  //log_info("%s", conn->filename);  
  int diskdb_status = sqlite3_open(filename, &conn->diskdb);
  if (diskdb_status != SQLITE_OK) {
    log_err("error occured at opening sql file");
    sqlite3_close(conn->diskdb);
    exit(1);
  }

  int memdb_status = sqlite3_open("file::memory:?cache=shared", &conn->memdb);
  if (memdb_status != SQLITE_OK) {
    log_err("error occured at opening in memory sql file");
    sqlite3_close(conn->memdb);
    exit(1);
  }
  
}

void merge_database(struct Connection *conn) {
    char attach_query[QUERY_MAX_SIZE];
    //const char *attach_query = "ATTACH DATABASE 'redis.db' AS diskdb;";
    snprintf(attach_query, sizeof(attach_query), "ATTACH DATABASE '%s' AS diskdb;", conn->filename);
    const char *create_table_query = "CREATE TABLE cache_0 (key TEXT, value TEXT, expires_on TIMESTAMP, created_on TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    const char *insert_query = "INSERT INTO cache_0 SELECT * FROM diskdb.cache_0;";
    const char *detach_query = "DETACH DATABASE diskdb;";
    char *err_msg;

    log_info("%s", create_table_query);
    log_info("%s", insert_query);
    
    sqlite3_exec(conn->memdb, attach_query, 0, 0, &err_msg);
    if (err_msg) {
        log_err("Error attaching database: %s", err_msg);
        sqlite3_free(err_msg);
        exit(1);
    }

    sqlite3_exec(conn->memdb, create_table_query, 0, 0, &err_msg);
    if (err_msg) {
        log_err("Error creating table in memory db: %s", err_msg);
        sqlite3_free(err_msg);
        exit(1);
    }

    sqlite3_exec(conn->memdb, insert_query, 0, 0, &err_msg);
    if (err_msg) {
        log_err("Error inserting into mem db: %s", err_msg);
        sqlite3_free(err_msg);
        exit(1);
    }

    sqlite3_exec(conn->memdb, detach_query, 0, 0, &err_msg);
    if (err_msg) {
        log_err("Error detaching database: %s", err_msg);
        sqlite3_free(err_msg);
        exit(1);
    }
}


void memdb_to_disk_transfer(struct Connection *conn) {
  char attach_query[QUERY_MAX_SIZE];
  //const char *attach_query = "ATTACH DATABASE 'redis.db' AS diskdb;";
  snprintf(attach_query, sizeof(attach_query), "ATTACH DATABASE '%s' AS diskdb;", conn->filename);
  const char *insert_query = "INSERT INTO diskdb.cache_0 SELECT * FROM cache_0 WHERE key NOT IN (SELECT key FROM diskdb.cache_0);";
  const char *detach_query = "DETACH DATABASE diskdb;";

  char *err_msg;

  sqlite3_exec(conn->memdb, attach_query, 0, 0, &err_msg);
  if (err_msg) {
    log_err("error: %s", err_msg);
    sqlite3_free(err_msg);
    exit(1);
  }

  sqlite3_exec(conn->memdb, insert_query, 0, 0, &err_msg);
  if (err_msg) {
    log_err("error: %s", err_msg);
    sqlite3_free(err_msg);
    exit(1);
  }

  sqlite3_exec(conn->memdb, detach_query, 0, 0, &err_msg);
  if (err_msg) {
    log_err("error: %s", err_msg);
    sqlite3_free(err_msg);
    exit(1);
  }

  sqlite3_close(conn->diskdb);
  sqlite3_close(conn->memdb);
  free(conn->filename);
}

void set(struct Connection *conn, char *key, char *value, time_t duration) {
  char insert_quey[QUERY_MAX_SIZE];
  time_t now = time(NULL);
  time_t expires_on = now + duration;
  struct tm *tm_info;
  char expires_on_str[20];

  tm_info = localtime(&expires_on);
  strftime(expires_on_str, sizeof(expires_on_str), "%Y-%m-%d %H:%M:%S", tm_info);
  snprintf(insert_quey, sizeof(insert_quey), "INSERT INTO cache_0 (key, value, expires_on) VALUES('%s', '%s', '%s')", key, value, expires_on_str);

  char *err_msg;

  sqlite3_exec(conn->memdb, insert_quey, 0, 0, &err_msg);
  if (err_msg) {
    log_err("error: %s", err_msg);
    sqlite3_free(err_msg);
    exit(1);
  }
}

void test_insert(struct Connection *conn) {
  const char *insert_query = "INSERT INTO cache_0 (key, value) VALUES ('key1', 'value1'), ('key2', 'value2'), ('key3', 'value3'), ('key4', 'value4'), ('key5', 'value5'), ('key6', 'value6')";
  char *err_msg;
  sqlite3_exec(conn->memdb, insert_query, 0, 0, &err_msg);
  if (err_msg) {
    log_err("error: %s", err_msg);
    sqlite3_free(err_msg);
    exit(1);
  }
}

// int main() {
//   const char *filename = "./redis.db";
//   struct Connection *conn = calloc(1, sizeof(struct Connection));
//   init_database(conn, filename);
//   merge_database(conn);
//   test_insert(conn);
//   memdb_to_disk_transfer(conn);
//   free(conn);

//   return 0;
// }

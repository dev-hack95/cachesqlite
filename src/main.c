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

bool file_exist(const char* filename) {
  FILE *fp = fopen(filename, "r");
  if (fp != NULL) {
    fclose(fp);
    return true;
  }
  return false;
}

void create_db_file(const char *filename) {
  if (!file_exist(filename)) {
    FILE *fpr;
    fpr = fopen(filename, "w+");
    check_mem(fpr);
    fclose(fpr);
  }
}

void init_database(struct Connection *conn,  const char *filename) {
  create_db_file(filename);
  conn->filename = calloc(strlen(filename) + 1, sizeof(char));
  strncpy(conn->filename, filename, strlen(filename) + 1);
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


void init_disk_table(struct Connection *conn) {
    const char *create_table_query = "CREATE TABLE IF NOT EXISTS cache_0 (key TEXT PRIMARY KEY, value TEXT, expires_on TIMESTAMP, created_on TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    char *err_msg;

    sqlite3_exec(conn->diskdb, create_table_query, 0, 0, &err_msg);
    if (err_msg) {
        log_err("Error creating table in disk db: %s", err_msg);
        sqlite3_free(err_msg);
        exit(1);
    }

}

void merge_database(struct Connection *conn) {
    char attach_query[QUERY_MAX_SIZE];
    snprintf(attach_query, sizeof(attach_query), "ATTACH DATABASE '%s' AS diskdb;", conn->filename);
    const char *create_table_query = "CREATE TABLE IF NOT EXISTS cache_0 (key TEXT PRIMARY KEY, value TEXT, expires_on TIMESTAMP, created_on TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    const char *insert_query = "INSERT INTO cache_0 SELECT * FROM diskdb.cache_0;";
    const char *detach_query = "DETACH DATABASE diskdb;";
    char *err_msg;


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
  snprintf(attach_query, sizeof(attach_query), "ATTACH DATABASE '%s' AS diskdb;", conn->filename);
  const char *insert_query = "INSERT INTO diskdb.cache_0 SELECT * FROM cache_0 WHERE key NOT IN (SELECT key FROM diskdb.cache_0);";
  const char *detach_query = "DETACH DATABASE diskdb;";
  const char *disk_delete_qury = "DELETE FROM cache_0;";

  char *err_msg;

  sqlite3_exec(conn->diskdb, disk_delete_qury, 0, 0, &err_msg);
  if (err_msg) {
    log_err("error: %s", err_msg);
    sqlite3_free(err_msg);
    exit(1);
  }

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
    char upsert_query[QUERY_MAX_SIZE];

    
    snprintf(upsert_query, sizeof(upsert_query),
         "INSERT INTO cache_0 (key, value, expires_on) "
         "VALUES('%s', '%s', datetime('now', 'localtime', '+%ld seconds')) "
         "ON CONFLICT(key) DO UPDATE SET "
         "value = excluded.value, "
         "expires_on = excluded.expires_on",
         key, value, (long)duration);    
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(conn->memdb, upsert_query, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        log_err("SQLite error: %s", err_msg);
        sqlite3_free(err_msg);
    }
}

void del(struct Connection *conn, char *key) {
  char delete_query[QUERY_MAX_SIZE];
  snprintf(delete_query, sizeof(delete_query), "DELETE FROM cache_0 WHERE key = '%s'", key);

  char *err_msg;
  sqlite3_exec(conn->memdb, delete_query, 0, 0, &err_msg);
  if (err_msg) {
    log_err("error: %s", err_msg);
    sqlite3_free(err_msg); 
  }
}

char* get(struct Connection *conn, char *key) {
    char *data = NULL;
    char select_query[QUERY_MAX_SIZE];
    char *err_msg;
    
    snprintf(select_query, sizeof(select_query), 
             "SELECT value FROM cache_0 WHERE key = '%s'", key);  
    
    sqlite3_stmt *stmt;
    int status = sqlite3_prepare_v2(conn->memdb, select_query, -1, &stmt, NULL);
    if (status != SQLITE_OK) {
        log_err("Failed to prepare statement: %s", sqlite3_errmsg(conn->memdb));
        return NULL;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* value = (const char *)sqlite3_column_text(stmt, 0);
        if (value != NULL) {
            data = malloc(strlen(value) + 1);
            check_mem(data);
            strcpy(data, value);
        }
    } else {
        log_info("No row found for key: %s", key);
    }
    
    sqlite3_finalize(stmt);
    return data;
}

static inline void ttl_check(struct Connection *conn) {
  time_t now = time(NULL);
  time_t curr_tm = now + 0;
  struct tm *tm_info;
  char curr_on_str[20];

  tm_info = localtime(&curr_tm);
  strftime(curr_on_str, sizeof(curr_on_str), "%Y-%m-%d %H:%M:%S", tm_info);
  char delete_query[QUERY_MAX_SIZE];

  snprintf(delete_query, sizeof(delete_query), "DELETE FROM cache_0 WHERE expires_on IS NOT NULL AND expires_on <= '%s'", curr_on_str);
  char *err_msg;

  sqlite3_exec(conn->memdb, delete_query, 0, 0, &err_msg);
  if (err_msg) {
    log_err("%s", err_msg);
    sqlite3_free(err_msg);
  }

  sqlite3_exec(conn->diskdb, delete_query, 0, 0, &err_msg);
  if (err_msg) {
    log_err("%s", err_msg);
    sqlite3_free(err_msg);
  }
}

static inline void dump_data(struct Connection *conn) {
    sqlite3_stmt *stmt;
    const char *select_query = "SELECT key, value, expires_on, created_on FROM cache_0;";
    int status, rows_processed = 0;

    status = sqlite3_prepare_v2(conn->memdb, select_query, -1, &stmt, NULL);
    if (status != SQLITE_OK) {
        log_err("Failed to prepare statement: %s", sqlite3_errmsg(conn->memdb));
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *key = (const char*)sqlite3_column_text(stmt, 0);
        const char *value = (const char*)sqlite3_column_text(stmt, 1);
        const char *expires_on = (const char*)sqlite3_column_text(stmt, 2);
        const char *created_on = (const char*)sqlite3_column_text(stmt, 3);

        if (key && value && expires_on && created_on) {
            sqlite3_stmt *insert_stmt;
            const char *insert_query = "INSERT INTO cache_0(key, value, expires_on, created_on) VALUES(?, ?, ?, ?)";
            
            status = sqlite3_prepare_v2(conn->diskdb, insert_query, -1, &insert_stmt, NULL);
            if (status != SQLITE_OK) {
                log_err("Failed to prepare insert statement: %s", sqlite3_errmsg(conn->diskdb));
                continue;
            }

            sqlite3_bind_text(insert_stmt, 1, key, -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_stmt, 2, value, -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_stmt, 3, expires_on, -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_stmt, 4, created_on, -1, SQLITE_STATIC);

            status = sqlite3_step(insert_stmt);
            if (status != SQLITE_DONE) {
                log_err("Insert failed: %s", sqlite3_errmsg(conn->diskdb));
            }

            sqlite3_finalize(insert_stmt);
            rows_processed++;
        }
    }

    sqlite3_finalize(stmt);
    log_info("Processed %d rows", rows_processed);
}

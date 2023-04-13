#define  _CRT_SECURE_NO_WARNINGS 1
#include "database.h"

int init_db(sqlite3 *db, int rc)
{
    char* errMsg = 0;

    if (rc)
    {
        fprintf(stderr, "Can't open database:%s\n", sqlite3_errmsg(db));
        return 1;
    }
    else
    {
        printf("Opened database successfully.\n");
    }

    if (sqlite3_exec(db, "create table if not exists person(username TEXT, password TEXT, plugin_name TEXT, uuid TEXT)", NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "create table error:%s\n", sqlite3_errmsg(db));
        return 1;
    }
    else
    {
        printf("create or open table success.\n");
        return 0;
    }
}

int exist(sqlite3* db, unsigned char* username)
{
    char sql[256];
    const char* errmsg;
    sqlite3_stmt* stmt;

    sprintf(sql, "select username from person where username='%s'", username);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "prepare error:%s\n", errmsg);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }
    else
    {
        fprintf(stderr, "step error:%s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }
}

int my_strtok(unsigned char* dest, const unsigned char* src, char delimeter)
{
    assert(dest && src);
    int count = 0;
    while (*src != delimeter)
    {
        *dest++ = *src++;
        count++;
    }
    *dest = '\0';

    return count;
}

int insert(sqlite3* db, unsigned char* username, unsigned char* password, unsigned char* plugin_name, unsigned char* uuid)
{
    char* errmsg;
    char sql[256];

    sprintf(sql, "insert into person values('%s', '%s', '%s', '%s');", username, password, plugin_name, uuid);

    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "insert error:%s\n", errmsg);
        return 1;
    }
    else
    {
        printf("Insert done.\n");
    }
    return 0;
}

int Delete(sqlite3* db, unsigned char* username)
{
    char sql[256];
    char* errmsg;

    int ret = exist(db, username);
    if (ret < 0)
    {
        printf("username does not exist.\n");
        return 1;
    }

    sprintf(sql, "delete from person where username ='%s'", username);

    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "delete error:%s\n", errmsg);
        return 1;
    }
    else
    {
        printf("Delete done.\n");
    }
    return 0;
}

int update(sqlite3* db, unsigned char* username, unsigned char* newPassword, unsigned char* newPlugin_name, unsigned char* newUuid)
{
    char* errmsg;
    char sql[256];

    int ret = exist(db, username);
    if (ret < 0)
    {
        printf("username does not exist.\n");
        return 1;
    }

    if (newPassword != NULL)
    {
        sprintf(sql, "update person set password='%s' where username='%s'", newPassword, username);
        if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
        {
            fprintf(stderr, "update password error:%s\n", errmsg);
            return 1;
        }
        else
        {
            printf("Update password done.\n");
        }

    }

    if (newPlugin_name != NULL)
    {
        sprintf(sql, "update person set plugin_name='%s' where username='%s'", newPlugin_name, username);
        if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
        {
            fprintf(stderr, "update plugin name error:%s\n", errmsg);
            return 1;
        }
        else
        {
            printf("Update plugin name done.\n");
        }
    }

    if (newUuid != NULL)
    {
        sprintf(sql, "update person set uuid='%s' where username='%s'", newUuid, username);
        if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
        {
            fprintf(stderr, "update uuid error:%s\n", errmsg);
            return 1;
        }
        else
        {
            printf("Update uuid done.\n");
        }
    }

    return 0;
}

static int callback(void* data, int argc, char** argv, char** azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int query(sqlite3* db)
{
    char sql[256] = { 0 };
    char* errmsg;

    sprintf(sql, "select * from person;");

    if (sqlite3_exec(db, sql, callback, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "query error:%s\n", errmsg);
        return 1;
    }
    else
    {
        printf("query done.\n");
        if (sqlite3_exec(db, "VACUUM", NULL, NULL, &errmsg) != SQLITE_OK)
        {
            fprintf(stderr, "vacuum error:%s\n", errmsg);
        }
    }
    return 0;
}
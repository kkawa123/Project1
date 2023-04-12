#define  _CRT_SECURE_NO_WARNINGS 1

#include "database.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>

#define DEBUG 1

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

int main()
{
    unsigned char receiveBuf[256] = { 0 };
    unsigned char sendBuf[256] = { 0 };

    

    unsigned char username[256] = { 0 };
    unsigned char password[256] = { 0 };
    unsigned char plugin_name[256] = { 0 };

    int lenU = my_strtok(username, receiveBuf, '\0');
    int lenPw = my_strtok(password, receiveBuf + lenU + 1, '\0');
    int lenPn = my_strtok(plugin_name, receiveBuf + lenU + lenPw + 2, '\0');
    if (!(lenU && lenPw && lenPn))
    {
        printf("receive error!\n");
        return 1;
    }

    sqlite3* db;
    char* errMsg = 0;
    int rc;

    rc = sqlite3_open("test.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database:%s\n", sqlite3_errmsg(db));
        return 1;
    }
    else
    {
        printf("Opened database successfully.\n");
    }

    if (sqlite3_exec(db, "create table if not exists person(username char, password char, plugin_name char)", NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "create table error:%s\n", sqlite3_errmsg(db));
        return 1;
    }
    else
    {
        printf("create or open table success.\n");
    }
    
    while (1)
    {
        printf("Input cmd;\n");
        printf("**********************\n");
        printf("1:insert  2:delete  3:query  4:updata  5:quit\n");
        printf("**********************\n");

        int cmd = 0;

        scanf("%d", &cmd);
        getchar();
        switch (cmd)
        {
        case 1:
        {
            printf("Input username:>");
            scanf("%s", username);
            printf("Input password:>");
            scanf("%s", password);
            printf("Input plugin_name:>");
            scanf("%s", plugin_name);

            insert(db, username, password, plugin_name);
            break;
        }
        case 2:
        {
            printf("Input username:>");
            scanf("%s", username);
            Delete(db, username);
            break;
        }
        case 3:
        {
            query(db);
            break;
        }
        case 4:
        {
            printf("Input username:>");
            scanf("%s", username);
            printf("Input password:>");
            scanf("%s", password);
            printf("Input plugin_name:>");
            scanf("%s", plugin_name);
            update(db, username, password, plugin_name);
            break;
        }
        case 5:
            sqlite3_close(db);
            exit(0);

        default:
            printf("Erro cmd\n");
            break;
        }
    }

    return 0;
}

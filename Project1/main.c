#define  _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <sqlite3.h>
#include <assert.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "sqlite3.lib")
#define DEBUG 1

int my_strtok(char* dest, const char* src, char delimeter)
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

int insert(sqlite3* db)
{
    char* username;
    char* password;
    char* plugin_name;
    char* errmsg;
    char sql[256];

    printf("Input username:>");
    scanf("%s", username);
    getchar();

    printf("Input password:>");
    scanf("%s", password);
    getchar();

    printf("Input plugin name:>");
    scanf("%s", plugin_name);
    getchar();

    sprintf(sql, "insert into person values('%s', '%s', '%s');", username, password, plugin_name);

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

int delete(sqlite3* db)
{
    char* username;
    char* errmsg;
    char sql[256];

    printf("Input username:>");
    scanf("%s", username);
    getchar();

    sprintf(sql, "delete from person where username=%s", username);

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

int update(sqlite3* db)
{
    char* username;
    char* password;
    char* plugin_name;
    char* errmsg;
    char sql[256];

    printf("Input username:>");
    scanf("%s", username);
    getchar();

    printf("Input update password:>");
    scanf("%s", password);
    getchar();

    printf("Input update plugin name:>");
    scanf("%s", plugin_name);
    getchar();

    sprintf(sql, "update person set password=%s where username=%s", password, username);
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "update username error:%s\n", errmsg);
        return 1;
    }
    else
    {
        printf("Update username done.\n");
    }

    sprintf(sql, "update person set plugin_name=%s where username=%s", plugin_name, username);
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "update plugin name error:%s\n", errmsg);
        return 1;
    }
    else
    {
        printf("Update plugin name done.\n");
    }
    return 0;
}



int main()
{
    char* sendBuf = (char *) malloc(256);
    char* receiveBuf = "awa\0a23456\0awaawa";

#ifndef DEBUG
    int sendLength = 0;
    int receiveLength = 0;
    int length = 0;

    SOCKET socket_server;
    SOCKET socket_receive;

    SOCKADDR_IN server_addr;
    SOCKADDR_IN client_addr;

    WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        printf("WSAStartup failed:%d\n", WSAGetLastError());
        return 0;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        printf("Version error!\n");
    }

    socket_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_server == INVALID_SOCKET)
    {
        printf("socket error:%d\n", WSAGetLastError());
        return 0;
    }

    server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 8080;// ?

    int ret = bind(socket_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
    if (ret != 0)
    {
        printf("bind error:%d\n", WSAGetLastError());
    }

    listen(socket_server, 20);

    length = sizeof(SOCKADDR);

    socket_receive = accept(socket_server, (SOCKADDR*)&client_addr, &length);
    receiveLength = recv(socket_receive, receiveBuf, 256, 0);

#endif
    
    char* username = (char *) calloc(1, 256);
    char* password = (char *) calloc(1, 256);
    char* plugin_name = (char *) calloc(1, 256);

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

    if (sqlite3_exec(db, "create table if not exist person(username char, password char, plugin_name char)", NULL, NULL, errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "create table error:%s\n", sqlite3_errmsg(db));
        return 1;
    }
    else
    {
        printf("create or open table success.\n");
    }



    sqlite3_close(db);

#ifndef DEBUG

    closesocket(socket_server);
    closesocket(socket_receive);

    WSACleanup();

#endif

    return 0;
}

#define  _CRT_SECURE_NO_WARNINGS 1

#include "database.h"
#include <iostream>

int main()
{
    unsigned char receiveBuf[256] = "awa\0a123456\0awaawa\0123";

    unsigned char username[256] = { 0 };
    unsigned char password[256] = { 0 };
    unsigned char plugin_name[256] = { 0 };
    unsigned char uuid[32] = { 0 };

    int lenUn = my_strtok(username, receiveBuf, '\0');
    int lenPw = my_strtok(password, receiveBuf + lenUn + 1, '\0');
    int lenPn = my_strtok(plugin_name, receiveBuf + lenUn + lenPw + 2, '\0');
    int lenUu = my_strtok(uuid, receiveBuf + lenUn + lenPw + lenPn + 3, '\0');

    if (!(lenUn && lenPw && lenPn && lenUu))
    {
        printf("receive error!\n");
        return 1;
    }

    sqlite3* db;
    int rc = sqlite3_open("test.db", &db);

    init_db(db, rc);
    
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
            printf("Input uuid:>");
            scanf("%s", uuid);

            insert(db, username, password, plugin_name, uuid);
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
            printf("Input uuid:>");
            scanf("%s", uuid);
            update(db, username, password, plugin_name, uuid);
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

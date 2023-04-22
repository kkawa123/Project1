#define  _CRT_SECURE_NO_WARNINGS 1

#include "database.h"
#include "CServer.hpp"
#include <iostream>
#include "PluginManager.hpp"


void initializer() {
    CServer server;
    server.listen(7980);
    auto data = std::make_shared<JsonData>();
    server.JsonData = data;
    PluginManager pluginMgr;
    while (true) {
        if (server.hasTask()) {
            auto task = server.getTask();
          
            switch (task.second.cmd) {
                case REQ_CLASS: {
                    const char* cstr = task.second.data->c_str();
                    std::vector<std::string> usr;
                    const char* currentStr = cstr;
                    while (*currentStr) {
                        usr.push_back(currentStr);
                        currentStr += usr.back().length() + 1;
                        auto usrName = data->data.find(usr[0]);
                        if (usrName == data->data.end()) {
                            server.m_endpoint.get_con_from_hdl(task.first)->close(websocketpp::close::status::invalid_payload, "用户名或密码错误");
                        }
                        auto usrRef = *usrName;
                        auto p = server.isHead.find(task.first)->second;
                        if (p->uuidStr != usrRef.second->uuid) {
                            server.m_endpoint.get_con_from_hdl(task.first)->close(websocketpp::close::status::invalid_payload, "错误的ip,禁止共享插件");
                        }
                        if (usrRef.second->psw != usr[1]) {
                            server.m_endpoint.get_con_from_hdl(task.first)->close(websocketpp::close::status::invalid_payload, "用户名或密码错误");
                        }
                        auto plugin = std::find(usrRef.second->plugins.begin(), usrRef.second->plugins.end(), usr[2]);
                        if (plugin == usrRef.second->plugins.end()) {
                            server.m_endpoint.get_con_from_hdl(task.first)->close(websocketpp::close::status::invalid_payload, "此用户无此插件");
                        }
                        auto classes = pluginMgr.getClassData(*plugin);
                        for (auto& clazz : *(classes.get())) {
                            auto head = p->encodeResponseHead(p->headData.get(), 1, NULL);
                            auto dt = p->encodeRequestBody((unsigned char*)clazz.c_str(), clazz.length());
                            server.m_endpoint.send(task.first, head->c_str(), head->length(), websocketpp::frame::opcode::binary);
                            for (auto& part : *dt) {
                                server.m_endpoint.send(task.first, part.c_str(), part.length(), websocketpp::frame::opcode::binary);
                            }
                        }
                    }
                    break;
                }
                case ADD_PLUGIN: {
            
                }

            }
        }
    }
}


int main()
{
    unsigned char receiveBuf[256] = "awa\0a123456\0awaawa";

    unsigned char username[256] = { 0 };
    unsigned char password[256] = { 0 };
    unsigned char plugin_name[256] = { 0 };
    unsigned char uuid[16] = { 0 };

    int lenUn = my_strtok(username, receiveBuf, '\0');
    int lenPw = my_strtok(password, receiveBuf + lenUn + 1, '\0');
    int lenPn = my_strtok(plugin_name, receiveBuf + lenUn + lenPw + 2, '\0');

    if (!(lenUn && lenPw && lenPn))
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
        printf("1:insert  2:delete  3:query  4:updata  5:judge  6:quit\n");
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
      
            update(db, username, password, plugin_name);
            break;
        }
        case 5:
        {
            printf("Input username:>");
            scanf("%s", username);
            printf("Input plugin_name:>");
            scanf("%s", plugin_name);

            exist(db, username, plugin_name);
            break;
        }
        case 6:
            sqlite3_close(db);
            exit(0);

        default:
            printf("Erro cmd\n");
            break;
        }
    }

    return 0;
}

#define  _CRT_SECURE_NO_WARNINGS 1

#include "database.h"
#include "CServer.hpp"
#include <iostream>
#include "PluginManager.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

std::vector<std::string> deserializeData(const char* str) {
    const char* cstr = str;
    std::vector<std::string> usr;
    const char* currentStr = cstr;
    while (*currentStr) {
        usr.push_back(currentStr);
        currentStr += usr.back().length() + 1;
    }
    return usr;
}




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
                    
                    std::vector<std::string> usr = deserializeData(task.second.data->c_str());
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
                    break;
                }
                case ADD_PLUGIN: {
                    std::vector<std::string> usr = deserializeData(task.second.data->c_str());
                    if (usr[0] != "860498e8-2206-44e7-87d7-8fa1df126a83") {
                        server.m_endpoint.get_con_from_hdl(task.first)->close(websocketpp::close::status::invalid_payload, "?");
                    }
                    auto user = data->data.find(usr[1]);
                    if (user == data->data.end()) {
                        server.m_endpoint.get_con_from_hdl(task.first)->close(websocketpp::close::status::invalid_payload, "无此用户");
                    }
                    user->second->plugins.push_back(usr[2]);
                    data->save();
                    auto p = server.isHead.find(task.first)->second;
                    auto str = std::string("成功添加插件");
                    auto head = p->encodeResponseHead(p->headData.get(), 100, &str);
                    break;
                }
                case ADD_USER: {
                    std::vector<std::string> usr = deserializeData(task.second.data->c_str());
                    if (usr[0] != "860498e8-2206-44e7-87d7-8fa1df126a83") {
                        server.m_endpoint.get_con_from_hdl(task.first)->close(websocketpp::close::status::invalid_payload, "?");
                    }
                    auto user = std::make_shared<JsonData::UserData>();
                    user->username = usr[1];
                    user->psw = usr[2];
                    user->ip = usr[3];
                    boost::uuids::uuid rand = boost::uuids::random_generator()();
                    const string uuid_string = boost::uuids::to_string(rand);
                    user->uuid = uuid_string;
                    data->data.emplace(user->username, user);
                    data->save();
                    auto p = server.isHead.find(task.first)->second;
                    auto str = std::string("成功添加用户");
                    auto head = p->encodeResponseHead(p->headData.get(), 100, &str);
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    if (argc > 0) {
        
    }
    return 0;
}

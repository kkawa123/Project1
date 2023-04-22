
#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>
#include "protocol.h"
#include "jsondata.hpp"

#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using namespace std;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;


class CServer {
public:
	
	server m_endpoint;
	shared_ptr<JsonData> JsonData;

	CServer() {
		m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
		m_endpoint.set_access_channels(websocketpp::log::alevel::access_core);
		m_endpoint.set_access_channels(websocketpp::log::alevel::app);

		m_endpoint.init_asio();
		m_endpoint.set_open_handler(bind(&CServer::on_open, this, ::_1));
		m_endpoint.set_message_handler(bind(&CServer::on_message, this,&m_endpoint, ::_1, ::_2));
	}

	void listen(uint16_t port) {
		m_endpoint.listen(port);
		m_endpoint.start_accept();
	}

	struct Task {
		unsigned char cmd;
		shared_ptr<string> data;
		Task(unsigned char cmd, shared_ptr<string> data) {
			this->cmd = cmd;
			this->data = data;
		}
	};

	mutex lock;
	queue<pair<websocketpp::connection_hdl,Task>> tasks;


	void enqueueTask(websocketpp::connection_hdl hdl,unsigned char cmd, shared_ptr<string> data) {
		lock.lock();
		tasks.push(pair<websocketpp::connection_hdl, Task>(hdl, Task(cmd, data)));
		lock.unlock();
	}

	pair<websocketpp::connection_hdl, Task> getTask() {
		lock.lock();
		auto p = tasks.front();
		tasks.pop();
		lock.unlock();
		return p;
	}

	bool hasTask() {
		lock.lock();
		bool r = tasks.empty();
		lock.unlock();
		return !r;
	}

    std::map<websocketpp::connection_hdl, std::shared_ptr<protocol>> isHead;

	void on_open(websocketpp::connection_hdl hdl) {
		auto prot = make_shared<protocol>();
		isHead.emplace(hdl, prot);
		auto ip = m_endpoint.get_con_from_hdl(hdl)->get_socket().remote_endpoint().address().to_string();
		bool flag = true;
		for (auto& usr : JsonData->data) {
			if (usr.second->ip == ip) {
				prot->setUUID(usr.second->uuid);
				flag = false;
				break;
			}
		}
		if (flag) {
			m_endpoint.get_con_from_hdl(hdl)->close(websocketpp::close::status::invalid_payload, "不存在的ip地址");
		}
	}

	void on_close(websocketpp::connection_hdl hdl) {
		isHead.erase(hdl);
	}



    void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
		auto pld = msg->get_payload();
		auto p = isHead.find(hdl);
		std::shared_ptr<protocol> prot = NULL;
		if (p == isHead.end()) {
			prot = std::make_shared<protocol>();
			isHead.emplace(hdl, prot);
		}
		else {
			prot = p->second;
		}
		if (prot->isHead) {
			prot->headData = std::move(prot->decodeRequest((unsigned char*)pld.c_str(), pld.size()));
		}
		else {
			//TODO 接受更大数据，目前默认数据小于32kb
			auto data = prot->decodeRequestBody((unsigned char*)pld.c_str(), prot->headData.get(), prot->count);
			if (data == NULL) {
				prot->count = 0;
				prot->isHead = true;
			}
			else {
				enqueueTask(hdl, prot->headData->command, std::move(data));
				prot->count++;
			}
		}
    }
};


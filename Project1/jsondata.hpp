#include "rapidjson/document.h"
//#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <exception>

class JsonData {
	const char* fileName = "./data.d";

public:

	struct UserData {
		std::string username;
		std::string ip;
		std::vector<std::string> plugins;
		std::string psw;
		std::string uuid;
	};

	std::map<std::string, std::shared_ptr<UserData>> data;

	JsonData() {
		std::ifstream in(fileName);
		if (!in.is_open()) {
			fprintf(stderr, "fail to read json file: %s\n", fileName);
			return;
		}

		std::string jsonContent((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		in.close();

		rapidjson::Document dom;
		if (!dom.Parse(jsonContent.c_str()).HasParseError()) {
			try {
				for (auto& m : dom.GetObj()) {
					auto d = std::make_shared<UserData>();
					d->username = std::string(m.name.GetString());
					auto o = m.value.GetObj();
					d->uuid = std::string(o["uuid"].GetString());
					d->psw = std::string(o["psw"].GetString());
					d->ip = std::string(o["ip"].GetString());
					for (auto& plugin : o["plugins"].GetArray()) {
						d->plugins.push_back(std::string(plugin.GetString()));
					}
					data.emplace(d->username, d);
				}
			}
			catch (std::exception e) {
				fprintf(stderr, "fail to parse json, error %s\n", e.what());
			}
		}
	}

	void insert(UserData* d) {
		data.emplace(d->username, std::shared_ptr<UserData>(d));
	}

	void save() {
		rapidjson::Document dom;

		auto& allocator = dom.GetAllocator();
		for (auto& p : data) {
			rapidjson::Value usr(rapidjson::kObjectType);
			usr.AddMember("uuid", rapidjson::StringRef(p.second->uuid.c_str()), allocator);
			usr.AddMember("ip", rapidjson::StringRef(p.second->ip.c_str()), allocator);
			usr.AddMember("psw", rapidjson::StringRef(p.second->psw.c_str()), allocator);
			rapidjson::Value plugins(rapidjson::kArrayType);
			for (auto& plugin : p.second->plugins) {
				plugins.PushBack(rapidjson::StringRef(plugin.c_str()), allocator);
			}
			usr.AddMember("plugins", plugins, allocator);
			dom.AddMember(rapidjson::StringRef(p.first.c_str()), usr, allocator);
		}
		FILE* fp = fopen(fileName, "wb");
		auto buf = std::make_unique<char[]>(65536);

		rapidjson::FileWriteStream fos(fp, buf.get(), 65536);
		rapidjson::Writer<rapidjson::FileWriteStream> writer(fos);
		dom.Accept(writer);
		
		fclose(fp);
	}
};
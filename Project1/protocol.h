#pragma once
#include "openssl/aes.h"
#include "openssl/rand.h"
#include "openssl/hmac.h"
#include "openssl/sha.h"
#include "openssl/evp.h"
#include <ctime>
#include <string>
#include <array>
#include <random>

#ifdef FULL
#define ADD_USER 10
#define ADD_PLUGIN 19
#define DEL_USER 30
#define DEL_PLUGIN 45
#endif // FULL
#define REQ_CLASS 1


class protocol {
	union TimeChar {
		long long time;
		unsigned char bytes[8];
	};

	union SizeChar {
		unsigned int i;
		unsigned char byte[4];
	};

	struct ByteReader
	{
		std::shared_ptr<unsigned char[]> data;
		int len = 0;
		int loc = 0;

		void read(unsigned char* buf, int len) {
			memcpy(buf, data.get() + loc, len * sizeof(char));
			this->len += len;
		}
	};


	struct ByteStack {
		int loc = 0;
		int len = 0;
		std::shared_ptr<unsigned char[]> data;

		ByteStack(int len) {
			data = std::shared_ptr<unsigned char[]>(new unsigned char[len]);
			this->len = len;
		}

		void push(unsigned char c) {
			data[loc] = c;
			loc++;
		}

		void push_all(unsigned char c[], int len) {
			memcpy(data.get() + loc, c, sizeof(char) * len);
			loc += len;
		}
	};
public:
#ifdef FULL
	struct CommandData {
		unsigned char version;
		std::unique_ptr<unsigned char[]> ivec = std::make_unique<unsigned char[]>(16);
		std::unique_ptr<unsigned char[]> key = std::make_unique<unsigned char[]>(16);
		unsigned char res;
		unsigned char paddingCount;
		unsigned char command;
		std::shared_ptr<unsigned char[]> padding;
		std::unique_ptr<unsigned char[]> hash = std::make_unique<unsigned char[]>(4);
		std::shared_ptr<unsigned char[]> data;

		CommandData(unsigned char* d) {
			
			version = d[0];
			memcpy(ivec.get(), d + 1, sizeof(char) * 16);
			memcpy(key.get(), d + 17, sizeof(char) * 16);
			res = d[33];
			paddingCount = d[34];
			command = d[35];
			padding = std::shared_ptr<unsigned char[]>(new unsigned char[paddingCount]);
			memcpy(padding.get(), d + 36, sizeof(char) * paddingCount);
			memcpy(hash.get(), d + 36 + paddingCount, sizeof(char) * 4);
			data = std::shared_ptr<unsigned char[]>(new unsigned char[36 + paddingCount]);
			memcpy(data.get(), d, 36 + paddingCount);
		}

		bool cmpHash() {
			SizeChar sc;
			unsigned int hs = fnv1aHash(data.get(), 36 + paddingCount);
			memcpy(sc.byte, hash.get(), 4);
			unsigned int hs1 = sc.i;
			return hs == hs1;
		}
	};
#endif
	struct ServerCommandData {
		unsigned char res = 0;
		unsigned char cmd = 0;
		unsigned char cmdLen = 0;
		std::shared_ptr<unsigned char[]> data;

		ServerCommandData(unsigned char d[], int l) {
			res = d[0];
			cmd = d[1];
			cmdLen = d[2];
			if (cmdLen > 0) {
				data = std::shared_ptr<unsigned char[]>(new unsigned char[cmdLen]);
				memcpy(data.get(), d + 3, cmdLen);
			}
		}
	};
	std::string uuidStr;
	bool isHead = true;
	std::unique_ptr<CommandData> headData = NULL;
	int count = 0;

	int encrypt(char* inByte,int inLen, char* outByte);
	int decrypt(char* inByte, int inLen, char* outByte);
	std::unique_ptr<std::string> encodeRequest(char command);
	std::unique_ptr<std::vector<std::string>>encodeRequestBody(unsigned char* data, int len);
#ifdef FULL
	std::unique_ptr<CommandData> decodeRequest(unsigned char* d, int len);
	std::unique_ptr<std::string> decodeRequestBody(unsigned char* data, CommandData* cmd, short count);
	std::unique_ptr<std::string> encodeResponseHead(CommandData* cmd, unsigned char ctl, std::string* msg);
#endif
	std::unique_ptr<ServerCommandData> decodeResponseHead(unsigned char* data, int len);
	std::unique_ptr<std::string> decodeResponseBody(unsigned char* data, short count);
	void toUUIDBytes(std::string s, unsigned char* id);
	void setUUID(unsigned char* uuid);
	void setUUID(std::string id);
	unsigned char getRev();
	private:
		static unsigned int fnv1aHash(unsigned char* data, int len);
		std::unique_ptr<unsigned char[]> encrypt_aes_cfb8(unsigned char* data, int len, unsigned char* key, unsigned char* ivec);
		std::unique_ptr<unsigned char[]> decrypt_aes_cfb8(unsigned char* data, int len, unsigned char* key, unsigned char* ivec);
};

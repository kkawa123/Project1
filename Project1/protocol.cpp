#include "protocol.h"

#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")
#ifdef _DEBUG
#include "atlbase.h"
#include "atlstr.h"
#endif // _DEBUG

#pragma warning(disable:4996)

#define VERSION 1
#define REQUEST_CLASS 0

unsigned char uuid[16];
std::default_random_engine e(time(NULL));
unsigned char encrypt_ivec[16];
unsigned char encrypt_key[16];
unsigned char rev;

int protocol::encrypt(char* inByte, int inLen, char* outByte) {
	memcpy(outByte, inByte, sizeof(char) * inLen);
	return inLen;
}
int protocol::decrypt(char* inByte, int inLen, char* outByte) {
	memcpy(outByte, inByte, sizeof(char) * inLen);
	return inLen;
}

#ifdef _DEBUG
void OutputDebugPrintf(const char* strOutputString, ...)
{
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	//vsprintf(strBuffer, strOutputString, vlArgs);
	va_end(vlArgs);
	OutputDebugString(CA2W(strBuffer));
}
#endif
std::unique_ptr<std::string> protocol::encodeRequest(char command)
{	

	time_t t;
	time(&t);
	std::uniform_int_distribution<signed> u(-30, 30);
	TimeChar tc;
	tc.time = t + u(e);
	unsigned char md[16];
	unsigned int len;
	std::unique_ptr<ByteStack> d = std::make_unique<ByteStack>(256);
	//认证信息 16位uuid+8位±30s时间戳 md5
	HMAC(EVP_md5(), uuid, 16, tc.bytes, 8, md, &len);
	
	EVP_MD_CTX* mdctx;
	mdctx = EVP_MD_CTX_new();
	
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
	if (d == NULL) {
		throw "Allocate Fail";
		exit(1);
	}
	d->data = std::shared_ptr<unsigned char[]>(new unsigned char[256]);
	d->len = 256;
	d->push_all(md, sizeof(char) * 16);
	EVP_DigestUpdate(mdctx, tc.bytes, 8 * sizeof(char));
	EVP_DigestUpdate(mdctx, tc.bytes, 8 * sizeof(char));
	EVP_DigestUpdate(mdctx, tc.bytes, 8 * sizeof(char));
	EVP_DigestUpdate(mdctx, tc.bytes, 8 * sizeof(char));

	//IvMD5
	unsigned char md5Iv[16];

	EVP_DigestFinal(mdctx, md5Iv, &len);

	//KeyMD5
	unsigned char keyP2[16];
	toUUIDBytes("f0acea43-0b36-4ff4-ab7e-ff6ef951426e", keyP2);
	unsigned char key[16];
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
	EVP_DigestUpdate(mdctx, uuid, 16 * sizeof(char));
	EVP_DigestUpdate(mdctx, keyP2, 16 * sizeof(char));

	EVP_DigestFinal(mdctx, key, &len);

	d->push(VERSION);

	RAND_seed(tc.bytes, 8);

	//数据加密Ivec/Key
	unsigned char ivec[16];
	RAND_bytes(ivec, 16);
	memcpy(encrypt_ivec, ivec, 16);
	unsigned char enkey[16];
	RAND_bytes(enkey, 16);
	memcpy(encrypt_key, enkey, 16);
	d->push_all(ivec, 16);
	d->push_all(enkey, 16);
	//响应认证
	unsigned char res;
	RAND_bytes(&res, 1);
	d->push(res);
	rev = res;

	//填充数量
	std::uniform_int_distribution<unsigned> padding(0, 15);
	unsigned char count = padding(e);
	d->push(count);

	//指令
	d->push(command);

	//填充
	std::unique_ptr<unsigned char[]> r = std::make_unique<unsigned char[]>(count);
	RAND_bytes(r.get(), count);
	d->push_all(r.get(), count);
	
	//校验值
	std::hash<unsigned char*> s;
	unsigned int hash = fnv1aHash(d->data.get() + 16, d->loc - 16);
	SizeChar sc;
	sc.i = hash;
	d->push_all(sc.byte, 4 * sizeof(char));
	
	auto out = std::shared_ptr<unsigned char[]>(new unsigned char[d->loc]);
	memcpy(out.get(), d->data.get(), 16);
	memcpy(out.get() + 16, encrypt_aes_cfb8(d->data.get() + 16, d->loc - 16, key, md5Iv).get(), d->loc - 16);
#ifdef _DEBUG
	
#endif // DEBUG

	auto result = std::make_unique<std::string>((char*)out.get(), d->loc);
	EVP_MD_CTX_free(mdctx);

	return result;
}

std::unique_ptr<std::vector<std::string>> protocol::encodeRequestBody(unsigned char* data, int len)
{
	std::unique_ptr<std::vector<std::string>> out = std::make_unique<std::vector<std::string>>();
	int lenLeft = len;
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	for (int i = 0; i <= len / 32752; i++) {
		//切块
		if (lenLeft == 0)
			break;
		unsigned char nIv[12];
		nIv[0] = i >> 8;
		nIv[1] = i;
		memcpy(nIv, encrypt_ivec, 10 * sizeof(char));
		EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, encrypt_key, nIv);
		unsigned short partLen = min(len - 32752 * i, 32752);
		std::unique_ptr<unsigned char[]> dataPart = std::make_unique<unsigned char[]>(partLen + 2 + 16);
		std::unique_ptr<unsigned char[]> outBuf = std::make_unique<unsigned char[]>(partLen);
		int outL;
		EVP_EncryptUpdate(ctx, outBuf.get(), &outL, data + i * 32752, partLen);
		dataPart[0] = (partLen+16) >> 8;
		dataPart[1] = partLen+16;
		memcpy(dataPart.get() + 2, outBuf.get(), outL);
		EVP_EncryptFinal(ctx, outBuf.get(), &outL);
		EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, outBuf.get());
		memcpy(dataPart.get() + 2 + 32752, outBuf.get(), 16 * sizeof(char));
		lenLeft -= partLen;
		out->push_back(std::string((char*)dataPart.get(), partLen + 2 + 16));
	}
	
	//尾部标记
	std::unique_ptr<unsigned char[]> end = std::make_unique<unsigned char[]>(2);
	end[0] = 0;
	end[1] = 0;
	out->push_back(std::string((char*)end.get(), 2));
	EVP_CIPHER_CTX_free(ctx);
	return out;
}

#ifdef FULL
std::unique_ptr<protocol::CommandData> protocol::decodeRequest(unsigned char* d, int leng) {
	std::unique_ptr<ByteReader> b = std::make_unique<ByteReader>();
	int dataLen = leng;
	b->data = std::shared_ptr<unsigned char[]>(new unsigned char[dataLen]);
	memcpy(b->data.get(), d, dataLen);

	//认证信息
	std::array<unsigned char, 16> identify;
	b->read(identify.data(), 16);
	time_t ti;
	time(&ti);
	TimeChar tc;
	tc.time = ti;
	std::array<unsigned char, 16> md;
	unsigned int len;
	bool flag = false;
	for (long long t = ti - 120; t <= ti + 60; t++) {
		tc.time = t;
		HMAC(EVP_md5(), uuid, 16, tc.bytes, 8, md.data(), &len);
		if (identify == md) {
			flag = true;
			break;
		}
	}
	if (!flag) {
		throw "Bad Data";
		exit(1);
	}
	
	EVP_MD_CTX* mdctx;
	mdctx = EVP_MD_CTX_new();
	EVP_DigestInit(mdctx, EVP_md5());
	
	EVP_DigestUpdate(mdctx, tc.bytes, 8 * sizeof(char));
	EVP_DigestUpdate(mdctx, tc.bytes, 8 * sizeof(char));
	EVP_DigestUpdate(mdctx, tc.bytes, 8 * sizeof(char));
	EVP_DigestUpdate(mdctx, tc.bytes, 8 * sizeof(char));

	//IvMD5
	unsigned char md5Iv[16];

	EVP_DigestFinal(mdctx, md5Iv, &len);

	//KeyMD5
	unsigned char keyP2[16];
	toUUIDBytes("f0acea43-0b36-4ff4-ab7e-ff6ef951426e", keyP2);
	unsigned char key[16];
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
	EVP_DigestUpdate(mdctx, uuid, 16 * sizeof(char));
	EVP_DigestUpdate(mdctx, keyP2, 16 * sizeof(char));

	EVP_DigestFinal(mdctx, key, &len);
	
	std::unique_ptr<unsigned char[]> data = std::make_unique<unsigned char[]>(dataLen - 16);
	data = std::move(decrypt_aes_cfb8(d + 16, dataLen - 16, key, md5Iv));
	
	//解析
	std::unique_ptr<CommandData> cmd = std::make_unique<CommandData>(data.get());
	if (!cmd->cmpHash()) {
		throw "Bad Data";
		exit(1);
	}
	EVP_MD_CTX_free(mdctx);
	return cmd;
}

std::unique_ptr<std::string> protocol::decodeRequestBody(unsigned char* data, CommandData* cmd, short count) {
	int len = (data[0] << 8) + data[1];
	if (len == 0) {
		return NULL;
	}
	EVP_CIPHER_CTX* ctx;
	ctx = EVP_CIPHER_CTX_new();
	unsigned char iv[12];
	iv[0] = count >> 8;
	iv[1] = count;
	memcpy(iv + 2, cmd->ivec.get(), 10 * sizeof(char));
	EVP_DecryptInit(ctx, EVP_aes_128_gcm(), cmd->key.get(), iv);
	unsigned char tag[16];
	memcpy(tag, data + 2 + len - 16, sizeof(char) * 16);
	int outlen;
	std::unique_ptr<unsigned char[]> outbuf = std::make_unique<unsigned char[]>(32768);
	outlen = len;
	EVP_DecryptUpdate(ctx, outbuf.get(), &outlen, data + 2, len - 16);
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, 16, tag);
	std::unique_ptr<std::string> plaintext = std::make_unique<std::string>((char*)outbuf.get(), len - 16);
	
	int rev = EVP_DecryptFinal_ex(ctx, outbuf.get(), &outlen);
	if (rev <= 0) {
		fprintf(stdout, "Verify Fail");
		exit(1);
	}
	EVP_CIPHER_CTX_free(ctx);
	return plaintext;
}



#endif 

std::unique_ptr<std::string> protocol::encodeResponseHead(CommandData* cmd, unsigned char ctl, std::string* msg = NULL) {
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	unsigned char civec[16];
	memcpy(civec, cmd->ivec.get(), 16);
	EVP_EncryptInit_ex(ctx, EVP_aes_128_cfb8(), NULL, cmd->key.get(), civec);
	std::unique_ptr<ByteStack> b = std::make_unique<ByteStack>(512);
	b->push(cmd->res);
	b->push(ctl);
	if (msg == NULL) {
		b->push(0);
	}
	else {
		b->push(msg->length() + 1);
		b->push_all((unsigned char*)msg->c_str(), msg->length() + 1);
	}
	unsigned char outBuf[512];
	int outL;
	EVP_EncryptUpdate(ctx, outBuf, &outL, b->data.get(), b->loc);
	auto out = std::make_unique<std::string>((char*)outBuf, outL);
	EVP_EncryptFinal(ctx, outBuf, &outL);
	EVP_CIPHER_CTX_free(ctx);
	return out;
}

std::unique_ptr<protocol::ServerCommandData> protocol::decodeResponseHead(unsigned char* data,int len)
{
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	unsigned char civec[16];
	memcpy(civec, encrypt_ivec, 16);
	EVP_DecryptInit_ex(ctx, EVP_aes_128_cfb8(), NULL, encrypt_key, civec);
	unsigned char outBuf[512];
	int outL;
	EVP_DecryptUpdate(ctx, outBuf, &outL, data, len);
	std::unique_ptr<protocol::ServerCommandData> out = std::make_unique<protocol::ServerCommandData>(outBuf, outL);
	EVP_DecryptFinal(ctx, outBuf, &outL);
	EVP_CIPHER_CTX_free(ctx);
	if (rev != out->res) {
		throw "Verify Fail";
		exit(1);
	}
	return out;
}

std::unique_ptr<std::string> protocol::decodeResponseBody(unsigned char* data, short count)
{
	int len = (data[0] << 8) + data[1];
	EVP_CIPHER_CTX* ctx;
	ctx = EVP_CIPHER_CTX_new();
	unsigned char iv[12];
	iv[0] = count >> 8;
	iv[1] = count;
	memcpy(iv + 2, encrypt_ivec, 10 * sizeof(char));
	EVP_DecryptInit(ctx, EVP_aes_128_gcm(), encrypt_key, iv);
	unsigned char tag[16];
	memcpy(tag, data + 2 + len - 16, sizeof(char) * 16);
	int outlen;
	std::unique_ptr<unsigned char[]> outbuf = std::make_unique<unsigned char[]>(32768);
	outlen = len;
	EVP_DecryptUpdate(ctx, outbuf.get(), &outlen, data + 2, len - 16);
	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, 16, tag);
	auto plaintext = std::make_unique<std::string>((char*)outbuf.get(), outlen);
	int rev = EVP_DecryptFinal_ex(ctx, outbuf.get(), &outlen);
	if (rev <= 0) {
		fprintf(stdout, "Verify Fail");
		exit(1);
	}
	EVP_CIPHER_CTX_free(ctx);
	return plaintext;
}

std::unique_ptr<unsigned char[]> protocol::encrypt_aes_cfb8(unsigned char* data, int len, unsigned char* key, unsigned char* ivec) {
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	unsigned char civec[16];
	memcpy(civec, ivec, 16);
	EVP_EncryptInit_ex(ctx, EVP_aes_128_cfb8(), NULL, key, civec);
	int outL;
	unsigned char outBuf[512];

	EVP_EncryptUpdate(ctx, outBuf, &outL, data, len);
	std::unique_ptr<unsigned char[]> out = std::make_unique<unsigned char[]>(outL);
	memcpy(out.get(), outBuf, outL);
	EVP_EncryptFinal(ctx, outBuf, &outL);
	EVP_CIPHER_CTX_free(ctx);
	return out;
}

std::unique_ptr<unsigned char[]> protocol::decrypt_aes_cfb8(unsigned char* data, int len, unsigned char* key, unsigned char* ivec)
{
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	unsigned char civec[16];
	memcpy(civec, ivec, 16);
	EVP_DecryptInit_ex(ctx, EVP_aes_128_cfb8(), NULL, key, civec);
	unsigned char outBuf[512];
	int outL;
	EVP_DecryptUpdate(ctx, outBuf, &outL, data, len);
	std::unique_ptr<unsigned char[]> out = std::make_unique<unsigned char[]>(outL);
	memcpy(out.get(), outBuf, outL);
	EVP_DecryptFinal(ctx, outBuf, &outL);
	EVP_CIPHER_CTX_free(ctx);
	return out;
}

void protocol::toUUIDBytes(std::string s, unsigned char* uuid)
{
	s.replace(s.find('-'), 1, "");
	s.replace(s.find('-'), 1, "");
	s.replace(s.find('-'), 1, "");
	s.replace(s.find('-'), 1, "");
	int loc = 0;
	for (auto iter = s.begin(); iter < s.end(); iter += 2) {
		char h = *iter;
		char l = *(iter + 1);
		char tmpH, tmpL;
		if (h >= 'a' && h <= 'f') {
			tmpH = h - 'a' + 10;
		}
		else {
			tmpH = h - '0';
		}
		if (l >= 'a' && l <= 'f') {
			tmpL = l - 'a' + 10;
		}
		else {
			tmpL = l - '0';
		}
		uuid[loc] = tmpH * 16 + tmpL;
		loc++;
	}
}

unsigned int protocol::fnv1aHash(unsigned char* data, int len) {
	int p = 16777619;
	uint32_t h = 2166136261;
	for (size_t i = 0; i < len; ++i) {
		h = h ^ data[i];
		h = h * 16777619;
		
	}
	return h;
}

unsigned char protocol::getRev()
{
	return rev;
}



void protocol::setUUID(unsigned char* id)
{
	memcpy(uuid, id, sizeof(char) * 16);
}

void protocol::setUUID(std::string id) {
	unsigned char uu[16];
	toUUIDBytes(id, uu);
	setUUID(uu);
	uuidStr = id;
}





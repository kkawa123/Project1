#pragma once

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <sqlite3.h>
#include <assert.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "sqlite3.lib")

int exist(sqlite3*, unsigned char*);
int my_strtok(unsigned char*, const unsigned char*, char);
int insert(sqlite3*, unsigned char*, unsigned char*, unsigned char*);
int Delete(sqlite3*, unsigned char*);
int update(sqlite3*, unsigned char*, unsigned char*, unsigned char*);
int query(sqlite3*);
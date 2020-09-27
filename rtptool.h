#ifndef FTPTOOL_H
#define FTPTOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cctype>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include<iostream>
#include<dirent.h>
#include<string>

#define MAX_LEN 1024*1
#define MAX_CMD_LEN 129

int ftp_upload_file(const char *ip, unsigned int port, const char *user, const char *pwd,
const char *upload_file,const char *upload_name, const char *folder_name);

#endif // FTPTOOL_H
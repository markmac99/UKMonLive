#pragma once
/*
Copyright 2018 Mark McIntyre.

UKMONLiveCL is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#define _WIN32_WINNT 0x0501

#include "EventLog.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ShlObj.h>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>


struct KeyData
{
	char AccountKey[128];
	char AccountName[128];
	char QueueEndPoint[128];
	char StorageEndPoint[128];
	char TableEndPoint[128];
	char BucketName[128];
	char AccountKey_D[128];
	char AccountName_D[128];
	char region[128];
};

extern struct KeyData theKeys;
extern char ProcessingPath[512];
extern int nCounter;
extern int maxretry;
extern int Debug;
extern int dryrun;
extern int delay_ms;
extern long framelimit;
extern long minbright;
extern long minframes;

int LoadIniFiles(void);

int String2Hex(char* out, char* in);
int Hex2String(char* out, char* in);
int Encrypt(char *out, char* in, int Key);
int Decrypt(char *out, char* in, int Key);

int put_file(char* buckname, const char* fname, char* reg, char* acct, char* secret, long frcount, long maxbmax);

int ProcessData(std::string pattern, long framelimit, long minbright, char *pth);
int ReadBasicXML(std::string pth, const char* cFileName, long &frcount, long &maxbmax);
int ReadAnalysisXML(std::string pth, const char* cFileName, double &mag);



// Encryptor.cpp : This file contains the 'main' function. Program execution begins and ends there.

//
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "UKMonLiveCL.h"

int delay_ms;
long framelimit, minbright; // unused but needed in auth.cpp

#define AUTHFILENAME "AUTH_UKMON.ini"

int main(int argc, char** argv)
{
	char plainkey[128] = { 0 }, plainsecret[128] = { 0 };
	char encKey[128] = { 0 }, encSecret[128] = { 0 };

	// testing
#if 0
	strcpy(encKey, "43494B434837344C353140465453584650464053");
	strcpy(encSecret, "2B49454A627749437D553561703237703736634967533C5C60675352705653685566337C61624166");
	Decrypt(plainkey, encKey, 712);
	Decrypt(plainsecret, encSecret, 1207);

	char encKey1[128] = { 0 }, encSecret1[128] = { 0 };
	Encrypt(encKey1, plainkey, 712);
	Encrypt(encSecret1, plainsecret, 1207);
	int res = strcmp(encKey1, "43494B434837344C353140465453584650464053");
	res = strcmp(encSecret1, "2B49454A627749437D553561703237703736634967533C5C60675352705653685566337C61624166");

#endif
	FILE* srcf;
	if(argc<2)
		srcf = fopen(".\\keys.txt", "r");
	else
		srcf = fopen(argv[1], "r");

	if (!srcf)
	{
		printf("unable to open %s\n", argc<2?"keys.txt":argv[1]);
		exit(0);
	}
	fgets(plainkey, sizeof(plainkey), srcf);
	plainkey[strlen(plainkey) - 1] = 0; // remove newline
	fgets(plainsecret, sizeof(plainsecret), srcf);
	if(plainsecret[strlen(plainsecret) - 1]=='\n') plainsecret[strlen(plainsecret) - 1] = 0; // remove newline
	Encrypt(encKey, plainkey, 712);
	Encrypt(encSecret, plainsecret, 1207);


	char szLocalPath[512];
	//HRESULT hres = SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szLocalPath);
	strcpy (szLocalPath, ".");
	char authfile[512];
	sprintf(authfile, "%s\\%s", szLocalPath, AUTHFILENAME);

	FILE* f = fopen(authfile, "w");

	fputs("UKMON Auth file\n", f);
	fputs(encSecret, f);
	fputc('\n', f);
	fputs(encKey, f);
	fputc('\n', f);
	fputs("queue.Amazonaws.com\n", f);
	fputs("s3-eu-west-1.Amazonaws.com\n", f);
	fputs("sdb.Amazonaws.com\n", f);
	fputs("ukmon-live", f);
	fclose(f);
}

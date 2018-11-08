#define _CRT_SECURE_NO_WARNINGS
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

To build this project you will need the AWS C++ SDK, Visual Studio 2015 or later and the relevant MSVC Runtime Libraries.

*/

#include "UKMonLiveCL.h"

#define BUFLEN 32768 // number of files I can read at one time
int nCounter;		 // number of events uploaded
int maxretry = 5;	 // number of retries 
int delay_ms=1000;	 // retry delay if the jpg isn't present or the upload fails
long framelimit=120; // max number of frames before we consider it to be an aircraft or bird
long minframes = 63; // software records 30 frames either side of event so 63 = 3 actual frames of data
long minbright=60;	 // min brightness to be too dim to bother uploading


std::ofstream errf;

int Debug = 0;
int dryrun = 0;

int main(int argc, char** argv)
{
	if (argc > 1 && argv[1][0] == 'D')
		Debug = 1;
	if (argc > 1 && argv[1][0] == 'T')
	{
		Debug = 1;
		dryrun = 1;
	}

	if (LoadIniFiles() < 0)
		return -1;

	std::time_t t = std::time(0);   // get time now
	std::tm* now = std::localtime(&t);

	std::cout << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << ' ' << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec;
	std::cout << " UKMON Live Filewatcher C++ version. Monitoring: " << ProcessingPath << std::endl;
	std::cout << "logging being sent to " << ErrFile << std::endl;
	if (framelimit > -1)
	{
		std::cout << "Checking for trails longer than " << framelimit << "; ";
		errf << "Checking for trails longer than " << framelimit << "; ";
	}
	else
	{
		std::cout << "No framecount active; ";
		errf << "No framecount active; ";
	}
	if (minbright > -1)
	{
		std::cout << "Checking for objects brighter than " << minbright;
		errf << "Checking for objects brighter than " << minbright;
	}
	else
	{
		std::cout << "No brightness check active ";
		errf << "No brightness check active ";
	}
	std::cout << std::endl;
	errf << std::endl;

	errf << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << ' ' << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec;
	errf << " UKMON Live Filewatcher C++ version. Monitoring: " << ProcessingPath << std::endl;
	
	if (Debug)
	{
		std::cout << "Debugging is on" << std::endl;
		errf << "Debugging is on" << std::endl;
	}
	if (dryrun)
	{
		std::cout << "Dry run enabled - nothing will be uploaded" << std::endl;
		errf << "Dry run enabled - nothing will be uploaded" << std::endl;
	}

	std::cout << "==============================================" << std::endl;

	wchar_t lpDir[512] = {0};
	DWORD buflen=0, retsiz=0;
	DWORD dwFilter = FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME| FILE_NOTIFY_CHANGE_SIZE;

	mbstowcs(lpDir, ProcessingPath, strlen(ProcessingPath));
	HANDLE hDir = CreateFile(
		lpDir,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);
	if (hDir == NULL)
	{
		errf << "Invalid data file path" << std::endl;
		errf.close();
		return -1;
	}

	nCounter = 0;

	// last filename retrieved - to avoid processing the same file twice
	// the initial write to a file generates one event but subsequent ones generate two
	wchar_t lastname[512] = { 0 }; 
	while (1)
	{
		FILE_NOTIFY_INFORMATION *lpBuf = (FILE_NOTIFY_INFORMATION *)calloc(BUFLEN, sizeof(DWORD));
		buflen = BUFLEN* sizeof(DWORD);
		FILE_NOTIFY_INFORMATION *pbuf;
		if (!lpBuf)
		{
			errf << "Unable to allocate buffer for directory reads" << std::endl;
			errf.close();
			exit (-1);
		}
		if (Debug) std::cout << "1. waiting for changes" << std::endl;
		if (ReadDirectoryChangesW(hDir, (LPVOID)lpBuf, buflen, TRUE, dwFilter, &retsiz, NULL, NULL))
		{
			if (Debug) std::cout << "2. Recieved " << retsiz << " bytes" << std::endl;
			if (lpBuf == NULL)
			{
				wchar_t msg[512] = { 0 };
				char msg_s[512] = { 0 };
				DWORD e = GetLastError();
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, e, 0, msg, 512,NULL);
				wcstombs(msg_s, msg, 512);
				std::cout << "Error " << e << " scanning directory" << ProcessingPath << " - buffer trashed" << msg << std::endl;
				errf << "Error " << e << " scanning directory" << ProcessingPath << " - buffer trashed" << msg << std::endl;
			}	
			else if (retsiz > 0)
			{
				DWORD offset; 
				pbuf = lpBuf;
				do 
				{
					wchar_t fname[512] = { 0 };
					wcsncpy(fname, pbuf->FileName, pbuf->FileNameLength/2);
					offset = pbuf->NextEntryOffset;

//					if (Debug) fprintf(stdout, "3. %S %S %X\n", lastname, fname, pbuf->Action);
					// check file is different and action is modified 
					if (wcsncmp(lastname, fname, pbuf->FileNameLength) != 0 && (pbuf->Action == FILE_ACTION_ADDED))
					{
						char filename_s[512] = { 0 };
						wcstombs(filename_s, fname, wcslen(fname));

						if (Debug) std::cout << filename_s << std::endl;


						// wait for the XML file, but skip files with a + in it as these are manual captures. 
						// and skip files with A.XML in them as these are analysis files
						std::string fn = filename_s;
						size_t m1, m2, m3, l;
						m1 = fn.find(".xml");
						m2 = fn.find("+");
						m3 = fn.find("A.XML");
						l = fn.npos;

						if(m1!=l && m2==l && m3 == l)
						{
							// check for data quality - long recordings are usually planes or birds, short ones are flashes
							// and dim ones arent worth uploading to the live stream

							long frcount = 0;
							long maxbmax = 0;
							std::string pth = ProcessingPath;
							int gooddata = 1;
							ReadBasicXML(pth, filename_s, frcount, maxbmax);
							if (framelimit > -1 && (frcount > framelimit || frcount < minframes ))
								gooddata = 0;
							if (minbright > -1 && maxbmax < minbright)
								gooddata = 0;

							if(Debug) std::cout << "A: " << filename_s << " " << theKeys.BucketName << std::endl;

							if (gooddata)
							{
								put_file(theKeys.BucketName, filename_s, theKeys.region, theKeys.AccountName_D, theKeys.AccountKey_D, frcount, maxbmax);
								fn.replace(m1, 4, "P.jpg");
								put_file(theKeys.BucketName, fn.c_str(), theKeys.region, theKeys.AccountName_D, theKeys.AccountKey_D, frcount, maxbmax);
							}
							else
							{
								nCounter++;
								std::cout << nCounter << ": Skipping " << filename_s << ":";
								if (frcount < minframes)
									std::cout << " framecount " << frcount << "<" << minframes;
								if (frcount > framelimit)
									std::cout << " framecount " << frcount << ">" << framelimit;
								if (maxbmax < minbright)
									std::cout << " brightness " << maxbmax << "<" << minbright;
								std::cout << std::endl;

								errf << nCounter << ": Skipping " << filename_s << ":";
								if (frcount < minframes)
									errf << " framecount " << frcount << "<" << minframes;
								if (frcount > framelimit)
									errf<< " framecount " << frcount << ">" << framelimit;
								if (maxbmax < minbright)
									errf << " brightness " << maxbmax << "<" << minbright;
								errf << std::endl;

							}
						}
					}
					char *p = (char *)pbuf;
					p += offset;
					pbuf = (FILE_NOTIFY_INFORMATION *)p;
					wcscpy(lastname, fname);
				} while (offset > 0);
				memset(lpBuf, 0, BUFLEN);
			}
		}
		else
		{
			wchar_t msg[512] = { 0 };
			char msg_s[512] = { 0 };
			DWORD e = GetLastError();
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, e, 0, msg, 512, NULL);
			wcstombs(msg_s, msg, 512);
			std::cout << "Error " << e << " scanning directory" << ProcessingPath << " " << msg << std::endl;
			errf << "Error " << e << " scanning directory" << ProcessingPath << " " << msg << std::endl;
			errf.close();
			exit(-1);
		}
		free(lpBuf);
	}
	errf.close();
	return 0;
}


int SendToS3(wchar_t * fname)
{
	return 0;
}
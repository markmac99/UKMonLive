#pragma once
#include "..\UKMonLiveCL\UKMonLiveCL.h"

int ReadBasicXML(std::string pth, const char* cFileName, long &frcount, long &maxbmax)
{
	std::ifstream thef;
	std::string fname = pth;
	fname += "/";
	fname += cFileName;
	thef.open(fname);
	int retry = 0;
	while (!thef.is_open() && retry++ < 100)
	{
		//errf << "Unable to open file " << cFileName << std::endl;
		Sleep(100);
		thef.open(fname);
	}
	if (!thef.is_open())
	{
		std::cout << "Unable to open xml file " << cFileName << std::endl;
		errf << "Unable to open xml file " << cFileName << std::endl;
	}
	else
	{
		std::string aline;
		std::getline(thef, aline); 
		std::getline(thef, aline);
		size_t m1 = aline.find("frames");
		if (m1 == aline.npos || aline.length()==0)
			std::cerr << "frames entry not found in " << cFileName << std::endl;
		else
		{
			size_t m2 = aline.find("\"", m1 + 8);
			std::string fr = aline.substr(m1 + 8, m2 - m1 - 8);
			frcount = atol(fr.c_str());
		}
		std::getline(thef, aline);
		m1 = aline.find("hit") + 5;
		if (m1 == aline.npos || aline.length() == 0)
			errf << "hit entry not found in " << cFileName << std::endl;
		else
		{
			size_t m2 = aline.find("\"", m1);
			std::string hi = aline.substr(m1, m2 - m1);
			long hitcount = atol(hi.c_str());
			if (Debug) std::cout << hitcount << " ";
			for (int i = 0; i < hitcount; i++)
			{
				std::getline(thef, aline);
				m1 = aline.find("bmax") + 6;
				if (m1 == aline.npos || aline.length() == 0)
					errf << "bmax entry not found in " << cFileName << std::endl;
				else
				{
					size_t m2 = aline.find("\"", m1);
					std::string bm = aline.substr(m1, m2 - m1);
					long bmax = atol(bm.c_str());
					if (bmax > maxbmax)
						maxbmax = bmax;
					if (Debug) std::cout << bmax << " ";
				}
			}
			if (Debug)std::cout << frcount << " " << maxbmax << std::endl;
			if(Debug) std::cout << cFileName << " frames=" << frcount << " ";
			if (Debug) std::cout << "max bmax=" << maxbmax << " ";
		}
		thef.close();
	}
	return 0;
}
int ReadAnalysisXML(std::string pth, const char* cFileName, double &mag)
{
	std::ifstream thef;
	std::string fname = pth;
	fname += "/";
	fname += cFileName;
	std::string postf = "A.xml";
	fname.replace(fname.length()-4, 5, postf);
	thef.open(fname);
	if (!thef.is_open())
		std::cout << "mag=99.99" << std::endl;
	else
	{
		std::string aline;
		std::getline(thef, aline); // skip this line
		size_t m1 = aline.find("mag=");
		while ((m1 = aline.find(" mag=")) == aline.npos && !thef.eof() && aline.length() != 0)
			std::getline(thef, aline);

		if (thef.eof())
			std::cout << "mag=99.99" << std::endl;
		else
		{
			size_t m2 = aline.find("\"", m1 + 6);
			std::string fr = aline.substr(m1 + 6, m2 - m1 - 6);
			mag = atof(fr.c_str());
		}
		std::cout << "mag=" << mag << std::endl;
		thef.close();
	}
	return 0;
}

int ProcessData(std::string pattern, long framelimit, long minbright, char *pth)
{
	std::ofstream csvfile;
	csvfile.open("c:/temp/analysis.csv");
	WIN32_FIND_DATAA data;
	HANDLE hFind = FindFirstFileA(pattern.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (Debug) std::cout << data.cFileName << std::endl;
			long maxbmax=0, frcount=0;
			double mag=99.99;
			ReadBasicXML(pth, data.cFileName, frcount, maxbmax);
			ReadAnalysisXML(pth, data.cFileName, mag);
			csvfile << data.cFileName << "," << maxbmax << "," << frcount << "," << mag << std::endl;
		} while (FindNextFileA(hFind, &data));
		FindClose(hFind);
	}
	csvfile.close();
	return 0;
}
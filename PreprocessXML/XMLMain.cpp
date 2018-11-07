#pragma once
#include "..\UKMonLiveCL\UKMonLiveCL.h"
std::ofstream errf;

int Debug = 0;

int main(int argc, char **argv)
{
	if (argc < 5)
	{
		std::cout << "Usage: preprocessxml path cameraid maxframes minbrightness" << std::endl;
		std::cout << "eg preprocessxml c:\\temp\\ NE 100 80" << std::endl;
	}
	std::string pattern = argv[1];
	pattern += "/*";
	pattern += argv[2];
	pattern += ".xml";
	long framelimit = atol(argv[3]);
	long minbright = atol(argv[4]);
	errf.open("C:/temp/preprocessxml.log");
	ProcessData(pattern, framelimit, minbright, argv[1]);
	errf.close();
	return 0;
}

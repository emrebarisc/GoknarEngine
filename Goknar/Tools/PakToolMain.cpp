#include <iostream>
#include <string>

#include "Goknar/Data/DataEncryption.h"

int main(int argc, char** argv)
{
	if (argc != 5)
	{
		std::cerr << "Usage: GoknarPakTool pack <source-directory> <pak-output-path> <mount-point>" << std::endl;
		return 1;
	}

	const std::string command = argv[1];
	const std::string sourceDirectory = argv[2];
	const std::string pakOutputPath = argv[3];
	const std::string mountPoint = argv[4];

	if (command != "pack")
	{
		std::cerr << "Unknown command: " << command << std::endl;
		return 1;
	}

	return DataEncryption::CreatePakFromDirectory(sourceDirectory, pakOutputPath, mountPoint) ? 0 : 2;
}

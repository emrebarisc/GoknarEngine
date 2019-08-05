#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__

#include "Goknar/Core.h"

class GOKNAR_API IOManager
{
public:
	IOManager();
	~IOManager();

	static bool GetFileRawText(const char* filePath, char* rawText);
protected:

private:
};

#endif
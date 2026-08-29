#ifndef __MODELLOADER_H__
#define __MODELLOADER_H__

#include <string>

class Content;

class ModelLoader
{
	friend class IOManager;
public:
	ModelLoader() = delete;

protected:

private:
	static Content* LoadModel(const std::string& path);
};

#endif

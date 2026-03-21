#ifndef __MODELLOADER_H__
#define __MODELLOADER_H__

#include <string>

class StaticMesh;

class ModelLoader
{
	friend class IOManager;
public:
	ModelLoader() = delete;

protected:

private:
	static StaticMesh* LoadPlyFile(const std::string& path);
	static StaticMesh* LoadModel(const std::string& path);
};

#endif
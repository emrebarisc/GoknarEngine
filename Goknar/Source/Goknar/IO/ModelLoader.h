#ifndef __MODELLOADER_H__
#define __MODELLOADER_H__

#include <string>

class StaticMesh;
struct aiScene;

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
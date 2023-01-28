#ifndef __MODELLOADER_H__
#define __MODELLOADER_H__

#include <string>

class StaticMesh;
struct aiScene;

class ModelLoader
{
public:
	ModelLoader() = delete;

	static StaticMesh* LoadPlyFile(const std::string& path);
	static StaticMesh* LoadModel(const std::string& path);

protected:

private:
};

#endif
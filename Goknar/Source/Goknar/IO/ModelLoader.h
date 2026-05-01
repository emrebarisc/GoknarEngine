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
	template<typename T>
	static T* LoadModel(const std::string& path);
};

#endif
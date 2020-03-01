#ifndef __MODELLOADER_H__
#define __MODELLOADER_H__

class StaticMesh;

class ModelLoader
{
public:
	ModelLoader() = delete;

	static StaticMesh* LoadPlyFile(const char* path);

protected:

private:
};

#endif
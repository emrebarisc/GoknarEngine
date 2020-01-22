#ifndef __MODELLOADER_H__
#define __MODELLOADER_H__

class Mesh;

class ModelLoader
{
public:
	ModelLoader() = delete;

	static Mesh* LoadPlyFile(const char* path);

protected:

private:
};

#endif
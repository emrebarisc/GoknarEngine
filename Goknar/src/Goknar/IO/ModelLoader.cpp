#include "pch.h"

#include "ModelLoader.h"

#include "Goknar/Model/Mesh.h"

Mesh* ModelLoader::LoadPlyFile(const char* path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		return nullptr;
	}

	std::string line;
	std::stringstream stringStream;

	Mesh* mesh = new Mesh();
	int vertexCount = 0;
	int faceCount = 0;

	bool headerEnded = false;

	bool readVertices = false;
	bool readNormals = false;
	bool readUVs = false;
	bool readColors = false;

	// Read header
	while (!headerEnded && std::getline(file, line))
	{
		std::string keyword, type, value;
		stringStream << line;
		while (stringStream >> keyword)
		{
			if (keyword == "element")
			{
				stringStream >> type;

				if (type == "vertex")
				{
					stringStream >> vertexCount;
				}

				if (type == "face")
				{
					stringStream >> faceCount;
				}
			}
			else if (keyword == "property")
			{
				stringStream >> type >> value;

				if (value == "x" || value == "y" || value == "z")
				{
					readVertices = true;
				}
				else if (value == "nx" || value == "ny" || value == "nz")
				{
					readNormals = true;
				}
				else if (value == "s" || value == "t")
				{
					readUVs = true;
				}
				else if (value == "red" || value == "green" || value == "blue" || value == "alpha")
				{
					readColors = true;
				}
			}
			else if (keyword == "end_header")
			{
				headerEnded = true;
				break;
			}
		}
		stringStream.clear();
	}

	// Read data
	float x = 0;
	float y = 0;
	float z = 0;
	float nx = 0;
	float ny = 0;
	float nz = 0;
	float s = 0;
	float t = 0;
	float red = 255.f;
	float green = 255.f;
	float blue = 255.f;
	float alpha = 255.f;
	
	for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		std::getline(file, line);
		stringStream << line;
		if (readVertices)
		{
			stringStream >> x >> y >> z;
		}
		if (readNormals)
		{
			stringStream >> nx >> ny >> nz;
		}
		if (readUVs)
		{
			stringStream >> s >> t;
		}
		if (readColors)
		{
			stringStream >> red >> green >> blue >> alpha;
		}

		mesh->AddVertexData(VertexData(Vector3(x, y, z), Vector3(nx, ny, nz), Vector4(red / 255.f, green / 255.f, blue / 255.f, alpha / 255.f), Vector2(s, t)));
		stringStream.clear();
	}

	for (int faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{
		std::getline(file, line);
		stringStream << line;

		int faceVertexCount;
		int facePoint1, facePoint2, facePoint3;
		stringStream >> faceVertexCount >> facePoint1 >> facePoint2 >> facePoint3;
		Face face(facePoint1, facePoint2, facePoint3);
		mesh->AddFace(face);

		stringStream.clear();
	}

	return mesh;
}
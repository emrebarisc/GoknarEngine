#ifndef __CONTENT_H__
#define __CONTENT_H__

#include "Core.h"

#include <string>

class GOKNAR_API Content
{
public:
	Content() :
		path_("")
	{

	}

	Content(const std::string& path) :
		path_(path)
	{

	}

	virtual ~Content() {}

	virtual void PreInit() {}
	virtual void Init() {}
	virtual void PostInit() {}

	const std::string& GetPath() const
	{
		return path_;
	}

	void SetPath(const std::string& path)
	{
		path_ = path;
	}

private:
	std::string path_;
};

#endif
#pragma once
#include "stb_image.h"
#include "Image.h"
#include <unordered_map>
class Textures
{
public:
	std::unordered_map<std::string, Walnut::Image*> images;
	void init();
	Textures();
	~Textures();
};


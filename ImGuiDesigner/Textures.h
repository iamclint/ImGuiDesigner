#pragma once
#include "..\..\Walnut\vendor\stb_image\stb_image.h"
#include "../includes/Image.h"
#include <unordered_map>
class Textures
{
public:
	std::unordered_map<std::string, Walnut::Image*> images;
	void init();
	Textures();
	~Textures();
};


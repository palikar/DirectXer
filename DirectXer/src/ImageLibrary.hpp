#pragma once

#include <stb_image.h>
#include <fmt/format.h>
#include <string>
#include <string_view>

#include "Graphics.hpp"

/*
  -> Loads images and puts them into textures
  -> One texture can contain several images and this
class keeps info on where exactly are those images - in which texture
and where exectly in this texture
  -> If the image that is to be loaded is "too big", we create a texure
for the whole image and do not put it into some shared texture
*/
class TextureLibrary
{
  public:

	std::string_view ResourcesPath;

	void Init(std::string_view t_ResourcesPath)
	{
		ResourcesPath = t_ResourcesPath;
	}

	void LoadImage(std::string_view t_ImagePath)
	{}
	


};

#pragma once

#include <vector>
namespace vis2 {


	enum cutType {
		NONE = 0,
		BOX = 1,
		BALL = 2,
		PLANE = 3 //not sure if necessary
		, TUBE = 4
		, WINDOW = 5
	};

	enum TexType {
		_NONE = 0,
		MATERIAL = 1,
		CHECKERED = 2,
		TEXTURE,
		NORMALS
	};

	enum shaderSetting
	{
		PHONG, FLAT, WIRE
	};


	/*
	const static vector<string> vecCutTypes = { "none", "box", "ball", "plane" };
	const static vector<string> texturingModes = { "none" , "checkered", "texture", "normals" };
	*/
}
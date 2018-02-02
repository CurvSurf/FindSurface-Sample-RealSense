#pragma once
#include <map>

struct ShaderSource {
	
	static std::map<const char*, const char*> vs_src;
	static std::map<const char*, const char*> fs_src;

	static void init();
};
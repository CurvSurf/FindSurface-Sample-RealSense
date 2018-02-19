#pragma once
#include <map>
#include <string>

struct ShaderSource {
	
	static std::map<std::string, const char*> vs_src;
	static std::map<std::string, const char*> fs_src;

	static void init();
};
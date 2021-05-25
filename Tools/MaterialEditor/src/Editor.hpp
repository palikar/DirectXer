#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <fstream>
#include <stb_image.h>
#include <fmt/format.h>
#include <filesystem>
#include <Glm.hpp>

static std::vector<unsigned char> LoadFile(const std::string &t_filename)
{
    std::ifstream infile(t_filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary);

	auto size = infile.tellg();
    infile.seekg(0, std::ios::beg);

	std::vector<unsigned char> v(static_cast<size_t>(size));
	infile.read((char*)&v[0], static_cast<std::streamsize>(size));

	return v;
}

static std::string LoadFileIntoString(const std::string &t_filename)
{
	auto v = LoadFile(t_filename);
	return std::string(v.begin(), v.end());
}

static std::vector<std::string> SplitLine(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;

    size_t last = 0;
    size_t next = 0;

    while ((next = s.find(delimiter, last)) != std::string::npos)
    {
        tokens.push_back(s.substr(last, next - last));
        last = next + 1;
    }
    tokens.push_back(s.substr(last));
    return tokens;
}

static glm::i32vec3 GetIndexData(std::string part)
{
	auto parts = SplitLine(part, '/');
	return glm::i32vec3{
		std::stoi(parts[0].c_str()) - 1,
		std::stoi(parts[1].c_str()) - 1,
		std::stoi(parts[2].c_str()) - 1};
}

static bool StartsWith(const std::string &str, const std::string &prefix)
{
	return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
{
    if (from.empty()) return str;

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // In case 'to' contains 'from', like
		// replacing 'x' with 'yx'
    }
    return str;
}

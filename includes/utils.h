#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <exception>
#include <unistd.h>
#include <fcntl.h>
#include <cctype>

typedef std::map<std::string, std::string> strstrMap;
typedef std::map<int, bool> intboolMap;
typedef std::map<int, std::string> intstrMap;
typedef std::map<std::string, int> strintMap;

void	p_error(std::string err);

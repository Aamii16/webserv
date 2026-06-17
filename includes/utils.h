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
#include "config.h"
#include <sys/stat.h>
#include <climits>
#include <utility>
#include <dirent.h>

std::string	num_to_str(long long num);
void	p_error(std::string err);

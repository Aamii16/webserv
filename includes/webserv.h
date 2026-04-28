#pragma once

#include "utils.h"
#include "config.h"
#include "httpRequest.hpp"

#define MAX_URI_LENGTH 2048
#define MAX_HEADER_FIELD_LENGTH 4096
typedef	enum codes
{
	METHOD_NOT_ALLOWED = 405,
	URI_TOO_LONG = 414,
	BODY_TOO_LONG = 413,
	LENGTH_REQUIRED = 411,
	HEADER_FIELD_TOO_LARGE = 431,
	UNSUPPORTED_HTTP = 505
}	codes;
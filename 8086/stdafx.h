// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#include "targetver.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <conio.h>
#include <exception>
#include <cassert>
#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <filesystem>

#include "Common.h"

#include "../Common/json.hpp"
#include "../Common/inipp.h"
#include "../Common/Logger.h"
#include "../Common/StringUtil.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef LoadCursor

#pragma warning( disable:4251 )
#pragma warning ( disable:26429 )
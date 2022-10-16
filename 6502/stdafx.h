// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#include "targetver.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif

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

#include <json.hpp>
#include <inipp.h>
#include <Logger.h>
#include <StringUtil.h>
#include <FileUtil.h>
#include <CPU/CPUCommon.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef LoadCursor

#pragma warning( disable:4251 )
#pragma warning ( disable:26429 )

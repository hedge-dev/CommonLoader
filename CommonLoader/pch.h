// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.
#pragma once

#ifndef PCH_H
#define PCH_H

#define CMN_LOADER_IMPL

#define NOMINMAX
#include <Windows.h>
#include <ShlObj.h>
#include <mscoree.h>
#include <metahost.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <charconv>

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <nlohmann/json.hpp>

#include "CommonLoader.h"
#include "ApplicationStore.h"
#include "SigScanner.h"
#include "externals/ini.h"
#include "externals/xxhash.h"

#include "Registry.h"
#include "Environment.h"
#include "clrhost/dotnet.h"

#endif // PCH_H

#pragma once
/*
MIT License, Copyright (c) 2025 @chcs1013
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef __cplusplus
#error "Must be included in C++"
#endif

// w32use.hpp


#ifndef W32USE_NO_CORE
// Use core
#include "./Core/w32def.hpp"
using w32oop::w32Object;
using namespace w32oop::def;
using w32oop::w32oop_exception;
#endif


#ifndef W32USE_NO_CONCURRENCY
// Use concurrency
#include "./Concurrency/Process.hpp"
#include "./Concurrency/Thread.hpp"
#include "./Concurrency/EventObject.hpp"
using namespace w32oop::concurrency;
#endif


#ifndef W32USE_NO_GUI
// Use GUI
#include "./GUI/Window.hpp"
using namespace w32oop::ui;
using namespace w32oop::ui::foundation;

#ifndef W32USE_PUBLIC_API_ONLY
#include "./GUI/InternalMessageBox.hpp"
#endif
#endif


#ifndef W32USE_NO_IO
// Use IO
#include "./IO/File.hpp"
#include "./IO/PipeServer.hpp"
#include "./IO/PipeClient.hpp"
using namespace w32oop::io;
#endif


#ifndef W32USE_NO_NETWORK
// Use Network
#include "./Network/WinINetTask.hpp"
#include "./Network/DownloadTask.hpp"
#endif


#ifndef W32USE_NO_SYSTEM
// Use System
#include "./System/Service.hpp"
#include "./System/Registry.hpp"
using w32oop::system::Service;
using w32oop::system::ServiceManager;
using w32oop::system::RegistryKey;
using w32oop::system::RegistryValue;
#ifndef W32USE_NO_HOOKS
// Use Hooks
#include "./System/Hooks.hpp"
#endif
#endif


#ifndef W32USE_NO_UTILITY
// Use Utility
#include "./Utility/RAII.hpp"
#include "./Utility/Memory.hpp"
#include "./Utility/StringUtil/converts.hpp"
#include "./Utility/StringUtil/encodings.hpp"
#include "./Utility/StringUtil/operations.hpp"
#ifndef W32USE_NO_ERRCHECK
#include "./Utility/ErrCheck.hpp"
using w32oop::util::ErrorChecker;
#endif
#endif




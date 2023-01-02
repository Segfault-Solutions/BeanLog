/*
    BeanLog is a minimal logging library for graphical applications.

    Copyright (C) 2023 GRX78FL (at) Segfault Solutions
    <https://github.com/GRX78FL>
    <https://github.com/Segfault-Solutions>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

/* Enforce /std:C++20 or above. */
#if _MSVC_LANG < 202002L
#error "C++20 or later is needed to use BeanLog."
#endif

/* Only DEBUG builds should include the following code. */
#ifdef _DEBUG

#include <Windows.h>

#include <chrono>
#include <format>
#include <iostream>

enum BeanLogLevel
{
    trace,
    info,
    warn,
    fail
};

class BeanLog
{
public:
    void SetLogLevel(BeanLogLevel lvl)
    {
        _logLevel = lvl;
    }

    /* Prints the properly formatted ansi debug message from the application. */
    template <typename... VA_ARGS>
    void LogA(BeanLogLevel lvl, const std::string_view& fmt, VA_ARGS&&... args)
    {
        if (lvl >= _logLevel && lvl >= 0 && lvl <= fail)
        {
            SetConsoleTextAttribute(_outHandle, _colors[lvl]);
            std::cout << std::format("[APP] [{}]: {}.",
                                     std::chrono::zoned_time{std::chrono::current_zone(), _sysClock.now()}.get_local_time(),
                                     std::vformat(fmt, std::make_format_args(args...)))
                      << std::endl;

            /* Prints an error message from the system if one is available. */
            if ((_tmpError = GetLastError()) != 0 && _tmpError != _lastError)
            {
                _PrintSystemError(_tmpError);
                _lastError = _tmpError;
            }
        }
    }

    /* Prints the properly formatted unicode debug message from the application. */
    template <typename... VA_ARGS>
    void LogW(BeanLogLevel lvl, const std::wstring_view& fmt, VA_ARGS&&... args)
    {
        if (lvl >= _logLevel && lvl >= 0 && lvl <= fail)
        {
            SetConsoleTextAttribute(_outHandle, _colors[lvl]);
            std::wcout << std::format(L"[APP] [{}]: {}.",
                                      std::chrono::zoned_time{std::chrono::current_zone(), _sysClock.now()}.get_local_time(),
                                      std::vformat(fmt, std::make_wformat_args(args...)))
                       << std::endl;

            /* Prints an error message from the system if one is available. */
            if ((_tmpError = GetLastError()) != 0 && _tmpError != _lastError)
            {
                _PrintSystemError(_tmpError);
                _lastError = _tmpError;
            }
        }
    }

    static BeanLog& GetInstance(void)
    {
        static BeanLog Logger;
        return Logger;
    }

private:
    /* Translates the last error to text and print it to the console. */
    void __inline _PrintSystemError(DWORD lastError)
    {
        DWORD status = 0;
        wchar_t* osMessage = nullptr;

        status = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                nullptr, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (wchar_t*) &osMessage, 0,
                                nullptr);

        std::wcout << std::format(L"[SYS] [{}]: {}",
                                  std::chrono::zoned_time{std::chrono::current_zone(), _sysClock.now()}.get_local_time(),
                                  osMessage);

        LocalFree(osMessage);
    }

protected:
    /* Allocates a console, opens stdout and enables colored output. */
    BeanLog()
    {
        _isConsoleAllocated = AllocConsole();
        if (!_isConsoleAllocated)
        {
            MessageBoxW(nullptr, L"Failed to allocate a console.", L"BeanLog::BeanLog", MB_ICONERROR | MB_OK);
            return;
        }

        freopen_s(&_fConOut, "CONOUT$", "w", stdout);
        if (!_fConOut)
        {
            MessageBoxW(nullptr, L"Failed to reopen STDOUT.", L"BeanLog::BeanLog", MB_ICONERROR | MB_OK);
            return;
        }
        else
        {
            _isStdoutOpen = true;
        }

        _outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (_outHandle == INVALID_HANDLE_VALUE)
        {
            MessageBoxW(nullptr, L"Failed to get STD_OUTPUT_HANDLE.\nOutput won't be colored.", L"BeanLog::BeanLog", MB_ICONWARNING | MB_OK);
            return;
        }
    }

    /* Deallocates the console and closes stdout. */
    ~BeanLog()
    {
        if (_isStdoutOpen)
        {
            if (fclose(_fConOut) == EOF)
            {
                MessageBoxW(nullptr, L"Failed to close STDOUT.", L"BeanLog::~BeanLog", MB_ICONERROR | MB_OK);
            }
        }

        if (_isConsoleAllocated)
        {
            if (!FreeConsole())
            {
                MessageBoxW(nullptr, L"Failed to free the console.", L"BeanLog::~BeanLog", MB_ICONERROR | MB_OK);
            }
        }
    }

public:
    BeanLog(const BeanLog&) = delete;
    BeanLog(BeanLog&&) = delete;
    BeanLog& operator=(const BeanLog&) = delete;
    BeanLog& operator=(BeanLog&&) = delete;

private:
    int _logLevel = BeanLogLevel::trace;
    DWORD _lastError = 0;
    DWORD _tmpError = 0;
    bool _isConsoleAllocated = false;
    bool _isStdoutOpen = false;
    FILE* _fConOut = nullptr;
    std::chrono::system_clock _sysClock;
    HANDLE _outHandle = INVALID_HANDLE_VALUE;
    int _colors[4] = {/* dim white */ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
                      /* bright green */ FOREGROUND_GREEN | FOREGROUND_INTENSITY,
                      /* bright yellow */ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
                      /* bright red */ FOREGROUND_RED | FOREGROUND_INTENSITY};
};

/* Maximizing ease of use as Singletons aren't exactly 'pretty'. */

#define bean_set_loglevel(LOG_LEVEL) BeanLog::GetInstance().SetLogLevel(LOG_LEVEL)
#define bean_trace_a(FORMAT_STRING, ...) BeanLog::GetInstance().LogA(BeanLogLevel::trace, FORMAT_STRING, __VA_ARGS__)
#define bean_info_a(FORMAT_STRING, ...) BeanLog::GetInstance().LogA(BeanLogLevel::info, FORMAT_STRING, __VA_ARGS__)
#define bean_warn_a(FORMAT_STRING, ...) BeanLog::GetInstance().LogA(BeanLogLevel::warn, FORMAT_STRING, __VA_ARGS__)
#define bean_fail_a(FORMAT_STRING, ...) BeanLog::GetInstance().LogA(BeanLogLevel::fail, FORMAT_STRING, __VA_ARGS__)
#define bean_trace_w(FORMAT_STRING, ...) BeanLog::GetInstance().LogW(BeanLogLevel::trace, FORMAT_STRING, __VA_ARGS__)
#define bean_info_w(FORMAT_STRING, ...) BeanLog::GetInstance().LogW(BeanLogLevel::info, FORMAT_STRING, __VA_ARGS__)
#define bean_warn_w(FORMAT_STRING, ...) BeanLog::GetInstance().LogW(BeanLogLevel::warn, FORMAT_STRING, __VA_ARGS__)
#define bean_fail_w(FORMAT_STRING, ...) BeanLog::GetInstance().LogW(BeanLogLevel::fail, FORMAT_STRING, __VA_ARGS__)

#elif NDEBUG

/*
    In RELEASE mode, these macros evaluate to nothing, the only thing
    left is the `;` semicolon at the end of a statement in the translation
    units that make use of this library.
*/

#define bean_set_loglevel(LOG_LEVEL)
#define bean_trace_a(FORMAT_STRING, ...)
#define bean_info_a(FORMAT_STRING, ...)
#define bean_warn_a(FORMAT_STRING, ...)
#define bean_fail_a(FORMAT_STRING, ...)
#define bean_trace_w(FORMAT_STRING, ...)
#define bean_info_w(FORMAT_STRING, ...)
#define bean_warn_w(FORMAT_STRING, ...)
#define bean_fail_w(FORMAT_STRING, ...)

#endif

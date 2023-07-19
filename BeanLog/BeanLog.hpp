/*
    Copyright (C) 2023 GRX78FL (at) Segfault Solutions - MIT License.
    <https://github.com/GRX78FL>
    <https://github.com/SegfaultSolutions>

    BeanLog is a minimal logging library for graphical applications.
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
#include <mutex>

enum BeanLogLevel
{
    trace,
    info,
    warn,
    fail,
    max
};

class BeanLog
{
public:
    static BeanLog& GetInstance(void)
    {
        static BeanLog Logger;
        return Logger;
    }

    void SetLogLevel(BeanLogLevel lvl)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _logLevel = lvl;
    }

    template <typename... ARGS>
    void Log(BeanLogLevel lvl, DWORD syserr, const wchar_t* fmt, ARGS... args)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        // Make sure to filter messages based on severity
        if (lvl < _logLevel)
        {
            SetLastError(0);
            return;
        }

        // Select the correct color for the output
        switch (lvl)
        {
            default:
            case BeanLogLevel::trace:
            {
                _color1 = L"\x1B[30;107m";
                _color2 = L"\x1B[0;97m ";
                break;
            }
            case BeanLogLevel::info:
            {
                _color1 = L"\x1B[30;102m";
                _color2 = L"\x1B[0;92m ";
                break;
            }
            case BeanLogLevel::warn:
            {
                _color1 = L"\x1B[30;103m";
                _color2 = L"\x1B[0;93m ";
                break;
            }
            case BeanLogLevel::fail:
            {
                _color1 = L"\x1B[30;101m";
                _color2 = L"\x1B[0;91m ";
                break;
            }
        }

        // Format application message
        std::wcout << _color1
                   << std::format(L"[APP] [{}]:", _GetTime())
                   << _color2
                   << std::vformat(fmt, std::make_wformat_args(std::forward<ARGS>(args)...))
                   << L"\x1B[0m" << std::endl;

        // Format system error
        if (syserr)
        {
            std::wcout << _color1
                       << std::format(L"[SYS] [{}]:", _GetTime())
                       << _color2
                       << std::error_code(syserr, std::system_category()).message().c_str()
                       << L"\x1B[0m" << std::endl;
            SetLastError(0);
        }
    }

private:
    std::chrono::local_time<std::chrono::system_clock::duration> _GetTime(void)
    {
        return std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()}.get_local_time();
    }

protected:
    /* Allocates a console, opens stdout and enables colored output. */
    BeanLog()
    {
        // Check if there's a console already attached to the current process
        if ((_outHandle = GetStdHandle(STD_OUTPUT_HANDLE)) == nullptr)
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

        if (!GetConsoleMode(_outHandle, &_mode))
        {
            MessageBoxW(nullptr, L"Failed to get the console mode.\nOutput won't be colored.", L"BeanLog::BeanLog", MB_ICONWARNING | MB_OK);
            return;
        }

        if (!SetConsoleMode(_outHandle, _mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        {
            MessageBoxW(nullptr, L"Failed to set the console mode.\nOutput won't be colored.", L"BeanLog::BeanLog", MB_ICONWARNING | MB_OK);
            return;
        }
    }

    /* Deallocates the console and closes stdout. */
    ~BeanLog()
    {
        // Restore the changes made in order to display colors, useful if the current process is a console application
        SetConsoleMode(_outHandle, _mode);

        // Only close stdout if BeanLog opened it
        if (_isStdoutOpen)
        {
            if (fclose(_fConOut) == EOF)
            {
                MessageBoxW(nullptr, L"Failed to close STDOUT.", L"BeanLog::~BeanLog", MB_ICONERROR | MB_OK);
            }
        }

        // Only free the console if BeanLog allocated it
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
    bool _isConsoleAllocated = false;
    bool _isStdoutOpen = false;
    FILE* _fConOut = nullptr;
    HANDLE _outHandle = INVALID_HANDLE_VALUE;
    std::mutex _mutex;
    const wchar_t* _color1 = nullptr;
    const wchar_t* _color2 = nullptr;
    DWORD _mode{};
};

/* Maximizing ease of use as Singletons aren't exactly 'pretty'. */

#define bean_set_loglevel(LOG_LEVEL) BeanLog::GetInstance().SetLogLevel(LOG_LEVEL)
#define bean_trace(FORMAT_STRING, ...) BeanLog::GetInstance().Log(BeanLogLevel::trace, GetLastError(), FORMAT_STRING, __VA_ARGS__)
#define bean_info(FORMAT_STRING, ...) BeanLog::GetInstance().Log(BeanLogLevel::info, GetLastError(), FORMAT_STRING, __VA_ARGS__)
#define bean_warn(FORMAT_STRING, ...) BeanLog::GetInstance().Log(BeanLogLevel::warn, GetLastError(), FORMAT_STRING, __VA_ARGS__)
#define bean_fail(FORMAT_STRING, ...) BeanLog::GetInstance().Log(BeanLogLevel::fail, GetLastError(), FORMAT_STRING, __VA_ARGS__)

#elif NDEBUG

/*
    In RELEASE mode, these macros evaluate to nothing, the only thing
    left is the `;` semicolon at the end of a statement in the translation
    units that make use of this library.
*/

#define bean_set_loglevel(LOG_LEVEL)
#define bean_trace(FORMAT_STRING, ...)
#define bean_info(FORMAT_STRING, ...)
#define bean_warn(FORMAT_STRING, ...)
#define bean_fail(FORMAT_STRING, ...)

#endif

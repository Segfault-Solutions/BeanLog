# BeanLog::About

BeanLog is a minimal logging library for graphical applications initially intended for [WinBeans](https://github.com/Segfault-Solutions/WinBeans) however, 
it can be used by any application that needs a logging system without the headaches of learning an entire new API.

# BeanLog::Usage

Using this library, *which is essentially a wrapper for std::w/cout, std::format and FormatMessage*, is as simple as:

```c++
#include <BeanLog/BeanLog.hpp>

int wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    /* 
        The available log levels are `trace`, `info`, `warn`, and `fail`.
        Setting a log level ensures that only messages of <BeanLogLevel> severity
        or greater are displayed. `BeanLogLevel::trace` is the default.
    */
    bean_set_loglevel(trace);
    
    /* 
        Logging messages to the console, (yes, this is taken care of automatically),
        looks like so:
    */
    
    bean_trace_a("one {}", 1);
    bean_info_a("one {}, two {}", 1, 'B');
    
    /*
        As per Win32 tradition, there's a char and wchar version of each logging macro:
    */
    
    bean_warn_w(L"one {}, two {}, three {}", 1, 'B', 3);
    bean_fail_w(L"one {}, two {}, three {}, {}", 1, 'B', 3, L"four");
    
    /*
        When an error occurs and Win32 calls `SetLastError` to notify your application
        that something went wrong, BeanLog will try to display a human readable version
        of the error code. 
        To induce an error we can try the following.
    */
    
    if (!GetProcessId(0))
    {
        bean_fail_a("{}", "something happened");
    }
    return EXIT_SUCCESS;
}
```

Here's what the sample program above will output in DEBUG mode (only!):

![image](https://user-images.githubusercontent.com/20095224/210196381-ce5c7df5-a625-4eed-9fb2-e5db9cde94f3.png)

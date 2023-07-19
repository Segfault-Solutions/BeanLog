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
    
    bean_trace(L"one {}", 1);
    bean_info(L"one {}, two {}", 1, 'B');   
    bean_warn(L"one {}, two {}, three {}", 1, 'B', 3);
    bean_fail(L"one {}, two {}, three {}, {}", 1, 'B', 3, L"four");
    bean_info(L"none");
    
    /*
        When an error occurs and Win32 calls `SetLastError` to notify your application
        that something went wrong, BeanLog will try to display a human readable version
        of the error code. 
        To induce an error we can try the following.
    */
    
    if (!GetProcessId(0))
    {
        bean_fail("Something went wrong!");
    }
    return EXIT_SUCCESS;
}
```

Here's what the sample program above will output in DEBUG mode (only!):

![image](https://github.com/GRX78FL/libdit/assets/20095224/42ee0263-2b5a-49cc-b00e-c28739cc684c)

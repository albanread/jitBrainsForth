// quit.cpp


#include <iostream>
#include "quit.h"
#include "interpreter.h"

// Define the WINAPI macro
#ifndef WINAPI
#define WINAPI __stdcall
#endif
typedef unsigned long long ULONG_PTR;
typedef unsigned long ULONG;
typedef long LONG;  // Define LONG as long
typedef void* PVOID;  // Define PVOID as a pointer to void
extern "C" {
    // Define the EXCEPTION_RECORD structure
    typedef struct _EXCEPTION_RECORD {
        LONG ExceptionCode;
        ULONG ExceptionFlags;
        struct _EXCEPTION_RECORD* ExceptionRecord;
        PVOID ExceptionAddress;
        ULONG NumberParameters;
        ULONG_PTR ExceptionInformation[15];
    } EXCEPTION_RECORD, *PEXCEPTION_RECORD;

    typedef struct _EXCEPTION_POINTERS {
        // Typically includes fields for ExceptionRecord and ContextRecord
        // Let's forward declare these as well
        struct _EXCEPTION_RECORD* ExceptionRecord;
        void* ContextRecord;
    } EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

    typedef long NTSTATUS;
    typedef NTSTATUS(WINAPI* PEXCEPTION_FILTER)(PEXCEPTION_POINTERS ExceptionInfo);
    PVOID WINAPI AddVectoredExceptionHandler(ULONG First, PEXCEPTION_FILTER Handler);
    ULONG WINAPI RemoveVectoredExceptionHandler(PVOID Handle);
    short GetAsyncKeyState(int vKey);

}


class AccessViolationException : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return "Caught an access violation (0xC0000005) exception";
    }
};
#define EXCEPTION_ACCESS_VIOLATION 0xC0000005
#define EXCEPTION_CONTINUE_SEARCH 1
LONG WINAPI VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
        throw AccessViolationException();
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

#define VK_ESCAPE 0x1B
bool escapePressed()
{
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        return true;
    }
    return false;
}




void Quit()
{
    PVOID handler = AddVectoredExceptionHandler(1, VectoredHandler);

    while (true)
    {
        try
        {
            interactive_terminal();
        }
        catch (const AccessViolationException& e)
        {
            std::cerr << e.what() << std::endl;
            // Reset context and stack as required
        }
        catch (const std::exception& e)
        {
            std::cerr << "Runtime error: " << e.what() << std::endl;
            // Reset context and stack as required
        }
    }

    RemoveVectoredExceptionHandler(handler);
}
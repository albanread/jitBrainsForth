// quit.h
#ifndef QUIT_H
#define QUIT_H
#include <csetjmp>
#include <bits/std_thread.h>
std::thread Quit();  // Declaration of Quit function
bool escapePressed();
static jmp_buf jumpBuffer;
#endif // QUIT_H
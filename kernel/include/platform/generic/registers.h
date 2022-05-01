#ifdef __i386__
#include <platform/x86/registers.h>
#elif __arm__
#include <platform/aarch32/registers.h>
#elif __aarch64__
#include <platform/aarch64/registers.h>
#endif
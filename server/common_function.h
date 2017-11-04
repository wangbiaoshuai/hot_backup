#ifndef COMMON_FUNCTION_H_
#define COMMON_FUNCTION_H_

#include <string>

extern "C"
{
    std::string GetCurrentIp();
    std::string GetCurrentPath();
};
#endif // COMMON_FUNCTION_H

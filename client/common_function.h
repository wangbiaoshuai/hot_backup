#ifndef COMMON_FUNCTION_H_
#define COMMON_FUNCTION_H_

#include <string>

extern "C"
{
    std::string GetCurrentIp();
    std::string GetCurrentPath();
    int ProcIsExist(const std::string& proc_name);
};
#endif // COMMON_FUNCTION_H

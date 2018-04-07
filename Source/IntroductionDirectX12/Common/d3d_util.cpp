
#include "d3d_util.h"

#include <comdef.h>

DxException::DxException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber)
    : errorCode_(hr),
    functionName_(functionName),
    filename_(filename),
    lineNumber_(lineNumber)
{
}

std::string DxException::ToString(void) const
{
    _com_error err(errorCode_);
    std::string msg = err.ErrorMessage();

    return functionName_ 
        + " failed in " 
        + filename_ 
        + "; line " 
        + std::to_string(lineNumber_)
        + "; error: "
        + msg;
}

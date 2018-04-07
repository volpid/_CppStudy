
#ifndef __D3D_UTIL__H__
#define __D3D_UTIL__H__

#include <d3d12.h>
#include <string>

class DxException
{
public:
    DxException(void) = default;
    DxException(HRESULT hr, const std::string& functionName, const std::string& filename, int lineNumber);

    std::string ToString(void) const;

    HRESULT errorCode_ = S_OK;
    std::string functionName_;
    std::string filename_;
    int lineNumber_ = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)    \
    {   \
        HRESULT hr__ = (x); \
        std::string fn = __FILE__;  \
        if (FAILED(hr__))   \
        {   \
            throw DxException(hr__, #x, fn, __LINE__);  \
        }   \
    }
#endif /**/

#ifndef ReleaseCom
#define ReleaseCom(x)   \
    {   \
        if (x) \
        {   \
            x->Release();   \
            x = 0;  \
        }   \
    }
#endif /**/

#endif /*__D3D_UTIL__H__*/
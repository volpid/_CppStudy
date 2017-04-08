
#define EXPORT_DLL
#include <dynamiclib/thread_attach_detach.h>
#include "thread_attach_detach_internal.h"

bool g_loggingAttachDetach = true;

//-----------------------------------------------------------------------------------
/*export*/
void StopThreadLogging(void)
{
    g_loggingAttachDetach = false;
}
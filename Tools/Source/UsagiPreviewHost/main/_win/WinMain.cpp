/****************************************************************************
//  Usagi preview host Win32 entry point.
****************************************************************************/
#include "..\..\PreviewHost.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
    UNREFERENCED_PARAMETER(previousInstance);
    UNREFERENCED_PARAMETER(commandLine);

    PreviewHost host;
    return host.Run(instance, showCommand);
}

#include <windows.h>
#pragma comment(lib,"Q:/ToolKit/VMProtect/Lib/COFF/VMProtectSDK32.lib")
#include "Q:/ToolKit/VMProtect/Include/C/VMProtectSDK.h"

int main()
{
    DebugBreak();   //����ϵͳ�ϵ�ֱ��ͣ���������⻯���
    VMProtectBegin("Test");    //VMProtect SDK�������
    if (MessageBoxA(0,"�������ѡ���ǡ�������OK:","ѡ��",MB_YESNO)==IDYES)
    {
        MessageBoxA(0,"OK","OK",MB_OK|MB_ICONINFORMATION);
    }else
    {
        MessageBoxA(0,"ERROR","ERROR",MB_OK|MB_ICONERROR);
    }
    VMProtectEnd();
    DebugBreak();
    return 0;
}


#include <windows.h>
#pragma comment(lib,"Q:/ToolKit/VMProtect/Lib/COFF/VMProtectSDK32.lib")
#include "Q:/ToolKit/VMProtect/Include/C/VMProtectSDK.h"

int main()
{
    DebugBreak();   //����ϵͳ�ϵ�ֱ��ͣ���������⻯���
    VMProtectBegin("Test");    //VMProtect SDK�������
    __asm {
        mov ecx,0xE0001234
        mov ebx,0xF0004567
lab_again:
        mov edi,0x12121212//����ͨ��������ѹջ����¶����
        xor ecx,ebx		//xorָ��������ǵ�Ŀ��ָ�������������Щ�����֧���������ָ��
        mov edi,0x34343434
        jmp lab_again
}
    VMProtectEnd();
    DebugBreak();
    return 0;
}



#include <windows.h>

#pragma comment(lib,"Q:/ToolKit/WinLicense/WinlicenseSDK/Lib/COFF/WinlicenseSDK32.lib")
#pragma comment(lib,"Q:/ToolKit/WinLicense/WinlicenseSDK/Lib/COFF/SecureEngineSDK32.lib")
#include "Q:/ToolKit/WinLicense/WinlicenseSDK/Include/C/WinlicenseSDK.h"
#include "Q:/ToolKit/WinLicense/WinlicenseSDK/Include/C/SecureEngineSDK.h"
int main()
{
    DebugBreak();
    VM_START
        __asm {
            mov edi,0x12345678
lab_again:
            push edi	//��ΪWinlicense��ƽջ�ģ��������ͨ��������ѹջ����¶����
            xor eax,eax		//xorָ��������ǵ�Ŀ��ָ������������Ǹ������֧���������ָ��
            pop edi		//ͨ��ջ�仯����λ���Ǵ���Ŀ�ʼ�ͽ���
            jmp lab_again
    }
    VM_END
        DebugBreak();
    return 0;
}

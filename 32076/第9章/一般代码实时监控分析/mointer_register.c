#include <windows.h>
#include "internal.h"
//�������������ʵʱ��ط���
//��ؼĴ������ʽű�

//����ָ��ִ�к�ص�
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	longptr val = vmexec_getReg(cpu,VM_REG_RAX);
	if (val == 0xBB40E64E)
	{
		addrprintf((long)rip,"EAX=%08X",val);
	}
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}
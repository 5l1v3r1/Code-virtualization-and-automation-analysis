#include <windows.h>
#include "internal.h"
//�������������ʵʱ��ط���
//��ؼĴ������ʽű�

//����ָ��ִ�к�ص�
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	vaddr lpmem;
	int szmem;//���ǻ�ȡ���һ��ָ����ڴ������Ϣ
	int mode = vmexec_getLastMemAcc(cpu,&lpmem,&szmem,0);
	if (mode != 0 && !(mode & memacc_flag_stack))
	{		//���ݷ��ʵ�ַ���˲���¼
		//__asm__("int3");
		longptr val = vmexec_getReg(cpu,VM_REG_RDI);
		if (lpmem >= val && lpmem <= val+0x100)
		{
			longptr val2 = vmexec_getReg(cpu,VM_REG_RAX);
			if (val2 >= 0xC && val2 < 0x1000)
			{
				addrprintf((long)rip,"Access to Context:%08X->eax=%08X",lpmem,val2);
			}
		}
	}
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}
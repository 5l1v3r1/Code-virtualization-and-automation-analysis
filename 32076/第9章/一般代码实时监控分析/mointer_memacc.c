#include <windows.h>
#include "internal.h"
//�������������ʵʱ��ط���
//����ڴ���ʽű�

//����ָ��ִ�к�ص�
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	vaddr lpmem;
	int szmem;//���ǻ�ȡ���һ��ָ����ڴ������Ϣ
	int mode = vmexec_getLastMemAcc(cpu,&lpmem,&szmem,0);
	if (mode != 0)
	{		//���ݷ��ʵ�ַ���˲���¼
		if (lpmem >= 0x408000 && lpmem <= 0x410000)
		{
			addrprintf((long)rip,"acc MEM:%08X",lpmem);
		}
	}
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}
#include <windows.h>
#include "internal.h"
//�������������ʵʱ��ط���

//����ָ��ִ��ǰ�ص�
int		vmexec_before_step(void* cpu,void* rip,void* inst)
{	//���Ǽ򵥵��ж��Ƿ�ripv��ϵͳ�����ڣ���������Ǿ���ͣ׷��
	int oplen;
	char* lpop = vmexec_getInstOpByte(inst,&oplen);
	if (!lpop)
		return vmexec_ret_normal;
	if ((unsigned char)*lpop == 0xE8)
	{
		addrprintf((unsigned long)rip,"call->target:%08X",(unsigned long)rip+*(long*)(lpop+1)+5);
	}
	return vmexec_ret_normal;
}
//����ָ��ִ�к�ص�
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_keep;
}
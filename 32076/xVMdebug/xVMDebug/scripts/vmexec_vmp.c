#include <windows.h>
#include "internal.h"
//������������и߼��Զ�׷�ٷ�������

//����ָ��ִ��ǰ�ص�
int		vmexec_before_step(void* cpu,void* rip,void* inst)
{	//���Ǽ򵥵��ж��Ƿ�ripv��ϵͳ�����ڣ���������Ǿ���ͣ׷��
	unsigned long ripv = (unsigned long)rip;
	if (ripv >= 0x60000000 && ripv <= 0x7F000000)
		return vmexec_ret_finished;
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
#include <windows.h>
#include "internal.h"
#include "codeblock.h"

//����ָ��ִ�к�ص�
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{

	cb_SeqExecute* se = vmexec_lastBKExec(cpu);
	if (se && se->rip == (longptr)rip)	//�ж��Ƿ��Ǵ�������
		se->ip = vmexec_getReg(cpu,VM_REG_RSI)+1;	//+1����

	//��������ʹ�ø߼����Զϵ㼼������ͣ����һ��OP�����֧�����
	int szop;
	char* lpop = vmexec_getInstOpByte(inst,&szop);
	if (lpop)
	{	//�����������ret ��ʽ����ת������Ϊ��OP�����֧����
		if (*(unsigned char*)lpop == 0xC2)
			return vmexec_ret_finished;	//�����˳������ָ��
	}
	return vmexec_ret_normal;
}


int main(int argc,char** argv)
{	
	//�������Ƿ���ȫ�ֽű���ǣ������ű�ֻ���ڵ�һ�α���
	return main_ret_global;
}
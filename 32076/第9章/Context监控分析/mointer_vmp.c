#include <windows.h>
#include "internal.h"
//�������������ʵʱ��ط���
//��ؼĴ������ʽű�

//����ָ��ִ�к�ص�
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	vaddr lpmem;
	int szmem;//���ǻ�ȡ���һ��ָ����ڴ������Ϣ
	longptr memval;
	int mode = vmexec_getLastMemAcc(cpu,FALSE,&lpmem,&szmem,&memval);
	if (mode != 0)
	{		//���ݷ��ʵ�ַ���˲���¼
		//__asm__("int3");	//���ǵĽű�֧�ֻ�࣬�������ڵ�����ͣ
		//�øó�ʽ��EDIָ��Context,ESIָ��OPCODE,EBPָ��vStack
		longptr val = vmexec_getReg(cpu,VM_REG_RDI);
		//����Contextû����ջ���У���Ϊ���Ǽ�����0x1000��СҲ��Ϊ��
		if (lpmem >= val)
		{	//������ʷ�����Context��
			longptr ctxoft = lpmem - val;
			//����ƫ�ƣ����ƫ��С��0x1000������Ϊ����Ч��Context����
			if (ctxoft >= 0x8 && ctxoft < 0x100)
			{	//���ǽ�������Ϣ���͵�����������־����
				addrprintf((long)rip,"[%c]AC:%08X->oft=%08X ->Value:%08X",(mode & memacc_flag_read)?'R':'W', lpmem,ctxoft,memval);
			}
		}
	}
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
	return main_ret_keep;
}
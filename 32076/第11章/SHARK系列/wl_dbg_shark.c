#include <windows.h>
#include "internal.h"
#include "codeblock.h"

cb_SeqExecute* gLastSeq = 0;
int		grunState = bkrun_none;
int		gtmpStep = 0;
longptr glstBPAddr = 0;
longptr	glpContext = 0;

void accessContextRead(void* cpu,longptr rip,int oft,int size,longptr val)
{

}


void accessContextWrite(void* cpu,longptr rip,int oft,int size,longptr val)
{
	addrprintf(rip,"WC:oft=%08X ->size:%d,Value:%08X",oft,size,val);
}

int vmexec_before_step(void* cpu,void* rip,void* inst)
{
	//ȷ�����ǵĵ���״̬�����µ�
	grunState = vmexec_getRunState(cpu);
	//�õ����һ����ִ�м�¼
	cb_SeqExecute* se = vmexec_lastBKExec(cpu);
	gLastSeq = se;
	//����п�ִ�м�¼��˵������ִ���Ѿ������ִ��ѭ����
	if (se && se->rip == (longptr)rip)	//�ж��Ƿ��Ǵ�������
	{
		//�������������ж������Ƿ��Ѿ��趨�˲�����ǣ����������ڲ�����ʼʱ�趨��
		//��Ϊ����ֻ������һ���������ڲ���ȷ����һ��������Ѿ�ִ�������
		// ��������жϲ����ı������һ�������׼��ִ�У���ô������Ϊ��������һ�������
		if (gtmpStep == 0)
		{	//���û�ж��������������˲���������ô�����趨���
			if (grunState == bkrun_stepover)
				gtmpStep = 1;
		} else
		{	//�������Ǽ�⵽���������󲽹����ԣ������Ѿ��趨�˱�ǣ������Ѿ�����һ��������ڣ�
			//��ô�����˳����������ִͣ��
			if (gtmpStep == 1)
			{
				gtmpStep = 0;
				se->flags |= cbseq_flag_temp;
				return vmexec_ret_finished;
			}
		}
		if (glpContext == 0)
			glpContext = vmexec_getReg(cpu,VM_REG_RBP);
		//�������Ǽ��OPCODE�ϵ㣬��ΪOPCODEָ�����ǵ��������޷��Զ�ʶ���
		//����ж��Ƿ񴥷��ϵ��ɽű��������
		longptr* optr = (longptr*)(glpContext + 0x27);	//+0x51Ϊopcode,ÿ�����ܿ��ܲ�һ��
		se->ip = *optr;
		if (glstBPAddr != se->ip)
		{
			if (vmexec_isBreak(cpu,(void*)se->ip))
			{
				glstBPAddr = se->ip;
				return vmexec_ret_finished;
			}
		}
	}
	glstBPAddr = 0;

	return vmexec_ret_normal;
}
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
		longptr val = glpContext;
		//����Contextû����ջ���У���Ϊ���Ǽ�����0x1000��СҲ��Ϊ��
		if (lpmem >= val && lpmem <= val + 0x1000)
		{	//������ʷ�����Context��
			longptr ctxoft = lpmem - val;
			//����ƫ�ƣ����ƫ��С��0x1000������Ϊ����Ч��Context����
			if (ctxoft < 0x1000)
			{	
				if (mode & memacc_flag_read)
				{
					accessContextRead(cpu,(longptr)rip,ctxoft,szmem,memval);
				} else if (mode & memacc_flag_write)
				{
					accessContextWrite(cpu,(longptr)rip,ctxoft,szmem,memval);
				}

			}
		}
	}
	return vmexec_ret_normal;
}
int main(int argc,char** argv)
{
	return main_ret_global;
}
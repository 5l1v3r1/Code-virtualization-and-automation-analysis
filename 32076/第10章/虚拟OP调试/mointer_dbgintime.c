#include <windows.h>
#include "internal.h"
#include "codeblock.h"

cb_SeqExecute* gLastSeq = 0;
int		grunState = bkrun_none;
int		gtmpStep = 0;
longptr glstBPAddr = 0;
void accessContextRead(void* cpu,longptr rip,int oft,int size,longptr val)
{
	//�������ǿ��Ƿ������һ����ִ�м�¼
	if (!gLastSeq) return;
	//�������Ƿ�OP��֧����
	switch (gLastSeq->uuid)
	{
	case 4:// push %02s->%08s
	{
			   gLastSeq->vars[0].type = cbvar_int;
			   gLastSeq->vars[0].sqword = oft;
			   gLastSeq->vars[1].type = cbvar_int;
			   gLastSeq->vars[1].sqword = val;
	}break;
	default: break;
	}
}


void accessContextWrite(void* cpu,longptr rip,int oft,int size,longptr val)
{
	//�������ǿ��Ƿ������һ����ִ�м�¼
	if (!gLastSeq) return;
	//�������Ƿ�OP��֧����
	switch (gLastSeq->uuid)
	{
	case 1:	//pop %1	
	{	//�����pop�����֧����ô���ǽ�����ͨ�����䴫�ݸ����Բ��
		gLastSeq->vars[0].type = cbvar_int;
		gLastSeq->vars[0].sqword = oft;
		gLastSeq->vars[1].type = cbvar_int;
		gLastSeq->vars[1].sqword = val;
	}break;
	case 4:// push %02s->%08s
	{
		gLastSeq->vars[0].type = cbvar_int;
		gLastSeq->vars[0].sqword = oft;
		gLastSeq->vars[1].type = cbvar_int;
		gLastSeq->vars[1].sqword = val;
	}break;
	default:break;
	}

}


void accessVStackRead(void* cpu,longptr rip,longptr stkaddr,int size,longptr val)
{
	if (!gLastSeq) return;
	switch (gLastSeq->uuid)
	{
	case 3:	//add [esp+4],[esp] <- eflags
	{	
		gLastSeq->vars[1].type = cbvar_int;
		gLastSeq->vars[1].sqword = val;
	}break;
	case 5:
	{
		gLastSeq->vars[1].type = cbvar_int;
		gLastSeq->vars[1].sqword = val;
	}break;
	default:break;
	}
}

void accessVStackWrite(void* cpu,longptr rip,longptr stkaddr,int size,longptr val)
{
	if (!gLastSeq) return;
	switch (gLastSeq->uuid)
	{
	case 2:	//push imm
	{	//ͬ����OP��֧������Ϊpush imm��ջд���ϼ����׼ȷ
		gLastSeq->vars[0].type = cbvar_int;
		gLastSeq->vars[0].sqword = val;
	}break;
	case 3:
	{
		longptr vstk = vmexec_getReg(cpu,VM_REG_RBP);
		longptr oft = stkaddr - vstk;
		if (oft == 0)
		{
			gLastSeq->vars[2].type = cbvar_int;
			gLastSeq->vars[2].sqword = val;
		} else if (oft == 4)
		{
			gLastSeq->vars[0].type = cbvar_int;
			gLastSeq->vars[0].sqword = val;
		}
	}break;
	case 5:
	{
		gLastSeq->vars[0].type = cbvar_int;
		gLastSeq->vars[0].sqword = val;
	}break;
	default:break;
	}
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
		//�������Ǽ��OPCODE�ϵ㣬��ΪOPCODEָ�����ǵ��������޷��Զ�ʶ���
		//����ж��Ƿ񴥷��ϵ��ɽű��������
		se->ip = vmexec_getReg(cpu,VM_REG_RSI) + 1;	//+1����
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
	vaddr lprmem;
	int szrmem;
	longptr mrval;
	int mode = vmexec_getLastMemAcc(cpu,TRUE,&lprmem,&szrmem,&mrval);
	if (mode != 0)
	{
		//�����ȴ���ֻ��ȡ����
		longptr valr = vmexec_getReg(cpu,VM_REG_RDI);
		//����Contextû����ջ���У���Ϊ���Ǽ�����0x1000��СҲ��Ϊ��
		if (lprmem >= valr)
		{	//������ʷ�����Context��
			longptr ctxoftr = lprmem - valr;
			//����ƫ�ƣ����ƫ��С��0xA0������Ϊ����Ч��Context����
			if (ctxoftr < 0xA0)
			{
				accessContextRead(cpu,(longptr)rip,ctxoftr,szrmem,mrval);
				if (grunState == bkrun_stepin)
				{
					grunState = bkrun_none;
					return vmexec_ret_finished;
				}
			}	
		}
		//�������Ǽ�ض�����ջ�ķ���
		valr = vmexec_getReg(cpu,VM_REG_RBP);
		if (lprmem >= valr)
		{	//������ʷ�����Context��
			longptr ctxoftr = lprmem - valr;
			//����ƫ�ƣ����ƫ��С��0xA0������Ϊ����Ч��Context����
			if (ctxoftr < 0x20)
			{
				accessVStackRead(cpu,(longptr)rip,lprmem,szrmem,mrval);
				if (grunState == bkrun_stepin)
				{
					grunState = bkrun_none;
					return vmexec_ret_finished;
				}
			}	
		}
	}

	vaddr lpwmem;
	int szwmem;//���ǻ�ȡ���һ��ָ����ڴ������Ϣ
	longptr mwval;
	mode = vmexec_getLastMemAcc(cpu,FALSE,&lpwmem,&szwmem,&mwval);
	if (mode != 0)
	{		//���ݷ��ʵ�ַ���˲���¼
		//__asm__("int3");	//���ǵĽű�֧�ֻ�࣬�������ڵ�����ͣ
		//�øó�ʽ��EDIָ��Context,ESIָ��OPCODE,EBPָ��vStack
		longptr valw = vmexec_getReg(cpu,VM_REG_RDI);
		//����Contextû����ջ���У���Ϊ���Ǽ�����0x1000��СҲ��Ϊ��
		if (lpwmem >= valw)
		{	//������ʷ�����Context��
			longptr ctxoftw = lpwmem - valw;
			//����ƫ�ƣ����ƫ��С��0x1000������Ϊ����Ч��Context����
			if (ctxoftw < 0xA0)
			{
				accessContextWrite(cpu,(longptr)rip,ctxoftw,szwmem,mwval);
				if (grunState == bkrun_stepin)
				{
					grunState = bkrun_none;
					return vmexec_ret_finished;
				}
			}
		}
		//�������Ǽ�ض�����ջ�ķ���
		valw = vmexec_getReg(cpu,VM_REG_RBP);
		if (lpwmem >= valw)
		{	//������ʷ�����Context��
			longptr ctxoftw = lpwmem - valw;
			//����ƫ�ƣ����ƫ��С��0xA0������Ϊ����Ч��Context����
			if (ctxoftw < 0x20)
			{
				accessVStackWrite(cpu,(longptr)rip,lpwmem,szwmem,mwval);
				if (grunState == bkrun_stepin)
				{
					grunState = bkrun_none;
					return vmexec_ret_finished;
				}
			}
		}
	}
	return vmexec_ret_normal;
}


int main(int argc,char** argv)
{	
	grunState = bkrun_none;
	//�������Ƿ���ȫ�ֽű���ǣ������ű�ֻ���ڵ�һ�α���
	return main_ret_global;
}
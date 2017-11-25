#include <windows.h>
#include "internal.h"
#include "codeblock.h"

cb_SeqExecute* gLastSeq = 0;

void accessContextRead(void* cpu,longptr rip,int oft,int size,longptr val)
{
	if (!gLastSeq) return;
	addrprintf(rip,"%d",gLastSeq->uuid);
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
	case 2:// push %02s->%08s
	{
			   gLastSeq->vars[0].type = cbvar_int;
			   gLastSeq->vars[0].sqword = oft;
			   gLastSeq->vars[1].type = cbvar_int;
			   gLastSeq->vars[1].sqword = val;
	}break;
	default:	addrprintf(rip,"%d",gLastSeq->uuid); break;
	}

}


void accessVStackRead(void* cpu,longptr rip,longptr stkaddr,int size,longptr val)
{
	if (!gLastSeq) return;
	addrprintf(rip,"%d",gLastSeq->uuid);
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
	default:	addrprintf(rip,"%d",gLastSeq->uuid); break;
	}
}



//����ָ��ִ�к�ص�
int		vmexec_after_step(void* cpu,void* rip,void* inst)
{
	cb_SeqExecute* se = vmexec_lastBKExec(cpu);
	if (se && se->rip == (longptr)rip)	//�ж��Ƿ��Ǵ�������
	{
		se->ip = vmexec_getReg(cpu,VM_REG_RSI) + 1;	//+1����
		gLastSeq = se;
	}
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
				accessContextRead(cpu,(longptr)rip,ctxoftr,szrmem,mrval);
		}
		//�������Ǽ�ض�����ջ�ķ���
		valr = vmexec_getReg(cpu,VM_REG_RBP);
		if (lprmem >= valr)
		{	//������ʷ�����Context��
			longptr ctxoftr = lprmem - valr;
			//����ƫ�ƣ����ƫ��С��0xA0������Ϊ����Ч��Context����
			if (ctxoftr < 0x20)
				accessVStackRead(cpu,(longptr)rip,lprmem,szrmem,mrval);
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
				accessContextWrite(cpu,(longptr)rip,ctxoftw,szwmem,mwval);
		}
		//�������Ǽ�ض�����ջ�ķ���
		valw = vmexec_getReg(cpu,VM_REG_RBP);
		if (lpwmem >= valw)
		{	//������ʷ�����Context��
			longptr ctxoftw = lpwmem - valw;
			//����ƫ�ƣ����ƫ��С��0xA0������Ϊ����Ч��Context����
			if (ctxoftw < 0x20)
				accessVStackWrite(cpu,(longptr)rip,lpwmem,szwmem,mwval);
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
	//�������Ƿ���ȫ�ֽű���ǣ������ű�ֻ���ڵ�һ�α���
	return main_ret_global;
}
#include <windows.h>
#include "internal.h"
#include "codeblock.h"

cb_SeqExecute* gLastSeq = 0;
int		grunState = bkrun_none;
int		gtmpStep = 0;
longptr glstBPAddr = 0;
longptr	glpContext = 0;
//ά�����100���ڴ���ʼ�¼
longptr	gwtedval[100];
int		gnwval = 0;
void detectOpBranch(void* cpu,cb_SeqExecute* se)
{
	//����ֻ��Ϊ����ʾ������������ַ��ʼ�¼��xor������36��֮�����ǽ�����һ���ж�
	if (gnwval >= 34 && gnwval <= 36)
	{	//���������5����¼��eflag������ΪΪxor,ע�����ǲ�׼ȷ�ģ�ֻ��Ϊ����ʾ����
		longptr efg = gwtedval[gnwval - 5];
		if ((efg & 0xFFFFFC00) == 0) //һ��eflagsǰ24λ��Ϊ0
		{
			se->vars[0].type = cbvar_setcomment;
			strcpy(se->vars[0].byte,"xor");
			gnwval = 0;
		}
	}
}

void accessContextWrite(void* cpu,longptr rip,int oft,int size,longptr val)
{
	//addrprintf(rip,"WC:oft=%08X ->size:%d,Value:%08X",oft,size,val);
	if (size == 4 && gnwval < 100)
	{	//����ֻ��¼100�����ڵ�4�ֽ�д��ָ��
		gwtedval[gnwval++] = val;
	}
}


void accessContextRead(void* cpu,longptr rip,int oft,int size,longptr val)
{

}

int vmexec_before_step(void* cpu,void* rip,void* inst)
{
	//ȷ�����ǵĵ���״̬�����µ�
	grunState = vmexec_getRunState(cpu);
	//�õ����һ����ִ�м�¼
	cb_SeqExecute* se = vmexec_lastBKExec(cpu);
	if (se != gLastSeq)
	{	//˵���Ѿ�ת������һ������飬���ǽ�����һ��������������
		detectOpBranch(cpu,gLastSeq);
	}
	gLastSeq = se;
	if (se && se->rip == (longptr)rip)	//�ж��Ƿ��Ǵ�������
	{
		gnwval = 0;	//ÿ�δ����������д���¼
		if (gtmpStep == 0)
		{	
			if (grunState == bkrun_stepover)
				gtmpStep = 1;
		} else
		{	
			if (gtmpStep == 1)
			{
				gtmpStep = 0;
				se->flags |= cbseq_flag_temp;
				return vmexec_ret_finished;
			}
		}
		if (glpContext == 0)
			glpContext = vmexec_getReg(cpu,VM_REG_RBP);
		longptr* optr = (longptr*)(glpContext + 0x23);	//+0x51Ϊopcode,ÿ�����ܿ��ܲ�һ��
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
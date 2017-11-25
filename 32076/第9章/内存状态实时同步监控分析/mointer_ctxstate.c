#include <windows.h>
#include "internal.h"
#include "xstate.h"
//�������������ʵʱ��ط���
//�ڴ�״̬ʵʱͬ����ط���

void*	gxstate = 0;//��Context��׷��״̬
void*	gvstkst = 0;//������ջ��׷��״̬
int		gmemberid = 1;

//������������һ����ԱIDջ������������ʱ���16����ԱID
#define max_nstackid	16
int		gmidstack[max_nstackid];
//ջ��ӵ�еĳ�Աʵ������
int		gnmid = 0;
//��ʾ��Ա�Ѿ������ʹ���
#define mid_state_dirty	0x10000

int		glastBlockID = -1;
//����Աѹ��ջ
void pushMemberToStack(int mid)
{
	//�������ｫ��Ա����ѹ���ԱIDջ
	if (gnmid >= max_nstackid)
	{
		gnmid = max_nstackid;
		//����������,ɾ�����ϵ�һ������ȻҲ�ɴ�ӡbug
		addrprintf(0,"MayBe Bug");
		for (int i = 1; i < gnmid; i++)
			gmidstack[i - 1] = gmidstack[i];
		gnmid--;
	}
	gmidstack[gnmid++] = mid;
}
//��4���������и��²��жϵ�ǰ�Ĵ����
int updateAndCheckLastBlock(void* cpu)
{
	int blockID = vmexec_lastBlockID(cpu);
	if (glastBlockID != blockID)
	{
		glastBlockID = blockID;
		return 0;
	}
	return 1;
}
void accessContextRead(void* cpu,longptr rip,int oft,int size)
{
	int mid;
	int voft = oft;
	//���²��ж��Ƿ������һ�������
	int isSameBlock = updateAndCheckLastBlock(cpu);
	//�������Ǵ�״̬����������ȥ��ǰContextƫ�Ʒ�Χ�Ƿ���״̬
	int szhit = xst_hit(gxstate,&voft,size,&mid);
	if (szhit == 0)
	{
		//�����Χ��û����ɫ��Ϣ��˵�����ǵ�׷������©
		addrprintf(rip,"MayBe Bug,Because Got rid of our trace:oft:%08X ->size:%d",oft,size);
	} else
	{
		mid &= ~mid_state_dirty;
		addrprintf(rip,"[%d]var%d <- oft:%08X ->size:%d",glastBlockID,mid,oft,size);
		pushMemberToStack(mid);
		//�������ǽ�Context�Ķ�Ӧ״̬�޸�Ϊ�Ѿ���ȡ��
		xst_set(gxstate,voft,size,mid | mid_state_dirty);
	}
}


void accessContextWrite(void* cpu,longptr rip,int oft,int size)
{
	int isSameBlock = updateAndCheckLastBlock(cpu);
	if (gnmid > 0)
	{
		//�����ԱIDջ�����г�Ա����ô���ǵ��ɳ�ԱǨ��������
		if (isSameBlock)
		{	//�������ͬһ��OP��֧���棬���ǲŴ���
			gnmid--;
			int mid = gmidstack[gnmid];
			mid &= ~mid_state_dirty;
			xst_set(gxstate,oft,size,mid);
			addrprintf(rip,"var%d -> oft:%08X->size:%d",mid,oft,size);
		}
	} else
	{
		//����ÿռ�û��״̬��˵����Ǩ������Ҫô��ͼ�ҵ���Ӧ�ı�����ϵ
		//Ҫô�½�һ��״̬ID
		int mid = gmemberid++;
		xst_set(gxstate,oft,size,mid);
		addrprintf(rip,"[%d]:new var%d -> oft=%08X ->size:%d",glastBlockID,mid,oft,size);
	}

}

void accessVStackRead(void* cpu,longptr rip,longptr stkaddr,int size)
{
	int mid;
	longptr voft = stkaddr;
	//���²��ж��Ƿ������һ�������
	int isSameBlock = updateAndCheckLastBlock(cpu);
	//�������Ǵ�״̬����������ȥ��ǰContextƫ�Ʒ�Χ�Ƿ���״̬
	int szhit = xst_hit(gvstkst,&voft,size,&mid);
	if (szhit != 0)
	{
		//������Ǽ�⵽�˳�Ա��Ϣ����������ջ����ô���ǽ������ԱID������ʱ����ջ
		//�������ｫ��Ա����ѹ���ԱIDջ
		pushMemberToStack(mid);
		//�������ǽ�״̬�������еĶ�Ӧ״̬�޸�Ϊ�Ѿ���ȡ��
		xst_set(gvstkst,voft,size,mid | mid_state_dirty);
		//��ӡ���ʼ�¼
		addrprintf(rip,"[%d]:var%d <- addr:%08X->size:%d",glastBlockID,mid & ~mid_state_dirty,stkaddr,size);
	}else
		addrprintf(rip,"[%d]:Vstack Read->addr:%08X->size:%d",glastBlockID,stkaddr,size);
}

void accessVStackWrite(void* cpu,longptr rip,longptr stkaddr,int size)
{
	int isSameBlock = updateAndCheckLastBlock(cpu);
	int loged = 0;
	if (gnmid > 0)
	{	//�����ԱIDջ�����г�Ա����ô���ǵ��ɳ�ԱǨ��������
		if (isSameBlock)
		{	//�������ͬһ��OP��֧���棬���ǲŴ���
			gnmid--;
			int mid = gmidstack[gnmid];
			mid &= ~mid_state_dirty;
			xst_set(gvstkst,stkaddr,size,mid);
			addrprintf(rip,"[%d]:var%d -> addr:%08X->size:%d",glastBlockID,mid,stkaddr,size);
			loged = 1;
		}
	}
	if (loged == 0)
		addrprintf(rip,"[%d]:VStack Write->addr:%08X->size:%d",glastBlockID,stkaddr,size);
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
				accessContextRead(cpu,(longptr)rip,ctxoftr,szrmem);
		}
		//�������Ǽ�ض�����ջ�ķ���
		valr = vmexec_getReg(cpu,VM_REG_RBP);
		if (lprmem >= valr)
		{	//������ʷ�����Context��
			longptr ctxoftr = lprmem - valr;
			//����ƫ�ƣ����ƫ��С��0xA0������Ϊ����Ч��Context����
			if (ctxoftr < 0x20)
				accessVStackRead(cpu,(longptr)rip,lprmem,szrmem);
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
				accessContextWrite(cpu,(longptr)rip,ctxoftw,szwmem);
		}
		//�������Ǽ�ض�����ջ�ķ���
		valw = vmexec_getReg(cpu,VM_REG_RBP);
		if (lpwmem >= valw)
		{	//������ʷ�����Context��
			longptr ctxoftw = lpwmem - valw;
			//����ƫ�ƣ����ƫ��С��0xA0������Ϊ����Ч��Context����
			if (ctxoftw < 0x20)
				accessVStackWrite(cpu,(longptr)rip,lpwmem,szwmem);
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
	//����һ��״̬ʵ��������Context
	gxstate = xst_allocstate();
	//����һ��״̬ʵ��������Stack
	gvstkst = xst_allocstate();
	//�������Ƿ���ȫ�ֽű���ǣ������ű�ֻ���ڵ�һ�α���
	return main_ret_global;
}
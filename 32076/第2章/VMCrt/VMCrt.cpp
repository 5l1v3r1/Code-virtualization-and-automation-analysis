// VMCrt.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "VMCrt.h"
#pragma comment(linker,"/ENTRY:simVMEntry")


struct x86Ctx	//�ֳ��ṹ������push 0x12345678,pushfd,pushadȷ��
{
	long edi;
	long esi;
	long ebp;
	long esp;
	long ebx;
	long edx;
	long ecx;
	long eax;
	long eflags;
	union{	//�����ʱ��push 0x12345678����OPָ�룬����ʱ�������ص�ַ
		long	opcode;
		void*	retaddr;
	};

};


struct vcpu_ctx
{
	long regs[8];	//�Ĵ���
	long eflags;	//���ڴ�ű�־�Ĵ���
	const char* lpopcode;	//���opcodeָ��
};


void* simVMLoop(vcpu_ctx* ctx)
{
	const char* lpop = ctx->lpopcode;	//����ָ�������ǵ����⻯ý��
	while (1)
	{
		switch (*lpop++)	//���벢ִ��
		{
		case op_mov:{//ģ��ִ��ָ�
			int regid = *lpop++;	//��ȡ����ý�鵱�еļĴ������
			ctx->regs[regid] = *(long*)lpop;	//ȡ�����������õ���Ӧ������Ĵ�������ȥ
			lpop += sizeof(long);
		}break;
		case op_add:{
			int regid = *lpop++;	//��ȡ����ý�鵱�еļĴ������
			//ȡ��������ģ��ִ��add����㷨
			ctx->regs[regid] += *(long*)lpop;
			lpop += sizeof(long);
		}break;
		case op_exit:
		{ //������������ֱ���趨���ڵ�ַ�����˳�����ִ��ѭ��
			return *(void**)lpop;
		}break;
		}
	}
	return 0;
}


void __stdcall simVMInit(x86Ctx* x86ctx)
{
	vcpu_ctx vctx;	//ֱ�ӳ�ʼ��һ�����⻯����
	vctx.regs[VREG_RAX] = x86ctx->eax;	//Ǩ�����ݵ����⻯����
	vctx.regs[VREG_RCX] = x86ctx->ecx;
	vctx.regs[VREG_RDX] = x86ctx->edx;
	vctx.regs[VREG_RBX] = x86ctx->ebx;
	vctx.regs[VREG_RSP] = x86ctx->esp;
	vctx.regs[VREG_RBP] = x86ctx->ebp;
	vctx.regs[VREG_RSI] = x86ctx->esi;
	vctx.regs[VREG_RDI] = x86ctx->edi;
	vctx.lpopcode = (const char*)x86ctx->opcode;
	vctx.eflags = x86ctx->eflags;
	void* lpout = simVMLoop(&vctx);	//�������ִ��
	x86ctx->eax = vctx.regs[VREG_RAX];	//Ǩ�����ݵ�ʵ�ʻ���
	x86ctx->ecx = vctx.regs[VREG_RCX];
	x86ctx->edx = vctx.regs[VREG_RDX];
	x86ctx->ebx = vctx.regs[VREG_RBX];
	x86ctx->esp = vctx.regs[VREG_RSP];
	x86ctx->ebp = vctx.regs[VREG_RBP];
	x86ctx->esi = vctx.regs[VREG_RSI];
	x86ctx->edi = vctx.regs[VREG_RDI];
	x86ctx->eflags = vctx.eflags;
	x86ctx->retaddr = lpout;	//���ó��ڵ�ַ
}


void __declspec(naked) simVMEntry()
{
	__asm{
		//�����������һ��վλָ�ʵ��ָ��ĳ����ڴ�����뵽Ŀ���ʽ��ʱ�ᱻ�滻��OPCODEָ��
		push 0x12345678		
		pushfd	//����eflags
		pushad	//����ͨ�üĴ������������ǵ�����ʱ������Ӱ�������Ĵ�����˲��ñ���
		push esp	//Ϊ�˷�������ֱ�ӽ�����ѹ�������ת����һ���ṹ��push esp���ǽṹָ��
		call simVMInit	//���ó�ʼ������
		popad		//���ڴ��룬�ָ�ͨ�üĴ���
		popfd		//�ָ���־�Ĵ���
		ret			//�������ǽ�OPCODEָ��ռ�õĿռ䣬�ڳ������������ص�ַ
	}
}



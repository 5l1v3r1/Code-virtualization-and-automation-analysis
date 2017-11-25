#ifndef internal_h__
#define internal_h__

#ifdef _M_IX86
typedef		unsigned long	vaddr;
typedef		unsigned long	longptr;
#else
typedef		unsigned long long vaddr;
typedef		unsigned long long longptr;
#endif

enum main_returns{	//�ű����з���״̬
	main_ret_error = -1,
	main_ret_finished = 0,
	main_ret_keep,		//�ű�Ҫ���������Ĵ��룬��������hook�������ű����ر�����
	main_ret_global		//�ű�Ҫ�����ýű�����ʵ����ֱ�������������
};

enum hook_type{
	hook_push = 0,
	hook_jmp,	//ʹ��jmp xxx��ʽ
	hook_bp,
	hook_bpcall,
	hook_callback,
	hook_callbackjmp
};

enum vmexec_return{
	vmexec_ret_normal = 0,	//�������أ�����ִ��
	vmexec_ret_finished,	//��������ʽ�˳������
	vmexec_ret_syscall,		//�ڲ�ʹ��
	vmexec_ret_signlestep,	//�Ե�ǰָ�����������쳣Ϊ���˳�
	vmexec_ret_exeception,	//�Ե�ǰָ�������쳣Ϊ���˳�
	vmexec_ret_fetchopfaild,//�ڲ�ʹ��
	vmexec_ret_fastcall		//Ϊ�˼��٣�ֱ��ִ��ָ��
};

enum vmrun_state{
	bkrun_none = -1,
	bkrun_run = 0,
	bkrun_stepover,
	bkrun_stepin
};

#define VM_REG_RAX 0
#define VM_REG_RCX 1
#define VM_REG_RDX 2
#define VM_REG_RBX 3
#define VM_REG_RSP 4
#define VM_REG_RBP 5
#define VM_REG_RSI 6
#define VM_REG_RDI 7

#define VM_REG_R8  8
#define VM_REG_R9  9
#define VM_REG_R10 10
#define VM_REG_R11 11
#define VM_REG_R12 12
#define VM_REG_R13 13
#define VM_REG_R14 14
#define VM_REG_R15 15

#pragma pack(push,1)
typedef struct _xhook_regs
{
#ifdef _M_X64
	longptr   r15;
	longptr   r14;
	longptr   r13;
	longptr   r12;
	longptr   r11;
	longptr   r10;
	longptr   r9;
	longptr   r8;
#endif
	longptr   rdi;
	longptr   rsi;
	longptr   rbp;
	longptr   rsp;
	longptr   rbx;
	longptr   rdx;
	longptr   rcx;
	longptr   rax;
	longptr   eflags;
}xhook_regs;


#pragma pack(pop)


#define memacc_flag_read		0x100	//���ڴ��ȡ����
#define memacc_flag_write   	0x200	//���ڴ�д�����
#define memacc_flag_stackread	0x400	//ESP�����ķ���
#define memacc_flag_stackwrite	0x800	//ESP�����ķ���
//typedef     void*   (CALLBACK* LPXHook_Inline_Callback)(xhook_stackregs* stregs,void* orgEntry,void* userdata);

//������ӿں���
char*		vmexec_getInstOpByte(void* inst,int* len);
int			vmexec_getLastMemAcc(void* cpu,int bread,vaddr* addr,int* size,vaddr* value);
longptr		vmexec_getReg(void* cpu,int reg);
long		vmexec_getEflags(void* cpu);
int			vmexec_lastBlockID(void* cpu);
int			vmexec_getBlockUUID(void* cpu,int seqID);
int			vmexec_getRunState(void* cpu);
int			vmexec_isBreak(void* cpu,void* rip);
//ȡ�����һ�������ִ�м�¼
struct cb_SeqExecute;
typedef struct cb_SeqExecute cb_SeqExecute;
cb_SeqExecute*		vmexec_lastBKExec(void* cpu);
void				vmexec_bkexec_setip(void* bklog,longptr ip);

//����ָ��ִ��ǰ�ص�
int		vmexec_before_step(void* cpu,void* rip,void* inst);
//����ָ��ִ�к�ص�
int		vmexec_after_step(void* cpu,void* rip,void* inst);
//����ѭ��ǰ�ص�
int		vmexec_before_loop(void* cpu,int reason);
//����ѭ����ص�
int		vmexec_after_loop(void* cpu,int reason);


extern char*		vcall(char* lpFunc,int nArg,...);
extern char*		vclasscall(char* lpclass,char* lpFunc,int nArg,...);
extern int			printf(const char * _Format,...);
extern int			addrprintf(long addr,const char * _Format,...);
//hook class functions
extern void*		hook_hookCode(void* lpAddr,void* lpProc,int hookType,void *userData);
extern int			hook_unhookCode(void* lpAddr);
extern void*		hook_hookCodeDirect(void* hNtdll,void* lpAddr,void* lpProc,int hookType,void* userdata);
extern void*		hook_hookApi(const wchar_t* dllName,const char* funcName,void* lpFunc,int hookType);
extern int 			hook_hookImport(void* hModule,const wchar_t* dllName,const char* funcName,void* lpFunc);

//pe class functions
extern int			pe_isValidPE(const char *lpimage);
extern long         pe_offsetToRva(const char* lpImage,long offset);
extern long         pe_rvaToOffset(const char* lpImage,long rva);

//debug class functions
extern void			dbg_raiseLoadDllException(void* hModule,const wchar_t* lpModPath);
void				dbg_raisePauseException();

//����ϵ�к�����չ��ֱ�Ӵ�ollydbg2 PDK�������������ǿ���ֱ���ڽű��а���ollydbg2��plugin.hͷ�ļ�
int					dbg_Setint3breakpoint(vaddr addr,int type,int fnindex,int limit,int count,wchar_t *condition,wchar_t *expression,wchar_t *exprtype);
int					dbg_Sethardbreakpoint(int index,int size,int type,int fnindex,vaddr addr,int limit,int count,wchar_t *condition,wchar_t *expression,wchar_t *exprtype);
int					dbg_Setmembreakpoint(vaddr addr,int size,int type,int limit,int count,wchar_t *condition,wchar_t *expression,wchar_t *exprtype);
int					dbg_Removeint3breakpoint(vaddr addr,int type);
int					dbg_Enableint3breakpoint(vaddr addr,int type);
int					dbg_Removemembreakpoint(vaddr addr);
int					dbg_Confirmint3breakpoint(vaddr addr);
int					dbg_Confirmhardwarebreakpoint(vaddr addr);
void				dbg_Wipebreakpointrange(vaddr addr0,vaddr addr1);
int					dbg_Enablemembreakpoint(vaddr addr,int enable);
int					dbg_Removehardbreakpoint(int index);
int					dbg_Enablehardbreakpoint(int index,int enable);
int					dbg_Findfreehardbreakslot(int type);


//
//
//#define _V_EFG_	0
//#define _V_EDI_	1
//#define _V_ESI_	2
//#define _V_EBP_	3
//#define _V_ESP_	4
//#define _V_EBX_	5
//#define _V_EDX_	6
//#define _V_ECX_	7
//#define _V_EAX_	8
//
//
//
//#define DEFILE_FUNC_ENTRY(name,func)	void* gfunc_core_##name = func;\
//										void* gfunc_retp_##name = 0;\
//										void func_entry_##name()\
//										{\
//											__asm__ __volatile__(	\
//											"leave\n\t"	\
//											"pusha\n\t"	\
//											"pushf\n\t"	\
//											"pushl %esp\n\t"	\
//											"call *gfunc_core_"#name"\n\t"	\
//											"popf\n\t"	\
//											"popa\n\t"	\
//											"jmp *gfunc_retp_"#name"\n\t"	\
//											);	\
//										}
//
//
//
//#define DEFILE_FUNC_RETP(name,ptr)	gfunc_retp_##name = ptr;
//
//
//// void __stdcall func_call_back(char** pvars)



#endif // internal_h__

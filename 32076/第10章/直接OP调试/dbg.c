#include <windows.h>
#include "internal.h"
#include "codeblock.h"
#include "plugin.h"	//ollydbg2��PDKͷ�ļ���û��ϸ���ַ���ɣ���������ļ�û�ṩ

int	gseqID = -1;
void* CALLBACK opcode_pass(xhook_regs* stregs,void* orgEntry,void* userdata)
{
	cb_SeqExecute se;
	struct opcode_* op;
	memset(&se,0,sizeof(se));
	
	se.seqid = gseqID;
	se.ip = stregs->rsi+1;	//ȡ��opcode��ַ
	//���ʹ����ִ�м�¼
	cb_send_execlog(&se);
	//�������Ǽ���Ƿ��д���鵥�����߶ϵ㴥��
	if (cb_getRunState() == bkrun_stepover || cb_isBreak(se.ip) != 0)
	{	//����жϵ㣬��ô����ֱ��ͨ���������Ķϵ㺯����HOOK��ʽ�ĳ������趨�ϵ�
		//��Ȼ����Ҳ����ͨ���Զ�������ͣ����������ʵ��ڻ��������κεط�
		dbg_Setint3breakpoint(orgEntry,BP_ONESHOT | BP_EXEC | BP_BREAK,0,0,0,0,0,0);
	}

	return orgEntry;
}
int main(int argc,char** argv)
{	
	//�Ƚ�����ģʽ�趨Ϊֱ�ӵ���ģʽ
	cb_set_debugmode(cb_dbg_direct);
	//����ֱ��ͨ���ű��Ĵ���HOOK������Ŀ���ʽ�Ĵ����֧��ת��HOOK
	hook_hookCode((void*)0x119B559,opcode_pass,hook_callback,0);
	//�����½�һ�������������ʾ��ʵ������ʵ�ʹ��������ǿ���ͨ���ֶ�����ȷ�Ĵ����
	gseqID = cb_make_codeblock(1,0x119B559,-1,0,L"dummy",0);
	//�������Ƿ���ȫ�ֽű���ǣ������ű���һֱפ���ڴ�����ᱻ�ͷ�
	return main_ret_global;
}
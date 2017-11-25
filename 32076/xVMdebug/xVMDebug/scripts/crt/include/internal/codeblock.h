#ifndef codeblock_h__
#define codeblock_h__


#ifdef _M_IX86
#define REG_MAX_NUM 8
typedef	unsigned long cb_addr;
#else
#define REG_MAX_NUM 16
typedef	unsigned long long cb_addr;
#endif

enum cb_seq_var_type{
	cbvar_none = 0,
	cbvar_int,
	cbvar_str,
	cbvar_setuuid,	//ͨ�������ش�����鹦��ID
	cbvar_setpattern,	//�ش�pattern
	cbvar_setcomment	//�ش�comment
};

enum cb_debugmode{
	cb_dbg_byvm = 0,
	cb_dbg_direct
};
#define max_var_num		4

#define cbseq_flag_temp	0x4
struct cb_SeqExecute{
	int			seqid;	//�������
	int			uuid;
	long		flags;	//���
	cb_addr		ip;		//OPCODE��ַ
	long		eflags;	//eflag
	cb_addr		rip;
	cb_addr		regs[REG_MAX_NUM];	//��Ӧ�����ļĴ���
	struct
	{
		int		type;	//��������
		union
		{
			signed long long sqword;
			char	byte[16];	//�16��ֱ��
		};
	}vars[max_var_num];	//���4������
};
//ֱ�ӽ��������
int		cb_make_codeblock(int seqID,vaddr entry,int uuid,long flags,const wchar_t* pattern,const wchar_t* comment);
//���Ϳ�ִ�м�¼
void	cb_send_execlog(cb_SeqExecute* se);
//�趨ƫ�õĵ��Է�ʽ(���ⷽʽ/ֱ�ӷ�ʽ/��Ϸ�ʽ)
void	cb_set_debugmode(int mode);
//�Ƿ��Ѿ������ϵ�
int		cb_isBreak(vaddr oip);
//��ȡ����״̬(����/����/����)
int		cb_getRunState();

#endif // codeblock_h__

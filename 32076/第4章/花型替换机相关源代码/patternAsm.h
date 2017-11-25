#ifndef patternAsm_h__
#define patternAsm_h__

#include "../../../nCom/nlinkedlist.h"
#include "../../../nCom/nautolargebuffer.h"
#include "../../../ncvm/ncasm/ncasm86.h"
#include "../../../nCom/avltree.h"
#include "../../../3rdparty/distorm/mnemonics.h"
#include "../../../nCom/nexpress.h"

enum patternMode{
	mode_sequence,	//˳��ģʽ
	mode_reverse	//����ģʽ
};

enum pattern_inst_type{
	pattern_inst_none,	//�����
	pattern_inst_instruction,//ָ��ƥ�����
	pattern_inst_control,	//�������
	pattern_inst_express,	//���ʽ���
	pattern_inst_memexpr	//�ڴ���ʽ���
};

#define op_varid_base	(1 << 24)	//��������ڴ�����id
enum pattern_op_type{	//��������
	pattern_op_none,
	pattern_op_any,	//�κβ���������
	pattern_op_reg,	//�Ĵ�������������
	pattern_op_imm,	//��������
	pattern_op_mem,	//�ڴ����������
	pattern_op_size,	//��С����
	pattern_op_mnemonic,	//ָ�����
	pattern_op_memexpr,	//�ڴ���ʽ����
	pattern_op_explicit	//ֱ��������
};

//�ű������ṹ���ýṹ�洢���б�������ֵ
struct pasm_operand{
	int					type;	//
	int					size;
	int					opcode;
	int					base;
	int					index;
	int					dispvar;
	int					scale;
	int					seg;
	int64_t				imm;

	explicit struct pasm_operand()
	{
		type = O_NONE;
		size = 0;
		base = R_NONE;
		index = R_NONE;
		scale = 0;
		imm = 0;
		seg = 0;
		dispvar = 0;
		opcode = I_UNDEFINED;
	}
};

#define  PATTERN_CFLAG_NONE				0
#define  PATTERN_CFLAG_NOREF			0x1	//����������
#define  PATTERN_CFLAG_NOT				0x2	//��ƥ��
#define  PATTERN_CFLAG_INALLOP			0x4		//�����в���������ƥ��
#define  PATTERN_CFLAG_REF				0x8 	//����ƥ��
#define  PATTERN_CFLAG_STRICT			0x10	//�ϸ������ƥ��
#define  PATTERN_CFLAG_8BIT				0x20	//��������С����
#define  PATTERN_CFLAG_16BIT			0x40
#define  PATTERN_CFLAG_32BIT			0x80
#define  PATTERN_CFLAG_64BIT			0x100
//��һָ�����������
struct pattern_op{
	pattern_op_type type;	//����������
	int				id;	//����������id
	int				szid;	//��������С����id
	long			cflags;	//����������
};
//ָ����ṹ
struct pattern_inst_body{
	ncasm::x86::inst			ins;	//ָ����ṹ
	int							szid;	//size������id	-1��ʾû��
	int							mnemonicid;	//ָ�������id
	long						cflags;	//ָ����
	pattern_op					ops[OPERANDS_NO];	//������
	struct pattern_inst_body()
	{
		szid = 0;
		mnemonicid = 0;
		cflags = PATTERN_CFLAG_NONE;
		for (int i = 0; i < OPERANDS_NO; i++)
		{
			ops[i].type = pattern_op_none;
			ops[i].szid = 0;
			ops[i].cflags = 0;
			ops[i].id = 0;
		}
			
	}
};

struct pasm_opcode{
	ncasm::x86::mnemonic	mnem;
};

#define		PATTERN_FLAG_NOESPCHANGE	0x1	//������ESP�б仯
#define		PATTERN_FLAG_NOEFLAGSCHG	0x2	//������Eflags�б仯
#define		PATTERN_FLAG_NOT			0x4
#define		PATTERN_FLAG_LESSMATCH		0x8
struct patternInst
{
	pattern_inst_type			type;	//������ͣ�ƥ����䣬������䣬���ʽ���ȣ�
	int							flags;	//���

	NLinkedList<pattern_inst_body>		insts;	//�����ƥ��ָ��ϸ��
	NLinkedList<pattern_inst_body>		rps;	//�滻ָ��ľ���ϵ��

	//pattern_inst_instruction ����
	int			minc;	//��Сƥ�����
	int			maxc;	//���ƥ�����
	void*		userdata;
	int			matched_count;	//��ǰָ���Ѿ��ɹ�ƥ�����	//for runtime

	//pattern_inst_express	//���ʽ���ʹ��
	int			varid;
	nexpr_expr*	expr;

	//memory express	//�ڴ���ʽ���ʹ��
	pasm_operand* memexpr;

	//mnemonic control	//ָ��ƥ�����ʹ��
	NLinkedList<pasm_opcode>	mnemops;

	struct patternInst()
	{
		type = pattern_inst_none;
		flags = 0;
		userdata = 0;
		minc = 1;
		maxc = 1;
		expr = 0;
		memexpr = 0;
	}
	~patternInst()
	{
		if (expr)
		{
			delete expr;
			expr = 0;
		}
		if (memexpr)
		{
			delete memexpr;
			memexpr = 0;
		}
	}
};
#define  PATTERN_SEQ_FLAG_NONE	0
#define  PATTERN_SEQ_FLAG_OFF	0x1

#define		MAX_VAR_NEWONCE	0x10
struct seqRuntime{
	patternInst*				inst;		//��ǰָ��λ��
	int							count;		//ƥ�����
	NAvlList<int,pasm_operand>	vars;		//����
	NAvlList<int,pasm_operand>	opvars;	//��̬����ƥ����ʹ��
	int		navar;						//����ƥ����������ı�������
	int		nvars[MAX_VAR_NEWONCE];		//����ľ������id,��ƥ��ʧ�ܵ�ʱ��ɾ��
	struct seqRuntime()
	{
		inst = 0;
		count = 0;
		navar = 0;
	}

};

struct patternSequence
{
	//����ʱ���ݣ���Щ�����ڱ��뻨�͹����в����������ǱȶԵ�����
	int								id;				//����ID
	patternMode						mode;			//�Ա�ģʽ
	long							flags;			//�û��͵�ȫ�ֱ��
	int								maximumInst;	//���Ա�ָ������
	NLinkedList<patternInst>		insts;			//�����������ľ������
	//����ʱ���ݣ�������ڶԱȵ�ʱ�����ɵ�ƥ�����ݺͱ���
	seqRuntime						crt;
	//��������ά��һ��˫��������ѭ������ҪĿ��������Ч��
	struct patternSequence*			prev;
	struct patternSequence*			next;

	std::string						comment;
	std::string						pattern;
	int								uuid;
	NAvlList<int,pasm_operand>		dyops;	//��̬��������ʱ����

	struct patternSequence()
	{
		mode = mode_sequence;
		next = 0;
		maximumInst = 30;
		flags = PATTERN_SEQ_FLAG_NONE;
		uuid = -1;
	}
	patternInst* newinst(pattern_inst_type type)
	{
		patternInst* inst = insts.Allocate();
		insts.AddAtLast(inst);
		inst->type = type;
		return inst;
	}
};

typedef index_map<std::string,short> mnemonicmap;
class patternAsm{
public:
	patternAsm();
	virtual ~patternAsm();
	//�������뼰������غ���
	void loadMnemonic();
	void clearPatterns();
	int loadPatternFromFile(const wchar_t* filename);
	int loadPattern(const char* lpsrc, int szsrc);
	int parsePattern(const char* lpsrc, int szsrc);
	int compile(patternSequence* seq,const char* lpsrc, int szsrc);
	int compile(patternSequence* seq,NStringListA* nsl);
	//���ʽ������غ���
	int calculateExpress(patternSequence* seq,nexpr_expr* expr,int64_t& val);
	int calculateMemoryExpress(patternSequence* seq,pasm_operand* expr,pasm_operand* op);

	//����ƥ�������غ���
	//��ʼ��ƥ�䣬��ʼ�������л�ѡȡ���û����Լ��趨ƥ��ģʽ
	int initForTest(bool breverse);
	//���Ծ����ĳһ��ָ��������ִ��һ�������Թ����У�
	// ������ֲ�ƥ��Ļ��ͣ����ᱻѡȡ�Ļ����������޳�
	// ���ǲ��ϲ���ʣ�µ�ƥ�仨�ͣ�ֱ��ʣ��1������û�л���ƥ�����Ǿ���Ϊƥ������ˡ�
	// �������ǵ�patternAsm�ಢ�����ⷽ��Ĺ�������Ҫʹ�øýű��Ĵ��������й���
	int testInstruction(const char* lpop,int szop,ULONG_PTR pc,bool isEspChanged,bool isEflagsChanged,void* userdata,patternSequence*& sr);
	int testForSequence(patternSequence* seq,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged,void* userdata);
	int testForInstruction(patternSequence* rt,patternInst* pi,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged,void* userdata);

	int isMatchInst(patternSequence* seq,patternInst* pi,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged);
	int isMatchInstBody(patternSequence* seq,patternInst* pi,pattern_inst_body* ib,ncasm::x86::inst* ins);
	int isMatchInstOperand(patternSequence* seq,patternInst* pi,pattern_inst_body* ib,pattern_op* pop,int ivop,ncasm::x86::operand* iop,int ix86op,ncasm::x86::inst* ins);
	//����������غ���
	int varid(pattern_op_type type,int id);
	int checkAndSaveVar(patternSequence* seq,pattern_op* pop,ncasm::x86::inst* ins,ncasm::x86::operand* iop);
	int saveMemoryExprVar(patternSequence* seq,pattern_op* pop,ncasm::x86::inst* ins,ncasm::x86::operand* iop);
	int resolveVar(patternSequence* seq,pasm_operand* op,pattern_op* pop);
	//�滻ָ�����ɺ���
	int genReplaceOpcode(patternSequence* seq,pattern_inst_body* pi,char* lpop,int szop,ULONG_PTR pc);

	patternSequence* getPattern(int pid);
private:
	//�ʷ�������غ���
  int parseLine(patternSequence* seq, const char* lpline, int len);
  int parseInst(patternSequence* seq,pattern_inst_body* inst,NStringA& line);
  int parseInstOption(patternInst* inst,NStringA& line);
  int parseExpress(patternInst* inst,NStringA& line);
  int parseMemoryExpress(patternInst* inst,NStringA& line);
  int parseMemoryOperand(pasm_operand* op,NStringA& line);
  int parseControlExpress(patternInst* inst,NStringA& line);
private:
	NAvlList<int, patternSequence>	m_Patterns;	//���б���õĻ���
	//��������ά��һ���򵥵��б�����ʾ��ǰ���õĻ���
	patternSequence*				m_seqList;
	mnemonicmap		m_mnemonics;
	ncasm::x86		m_x86asm;

};


#endif // patternAsm_h__

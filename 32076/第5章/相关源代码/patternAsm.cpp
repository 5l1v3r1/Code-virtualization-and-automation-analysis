
#include "patternAsm.h"
#include "../../../nCom/nstringhash.h"

void ToUpper(std::string &str)
{
	int slen = str.size();
	char* tcp = (char*)str.data();
	for (int i = 0; i < slen; i++)
		tcp[i] = toupper(tcp[i]);
}

patternAsm::patternAsm()
:m_x86asm(32)
,m_seqList(0)
{
	
  loadMnemonic();
}


patternAsm::~patternAsm()
{
}

int patternAsm::compile(patternSequence* seq, const char* lpsrc, int szsrc)
{
	NStringListA nsl;
	int nln = 0;
	int pos = 0;
	for (int i = 0; i < szsrc; i++)
	{
		if (lpsrc[i] == '\n')//new line found
		{
			nsl.push(lpsrc + pos, i - pos, NSTRINGLIST_PUSH_TRIM | NSTRINGLIST_PUSH_RESHAP);
			nln++;
			pos = i + 1;
		}
	}
	nsl.push(lpsrc + pos, szsrc - pos, NSTRINGLIST_PUSH_TRIM | NSTRINGLIST_PUSH_RESHAP);
	return compile(seq,&nsl);
}

int patternAsm::compile(patternSequence* seq, NStringListA* nsl)
{
	if (nsl->count() < 1)	//NStringListA��һ���ִ��б�������һ�����͵����нű����
		return 0;
	int ipos = 0;
	const char* lpline = nsl->first(ipos); //�Խű�����Ԥ����
	while (lpline)
	{
		int ln = nsl->valueLength(ipos);	
		//��������Դ���5��-���ſ�ͷ��ô������������
		//�������ĳ����������ڷָ�ű��ļ�ʱ���µ�
		if (ln > 0 && !((ln >= 5) && (strncmp(lpline,"-----",5) == 0)))
		{
			for (int i = 0; i < ln - 1;i++)
			{	//Ԥ������ע��
				if (lpline[i] == '/' && lpline[i + 1] == '/')
				{
					ln = i;
					break;
				}
			}
			//����ʵʩ�����������
			if (ln > 0) parseLine(seq,lpline,ln);
		}
		lpline = nsl->next(ipos);
	}
	return 1;
}
//���������ű����
int patternAsm::parseLine(patternSequence* seq, const char* lpline, int len)
{
	if (len < 1) return 0;
	NStringA	line,replace,word;
	int palen = len;
	int rplen = 0;
	const char* lprep = 0;
	for (int i = 0; i < len - 1; i++)
	{
		//��������̽��->�ָ���ţ������������ű�ʾ�������ʵ���ϰ�����ƥ����滻�������������Ҫ�ֿ�����
		if (lpline[i] == '-' && lpline[i + 1] == '>')	
		{	//������滻ָ���¼�������Ϣ���ں������ڵ�������
			palen = i;
			lprep = lpline + i + 2;
			rplen = len - i - 2;
			break;
		}
	}
	//���Ǹ���һ�����Ŀ���
	line.copy(lpline,palen);
	if (lprep && rplen > 0)
	{
		replace.copy(lprep,rplen);
		replace.trim("\r\n \t");
	}
	//���������Ȳ�������Ƿ�߱����ʽ�����������???=???��ʽ
	int wordlen = 0;
	const char* lpword = line.firstword(wordlen,"=","\t =");
	if (wordlen > 3 && strnicmp(lpword,"imm",3) == 0)
	{	//we parse imm?=
		//����߱��������ʽ������������ֱ�ӳ�ʼ�����ʽָ��ṹ��ת����ʽ������
		patternInst* inst = seq->newinst(pattern_inst_express);
		//������ʽ����ʧ���ˣ�ɾ����ʼ����ָ��
		int stp = parseExpress(inst,line);
		if (stp < 0)
			seq->insts.erase(inst);
		return stp;
	} else if (wordlen >= 3 && strnicmp(lpword,"mem",3) == 0)
	{	//parse mem?=
		//������ڴ������ת���ڴ����������
		patternInst* inst = seq->newinst(pattern_inst_memexpr);
		int stp = parseMemoryExpress(inst,line);
		if (stp < 0)
			seq->insts.erase(inst);
		return stp;
	} else if (wordlen >= 4 && strnicmp(lpword,"mnem",4) == 0)
	{	//parse mnem?=
		//�����ָ���������ת�����������
		patternInst* inst = seq->newinst(pattern_inst_control);
		int stp = parseControlExpress(inst,line);
		if (stp < 0)
			seq->insts.erase(inst);
		return stp;
	}
	//�����￪ʼ�����ǵ���һ���ƥ�������н���
	//����ȡ����һ���Էָ����ָ�ĵ���
	lpword = line.firstword(wordlen,"\t ","\t ");
	if (!lpword || wordlen < 1) return 0;
	word.copy(lpword, wordlen);
	long shash = nhash_lower_len<char>(word.data(), word.length());
	patternInst* inst = 0;
	switch (shash)	//���Ե����Ƿ����ڹؼ�������
	{
		case HASH_STRING(5, ('.', 'm', 'o', 'd', 'e')):  //.mode
		{	//����ǹؼ���.mode����ô������ز���
			line.mid(line.pos(lpword + wordlen));
			const char* word2 = line.firstword(wordlen,"\t ","\t ");
			if (word2 && stricmp(word2, "reserve") == 0)
				seq->mode = mode_reverse;
			else
				seq->mode = mode_sequence;

		}break;
		case HASH_STRING(4, ('.', 'm', 'a', 'x')):
		{
			line.mid(line.pos(lpword + wordlen));
			const char* word2 = line.firstword(wordlen,"\t ","\t ");
			int64_t tv = 0;
			if (word2 && ncasm::x86::parseInteger(tv,word2,wordlen))
				seq->maximumInst = tv;
		}break;
		case HASH_STRING(5,('.','u','u','i','d')):
		{
			line.mid(line.pos(lpword + wordlen));
			const char* word2 = line.firstword(wordlen,"\t ","\t ");
			int64_t tv = 0;
			if (word2 && ncasm::x86::parseInteger(tv,word2,wordlen))
				seq->uuid = tv;
		}break;
		case HASH_STRING(4,('.','o','f','f')):
		{
			seq->flags |= PATTERN_SEQ_FLAG_OFF;
		}break;
		case HASH_STRING(3,('.','o','n')):
		{
			seq->flags &= ~PATTERN_SEQ_FLAG_OFF;
		}break;
		case HASH_STRING(8,('.','c','o','m','m','e','n','t')):
		{
			line.mid(line.pos(lpword + wordlen));
			line.trim("\r\n \t");
			seq->comment = line.data();
		}break;
		case HASH_STRING(8,('.','p','a','t','t','e','r','n')):
		{
			line.mid(line.pos(lpword + wordlen));
			line.trim("\r\n \t");
			seq->pattern = line.data();
		}break;
		default:
		{
			//������ǹؼ�����䣬��ô���ǵ���һ���ƥ��ָ��������
			inst = seq->newinst(pattern_inst_instruction);
			lpword = line.split(wordlen,"::","\t ",true);
			if (lpword && wordlen > 0)
			{	//���Ȳ��Ҳ�ƥ���������趨���
				wordlen--;
				NStringA lopt;
				lopt.copy(lpword,wordlen);
				parseInstOption(inst,lopt);	//��������������﷨
				line.mid(line.pos(lpword + wordlen)+2);
				line.trim("\r\n \t");
			}
			//�������Ǵ�����||�ָ�ؼ��ַָ�Ķ���ƥ��ָ��
			lpword = line.split(wordlen,"||","|\t ",false);
			while (lpword && wordlen > 0)
			{
				pattern_inst_body* ip = inst->insts.new_back();
				NStringA na;
				na.copy(lpword,wordlen);
				if (parseInst(seq,ip,na) < 0)	//��������ƥ��ָ��
				{
					seq->insts.erase(inst);
					inst = 0;
					break;
				}
				line.mid(line.pos(lpword + wordlen));
				line.trim("\r\n \t");
				lpword = line.split(wordlen,"||","|\t ",false);
			}	
			//������ǽ����滻ָ��
			if (inst && replace.length() > 0)
			{
				inst->rps.clear();
				wordlen = 0;
				lpword = replace.split(wordlen,"||","|\t ",false);
				while (lpword && wordlen > 0)
				{	//�滻ָ�����Ǵ��滻ָ��洢�ṹ�������ݿռ�
					pattern_inst_body* ip = inst->rps.new_back();
					NStringA na;
					na.copy(lpword,wordlen);
					if (parseInst(seq,ip,na) < 0)	//�滻ָ��ʵ������ƥ��ָ��Ľ�����ʽ��ͬ
						inst->rps.erase(ip);
					replace.mid(replace.pos(lpword + wordlen));
					replace.trim("\r\n \t");
					lpword = replace.split(wordlen,"||","|\t ",false);
				}
			}
		}break;
	}
	return 0;
}



int patternAsm::parseInstOption(patternInst* inst,NStringA& line)
{
	int wordlen = 0;
	const char* lpword = line.split(wordlen,"}","{\t }",false);
	while (lpword && wordlen > 0)
	{
		long shash = nhash_lower_len<char>(lpword,wordlen);
		switch (shash)
		{
		case HASH_STRING(1,('*')):  //*
		{
			inst->minc = 0;
			inst->maxc = 0xFFFFFF;
		}break;
		case HASH_STRING(1,('?')):  //*
		{
			inst->minc = 1;
			inst->maxc = 1;
		}break;
		case HASH_STRING(1,('+')):  //*
		{
			inst->minc = 1;
			inst->maxc = 0xFFFFFF;
		}break;
		case HASH_STRING(1,('-')):  //*
		{
			inst->flags = PATTERN_FLAG_LESSMATCH;
		}break;
		case HASH_STRING(5,('s','t','a','c','k')):
		{
			inst->flags |= PATTERN_FLAG_NOESPCHANGE;
		}break;
		case HASH_STRING(6,('e','f','l','a','g','s')):
		{
			inst->flags |= PATTERN_FLAG_NOEFLAGSCHG;
		}break;
		default:{
					NStringA na;
					na.copy(lpword,wordlen);
					int word2len = 0;
					const char* word2 = na.split(word2len,",",",{\t }",false);
					while (word2 && word2len > 0)
					{
						shash = nhash_lower_len<char>(word2,word2len);
						switch (shash)
						{
						case HASH_STRING(6,('r','e','p','e','a','t')):  //*
						{
							na.mid(na.pos(word2 + word2len));
							na.trim("\r\n \t");
							int64_t v1 = 0;
							int64_t v2 = 0;
							word2 = na.split(word2len,",",",{\t }",false);
							if (word2 && word2len > 0)
							{
								ncasm::x86::parseInteger(v1,word2,word2len);
								word2 = na.split(word2len,",",",{\t }",false);
								na.mid(na.pos(word2 + word2len));
								na.trim("\r\n \t");
								if (word2 && word2len > 0)
								{
									ncasm::x86::parseInteger(v2,word2,word2len);
									inst->minc = v1;
									inst->maxc = v2;
								} else
								{
									inst->minc = v1;
									inst->maxc = v1;
								}
							}
						}break;
						}
						na.mid(na.pos(word2 + word2len));
						na.trim("\r\n \t");
						word2 = na.split(word2len,",",",{\t }",false);
					}

		}break;
		}


		line.mid(line.pos(lpword + wordlen));
		line.trim("\r\n \t");
		lpword = line.split(wordlen,"}","{\t }",false);
	}
	return 0;
}

int patternAsm::parseExpress(patternInst* inst,NStringA& line)
{
	int wordlen = 0;
	const char* lpword = line.firstword(wordlen,"=","\t =");
	int vid = 0;
	if (wordlen >= 3 && strnicmp(lpword,"imm",3) == 0)
	{	//we parse imm?=
		vid = atoi(lpword+3);
		if (vid == 0) return -1;
		line.mid(line.pos(lpword + wordlen));
		line.trim("\r\n \t");
		line.pop_front(1);
		line.trim("\r\n \t");
	}
	if (vid == 0) return -1;
	inst->varid = vid;
	int st = nparseExpress(&inst->expr,line.data(),line.length());
	if (!inst->expr)
		return -2;
	return 0;
}


int patternAsm::loadPatternFromFile(const wchar_t* filename)
{
	NAutoLargeBufferA nla;
	nla.load(filename);
	if(nla.size() == 0) return 0;
	int szsrc = 0;
	const char* lpsrc = nla.data(szsrc,0,false);
	return loadPattern(lpsrc,szsrc);
}

void patternAsm::clearPatterns() 
{
	m_Patterns.cleanup();
	m_seqList = 0;
}

int patternAsm::loadPattern(const char* lpsrc, int szsrc) 
{
	int npa = 0;
	int pos = 0;
	int lns = 0;
	for (int i = 0; i < szsrc; i++)
	{
		if (lpsrc[i] == '\n')//new line found
		{
			if ((i - lns) >= 5 && strncmp(lpsrc + lns, "-----", 5) == 0)	//new pattern define found
			{
				parsePattern(lpsrc + pos, i-pos);
				npa++;
				pos = i;
			}
			lns = i+1;
		}
	}
	parsePattern(lpsrc + pos, szsrc-pos);
	npa++;
	return npa;
}

int patternAsm::parsePattern(const char* lpsrc, int szsrc)
{
	int pid = m_Patterns.count();
	patternSequence*	pattern = m_Patterns.new_back(pid);
	pattern->id = pid;
	int st = compile(pattern,lpsrc, szsrc);
	if (pattern->insts.GetCount() < 1)
	{
		m_Patterns.remove(pattern);
	}
	return st;
}


int patternAsm::parseMemoryExpress(patternInst* inst,NStringA& line)
{
	int wordlen = 0;
	const char* lpword = line.firstword(wordlen,"=","\t =");
	int vid = 0;
	if (wordlen >= 3 && strnicmp(lpword,"mem",3) == 0)
	{	//we parse imm?=
		vid = atoi(lpword + 3);
		if (vid == 0) return -1;
		line.mid(line.pos(lpword + wordlen));
		line.trim("\r\n \t");
		line.pop_front(1);
		line.trim("\r\n \t");
	}
	if (vid == 0) return -1;
	inst->varid = vid;
	line.trim("\r\n \t");
	if (!inst->memexpr)
		inst->memexpr = new pasm_operand();
	return parseMemoryOperand(inst->memexpr,line);
}

int patternAsm::parseMemoryOperand(pasm_operand* op,NStringA& line)
{
	int wordlen = 0;
	const char* lpword = line.firstword(wordlen,"}","\t }",true);
	while (lpword && wordlen > 0 && *lpword == '{')
	{
		lpword++;
		wordlen--;
		if (wordlen >= 4 && strnicmp(lpword,"size",4) == 0)
		{
			int vid = atoi(lpword + 4);
			if (vid != 0)
			{
				op->size = varid(pattern_op_size,vid);
			}
		}
		line.mid(line.pos(lpword + wordlen));
		line.pop_front(1);
		line.trim("\r\n \t");
		lpword = line.firstword(wordlen,"}","\t }",true);
	}

	int rgstk[4];
	for (int i = 0; i<4; i++)
		rgstk[i] = R_NONE;

	int ridx = 0;

	lpword = line.firstword(wordlen,"\t :[","\t :");
	while (lpword && wordlen > 0 && op->type == O_NONE)
	{
		NStringA::trimRight(lpword,wordlen,"[");
		long shash = nhash_lower_len<char>(lpword,wordlen);
		switch (shash)
		{
		case HASH_STRING(5,('q','w','o','r','d')):op->size = 64; break;//qword
		case HASH_STRING(5,('d','w','o','r','d')):op->size = 32; break;//dword
		case HASH_STRING(4,('w','o','r','d')):op->size = 16; break;//word
		case HASH_STRING(4,('b','y','t','e')):op->size = 8; break;//byte
		case HASH_STRING(3,('p','t','r')):break;//ptr
		case HASH_STRING(2,('c','s')):op->seg = R_CS; break;//segment
		case HASH_STRING(2,('d','s')):op->seg = R_DS; break;//segment
		case HASH_STRING(2,('e','s')):op->seg = R_ES; break;//segment
		case HASH_STRING(2,('f','s')):op->seg = R_FS; break;//segment
		case HASH_STRING(2,('g','s')):op->seg = R_GS; break;//segment
		case HASH_STRING(2,('s','s')):op->seg = R_SS; break;//segment
		default:
		{
				   if (lpword[0] == '[')	//parse as memory operand
				   {
					   op->type = O_DISP;
					   lpword = line.firstword(wordlen,"+]*","+]*[: \t");
					   while (lpword && wordlen > 0)
					   {
						   int treg = ncasm::x86::get_regbyname(lpword,wordlen);
						   if (treg == R_NONE && wordlen > 3 && strnicmp(lpword,"reg",3) == 0)
						   {	//try reg variant
							   int vid = atoi(lpword + 3);
							   if (vid != 0)
								   treg = varid(pattern_op_reg,vid);
						   } else if (treg == R_NONE && wordlen > 3 && strnicmp(lpword,"imm",3) == 0)
						   {	//try reg variant
							   int vid = atoi(lpword + 3);
							   if (vid != 0)
								   op->dispvar = varid(pattern_op_imm,vid);
						   }

						   if (treg != R_NONE)
						   {
							   if (op->type == O_SMEM) op->type = O_MEM;
							   if (op->type != O_MEM) op->type = O_SMEM;
							   rgstk[ridx++] = treg;

							   int ltm = wordlen;
							   NStringA::trimRight(lpword,ltm,"+][: \t");
							   if (ltm > 0 && lpword[ltm] == '*')
							   {
								   if (ridx < 1) return -9;
								   op->type = O_MEM;
								   op->index = rgstk[--ridx];
								   int sublen = line.length() - wordlen;
								   const char* lpreg = NStringA::getword(lpword + wordlen,sublen,sublen,"","* \t");
								   if (lpreg && sublen > 0)
								   {
									   int64_t vsc = 0;
									   if ((sublen = ncasm::x86::parseInteger(vsc,lpreg,sublen)) > 0)
									   {
										   op->scale = vsc;
										   line.mid(line.pos(lpreg + sublen));
										   line.trim("\r\n \t");
										   lpword = line.firstword(wordlen,"+]*","+]*[: \t");
										   continue;
									   } else
										   op->scale = 1;
								   }
							   }
						   } else
						   {
							   int64_t vsc = 0;
							   if (ncasm::x86::parseInteger(vsc,lpword,wordlen) > 0)
								   op->imm = vsc;
						   }


						   line.mid(line.pos(lpword + wordlen));
						   line.trim("\r\n \t");
						   lpword = line.firstword(wordlen,"+]*","+]*[: \t");
					   }
					   if (ridx > 0)
					   {
						   op->base = rgstk[--ridx];
					   }
				   } else return -1;
		}break;
		}
		line.mid(line.pos(lpword + wordlen));
		line.trim("\r\n \t");
		lpword = line.firstword(wordlen,"\t :[","\t :");
	}
	return 0;
}

int patternAsm::parseControlExpress(patternInst* inst,NStringA& line)
{
	int wordlen = 0;
	const char* lpword = line.firstword(wordlen,"=","\t =");
	int vid = 0;
	if (wordlen >= 4 && strnicmp(lpword,"mnem",4) == 0)
	{	//we parse imm?=
		vid = atoi(lpword + 4);
		if (vid == 0) return -1;
		int clen = 0;
		if (NStringA::getword(lpword,wordlen,clen,"!","\t ",true))
		{
			inst->flags |= PATTERN_FLAG_NOT;
		}
		line.mid(line.pos(lpword + wordlen));
		line.trim("\r\n \t");
		line.pop_front(1);
		line.trim("\r\n \t");
	}
	if (vid == 0) return -1;
	inst->varid = vid;
	line.trim("\r\n \t");
	while ((lpword = line.firstword(wordlen,"|","\t |=")) && wordlen > 0)
	{
		NStringA::strnupr((char*)lpword,wordlen);
		std::string smn;
		smn.append(lpword,wordlen);
		mnemonicmap::iterator itr = m_mnemonics.find(smn);
		if (itr != m_mnemonics.end())
		{
			ncasm::x86::mnemonic mne = (ncasm::x86::mnemonic)itr->second;
			pasm_opcode* mn = inst->mnemops.new_back();
			mn->mnem = mne;
		}
		line.mid(line.pos(lpword + wordlen));
	}

	return 0;
}


void patternAsm::loadMnemonic()
{
  m_mnemonics.clear();
  int nmnemonic = sizeof(ncasm::x86::mnemonicids) / sizeof(short);
  for (int i = 0; i < nmnemonic;i++)
  {
	  short mnic = ncasm::x86::mnemonicids[i];
	const char* mname = (const char*)GET_MNEMONIC_NAME(mnic);
	m_mnemonics.insert(std::make_pair(mname,mnic));
  }

}
//������һ��ƥ��ָ��
int patternAsm::parseInst(patternSequence* seq,pattern_inst_body* inst,NStringA& line)
{
	line.trim("\r\n \t");
	m_x86asm.inst_init(&inst->ins);
	int wordlen = 0;
	const char* lpword = line.firstword(wordlen,"\t ","\t ");
	while (lpword && wordlen > 0)
	{	//�������Ǽ��ָ���Ƿ�߱�ǰ׺������lock,rep,o16,a16�ȵ�
		ncasm::x86::inst_prefix prefix = ncasm::x86::checkPrefix(lpword, wordlen);
		if (prefix != ncasm::x86::prefix_none)
		{
			ncasm::x86::set_inst_prefix(&inst->ins, prefix);
			line.mid(line.pos(lpword + wordlen));
		} else break;
		line.trim("\r\n \t");
		lpword = line.firstword(wordlen,"\t ","\t ");
	}
	//��������Ҫ���Խ���ƥ��ָ�������趨���
	inst->szid = 0;
	inst->mnemonicid = 0;
	lpword = line.firstword(wordlen,"}","\t }",true);
	while (lpword && wordlen > 0 && *lpword == '{')
	{	//���Ƿֱ�����﷨���嵱�е�{size},{mnem},{strint},{!},{8}��ƥ��ָ���������
		lpword++;
		wordlen--;
		if (wordlen >= 4 && strnicmp(lpword,"size",4) == 0)
		{
			inst->szid = atoi(lpword + 4);
		} else if (wordlen >= 4 && strnicmp(lpword,"mnem",4) == 0)
		{
			inst->mnemonicid = atoi(lpword +4);
		} else if (wordlen >= 6 && strnicmp(lpword,"strict",6) == 0)
		{
			inst->cflags |= PATTERN_CFLAG_STRICT;	//�趨��Ӧ��ָ�����Բ���
		} else if (wordlen == 1 && *lpword == '!')
		{
			inst->cflags |= PATTERN_CFLAG_NOT;
		} else if (wordlen == 1 && *lpword == '8')
		{
			inst->cflags |= PATTERN_CFLAG_8BIT;
		} else if (wordlen == 2 && *(WORD*)lpword == 0x3631)
		{
			inst->cflags |= PATTERN_CFLAG_16BIT;
		} else if (wordlen == 2 && *(WORD*)lpword == 0x3233)
		{
			inst->cflags |= PATTERN_CFLAG_32BIT;
		} else if (wordlen == 2 && *(WORD*)lpword == 0x3436)
		{
			inst->cflags |= PATTERN_CFLAG_64BIT;
		}
		line.mid(line.pos(lpword + wordlen));
		line.pop_front(1);
		line.trim("\r\n \t");
		lpword = line.firstword(wordlen,"}","\t }",true);
	}
	//�������Ƿ���ƥ��ָ����䣬ע��any�ؼ����Ǳ����ģ���ʾ������ָ��ָ��
	lpword = line.firstword(wordlen,"\t ","\t ");
	if (!lpword || wordlen < 1) return -1;
	if (strnicmp(lpword, "any", wordlen) == 0)
	{
		ncasm::x86::set_inst_mnemonic(&inst->ins, I_UNDEFINED);
	} else
	{	//����������Ҫ�������ҳ�ָ�������
		NStringA::strnupr((char*)lpword, wordlen);
		std::string smn;
		smn.append(lpword, wordlen);
		mnemonicmap::iterator itr = m_mnemonics.find(smn);
		if (itr == m_mnemonics.end())
			return -2;
		ncasm::x86::mnemonic mne = (ncasm::x86::mnemonic)itr->second;
		ncasm::x86::set_inst_mnemonic(&inst->ins, mne);
	}
	line.mid(line.pos(lpword + wordlen));
	line.trim("\r\n \t");
	lpword = line.firstword(wordlen,",",", \t");
	int ridx = 0;
	while (lpword && wordlen > 0)
	{	//��������������������
		pattern_op* pop = &inst->ops[ridx];
		pop->szid = 0;
		pop->cflags = PATTERN_CFLAG_NONE;
		//parse each op's attrib
		int wordlen2 = 0;
		const char* lpword2 = line.firstword(wordlen2,"}",",\t }",true);
		while (lpword2 && wordlen2 > 0 && *lpword2 == '{')
		{	//��������������������趨���
			lpword2++;
			wordlen2--;
			if (strnicmp(lpword2,"size",4) == 0)
			{
				pop->szid = atoi(lpword2 + 4);
			} else if (wordlen2 == 1 && *lpword2 == '!')
			{
				pop->cflags |= PATTERN_CFLAG_NOT;	//�趨��Ӧ�Ĳ���������
			} else if (wordlen2 == 1 && *lpword2 == '*')
			{
				pop->cflags |= PATTERN_CFLAG_INALLOP;
			} else if (wordlen2 == 1 && *lpword2 == 'r')
			{
				pop->cflags |= PATTERN_CFLAG_REF;
			} else if (wordlen2 == 1 && *lpword2 == '8')
			{
				pop->cflags |= PATTERN_CFLAG_8BIT;
			} else if (wordlen2 == 2 && *(WORD*)lpword2 == 0x3631)
			{
				pop->cflags |= PATTERN_CFLAG_16BIT;
			} else if (wordlen2 == 2 && *(WORD*)lpword2 == 0x3233)
			{
				pop->cflags |= PATTERN_CFLAG_32BIT;
			} else if (wordlen2 == 2 && *(WORD*)lpword2 == 0x3436)
			{
				pop->cflags |= PATTERN_CFLAG_64BIT;
			} else if (wordlen2 == 2 && strnicmp(lpword2,"!r",2) == 0)
			{
				pop->cflags |= PATTERN_CFLAG_NOREF;
			}

			line.mid(line.pos(lpword2 + wordlen2));
			line.pop_front(1);
			line.trim("\r\n \t");
			lpword2 = line.firstword(wordlen2,"}","\t }",true);
		}
		lpword = line.firstword(wordlen,",",", \t");
		if (lpword && wordlen > 0)
		{	//�����������Ĳ����������﷨
			if (strnicmp(lpword,"op",2) == 0)
			{
				pop->type = pattern_op_any;	//�趨��Ӧ�Ĳ���������
				pop->id = atoi(lpword + 2);
			} else if (strnicmp(lpword,"reg",3) == 0)	
			{
				pop->type = pattern_op_reg;
				pop->id = atoi(lpword + 3);
			} else if (strnicmp(lpword,"imm",3) == 0)	
			{
				pop->type = pattern_op_imm;
				pop->id = atoi(lpword + 3);
			} else if (strnicmp(lpword,"mem",3) == 0)
			{
				pop->type = pattern_op_mem;
				pop->id = atoi(lpword + 3);
			} else
			{	//������Ǳ������ͣ���ôֱ�ӵ��������Ĳ�����������
				pop->type = pattern_op_none;
				if (m_x86asm.parse_op_text(&inst->ins,ridx,lpword,wordlen) < 0)
				{	//�������ʧ�ܣ����ǻ�����һ�ֶ�̬�ڴ�������Ľ���
					pop->type = pattern_op_memexpr;
					int dyid = seq->dyops.count();
					pasm_operand* pasmop = seq->dyops.new_back(dyid);
					pop->id = dyid;
					NStringA word;
					word.copy(lpword,wordlen);
					if (parseMemoryOperand(pasmop,word) < 0)
						return -3;
				} else
				{
					pop->type = pattern_op_explicit;
				}
					
			}
		}
		ridx++;
		line.mid(line.pos(lpword + wordlen));
		line.trim("\r\n \t");
		lpword = line.firstword(wordlen, ",", ", \t");
	}
	return 0;
}

int patternAsm::initForTest(bool breverse)
{
	m_seqList = 0;
	patternSequence* lastSeq = 0;
	int nc = 0;
	for (patternSequence* seq = m_Patterns.first(); seq; seq = m_Patterns.next(seq))
	{	//ѭ�����������Ѿ�����Ļ����б�
		if (seq->flags & PATTERN_SEQ_FLAG_OFF)	//��������б����ã����ǲ�ѡȡ
			continue;
		//���Ի��͵�����ģʽ�Ƿ��뵱ǰ��ƥ��ģʽ��ͬ���������ͬ���ǲ�ѡ��
		if (seq->mode == (breverse ? mode_reverse : mode_sequence))
		{
			if (!m_seqList) m_seqList = seq;
			if (lastSeq) lastSeq->next = seq;
			//�����ʾ�û��ͱ�ѡ���ˣ����ǳ�ʼ���û��͵���ؽṹ����Ҫ��ʼ��crt��Ա
			seq->prev = lastSeq;
			lastSeq = seq;
			lastSeq->next = 0;
			lastSeq->crt.count = 0;
			lastSeq->crt.vars.cleanup();
			lastSeq->crt.opvars = lastSeq->dyops;
			lastSeq->crt.inst = lastSeq->insts.first();
			nc++;
		}
	}
	//��������ѡȡ�����еĹؼ�����
	for (patternSequence* seq = m_seqList; seq; seq = seq->next)
	{
		for (patternInst* pi = seq->insts.first(); pi; pi = seq->insts.next(pi))
		{
			pi->matched_count = 0;
			pi->userdata = 0;
		}
	}
	return nc;
}

int patternAsm::testInstruction(const char* lpop,int szop,ULONG_PTR pc,bool isEspChanged,bool isEflagsChanged,void* userdata,patternSequence*& sr)
{
	ncasm::x86::inst ins;
	ncasm::x86::code ci;
	ci.code = (const uint8_t*)lpop;
	ci.codeLen = szop;
	ci.codeOffset = pc;
	ci.dt = Decode32Bits;	//ollydbgֻ֧��32λ���������Ӳ����32λ�����
	//ʵ�������ǵĽű�������ƶ��������û���޶���
	//���Ǵ��ݹ����Ĵ�����ָ���ֽ���������Ҫ�����
	int len = ncasm::x86::disasm(&ci, &ins, 1);	
	//ѭ�������Ѿ�ѡȡ�Ļ��ͽṹ
	for (patternSequence* seq = m_seqList; seq;)
	{	//����ÿһ�����ͽṹ�Ƿ�ƥ�䵱ǰ���ݹ�����ָ��
		int state = testForSequence(seq,&ins,isEspChanged,isEflagsChanged,userdata);
		if (state < 0)//not match
		{
			//���ƥ��ʧ�ܣ�������
			for (patternInst* pi = seq->insts.first(); pi; pi = seq->insts.next(pi))
			{
				pi->userdata = 0;
				pi->matched_count = 0;
			}
			//�ӻ����б����޳��û���
			if (m_seqList && m_seqList == seq)
				m_seqList = seq->next;
			if (!seq->prev && !seq->next)
			{
				m_seqList = 0;
				break;
			} else
			{
				if (seq->prev) seq->prev->next = seq->next;
				if (seq->next) seq->next->prev = seq->prev;
			}
		} else if (state == 1)
		{
			sr = seq;
			return 1; //ƥ��ɹ���
		}
		seq = seq->next;
	}
	if (m_seqList) return 0;	//��������б��л��л��ͣ���ô����
	return -1;
}

int patternAsm::testForSequence(patternSequence* seq,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged,void* userdata)
{
	if (seq->crt.count++ >= seq->maximumInst)	//ƥ�������Ƿ񳬳��������Χ
		return -6;
	bool instruction_psed = false;
	for (patternInst* pi = seq->crt.inst;pi;)	//�����������������ָ������ദ��
	{
		switch (pi->type)
		{
		case pattern_inst_instruction:	//ƥ��ָ��Ĵ�����testForInstruction�����������
		{
			if (instruction_psed) return 0;
			instruction_psed = true;
			int state = testForInstruction(seq,pi,ins,isEspChanged,isEflagsChanged,userdata);
			if (state < 0) return state;
			pi = seq->crt.inst;
			if (!pi)  return 1;
			if (pi->type != pattern_inst_instruction) continue;
			return 0;
		}break;
		case pattern_inst_express:	//���ʽָ�����ǽ���calculateExpress��������ֵ
		{
			int64_t val = 0;
			if (calculateExpress(seq,pi->expr,val) < 0)
				return -8;
			int vid = varid(pattern_op_imm,pi->varid);
			pasm_operand* op = seq->crt.vars.find(vid);
			if (!op)
				op = seq->crt.vars.new_back(vid);
			op->type = O_IMM;
			op->imm = val;
			pi = seq->insts.next(pi);
		}break;
		case pattern_inst_memexpr:	//�ڴ���ʽָ��Ҳ����calculateMemoryExpress����ֵ
		{
			int vid = varid(pattern_op_mem,pi->varid);
			pasm_operand* op = seq->crt.vars.find(vid);
			if (!op)
				op = seq->crt.vars.new_back(vid);
			if (calculateMemoryExpress(seq,pi->memexpr,op) < 0)
			{
				seq->crt.vars.remove(vid);
				return -7;
			}
			pi = seq->insts.next(pi);
		}break;
		case pattern_inst_control:	//�������������Ҫ���в���
		{
			int vid = varid(pattern_op_mnemonic,pi->varid);
			pasm_operand* op = seq->crt.vars.find(vid);
			if (!op)
				return -8;
			BOOL bmatch = FALSE;
			for (pasm_opcode* pao = pi->mnemops.first(); pao; pao = pi->mnemops.next(pao))
			{
				if (pao->mnem == op->opcode)
				{
					bmatch = TRUE;
					break;
				}
			}
			if (pi->flags & PATTERN_FLAG_NOT)
			{
				if (bmatch) return -9;
			} else
			{
				if (!bmatch) return -10;
			}
			pi = seq->insts.next(pi);		
		}break;
		default:pi = seq->insts.next(pi);break;
		}
	}
	return 1;
}

int patternAsm::varid(pattern_op_type type,int id)
{
	return ((int)type << 24) | id;
}

int patternAsm::isMatchInst(patternSequence* seq,patternInst* pi,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged)
{
	seq->crt.navar = 0;	//������������趨Ϊ0
	int state = -1;
	if ((pi->flags & PATTERN_FLAG_NOESPCHANGE) && isEspChanged)
	{
		state = -2;
	} else if ((pi->flags & PATTERN_FLAG_NOEFLAGSCHG) && isEflagsChanged)
	{
		state = -3;
	} else if ((pi->flags & PATTERN_CFLAG_16BIT) && (ins->size != 0 ))
	{
		state = -4;
	} else if ((pi->flags & PATTERN_CFLAG_32BIT) && (ins->size != 1))
	{
		state = -4;
	} else if ((pi->flags & PATTERN_CFLAG_64BIT) && (ins->size != 2))
	{
		state = -4;
	}else
	{
		bool hasMatched = false;
		for (pattern_inst_body* ib = pi->insts.first(); ib; ib = pi->insts.next(ib))
		{
			if (ib->szid > 0)
			{
				int vid = varid(pattern_op_size,ib->szid);
				pasm_operand* op = seq->crt.vars.find(vid);
				if (!op)
				{
					op = seq->crt.vars.new_back(vid);
					seq->crt.nvars[seq->crt.navar++] = vid;
					op->size = FLAGSIZE_TO_BITS(FLAG_GET_OPSIZE(ins->flags));
				} else
				{
					if (FLAGSIZE_TO_BITS(FLAG_GET_OPSIZE(ins->flags)) != op->size)
						continue;
				}
			}

			int st = isMatchInstBody(seq,pi,ib,ins);
			if (st >= 0)	//�����ƥ��
			{
				hasMatched = true;
				if (ib->mnemonicid > 0)
				{
					int vid = varid(pattern_op_mnemonic,ib->mnemonicid);
					pasm_operand* op = seq->crt.vars.find(vid);
					if (!op)
					{
						op = seq->crt.vars.new_back(vid);
						seq->crt.nvars[seq->crt.navar++] = vid;
						op->opcode = ins->opcode;
					} else
					{
						if (op->opcode != ins->opcode)
							hasMatched = false;
					}
				}
				
				break;
			}
		}
		if (hasMatched)
		{
			pi->matched_count++;
			state = 1;
		}
	}
	return state;
}

int patternAsm::genReplaceOpcode(patternSequence* seq,pattern_inst_body* pi,char* lpop,int szop,ULONG_PTR pc)
{
	ncasm::x86::inst ins = pi->ins;	//�������Ǵ�ָ��ṹ���и��Ƴ�������ָ��壬�����ڽű������ʱ�����ɵ�
	for (int i = 0; i < OPERANDS_NO; i++)	//���Ǳ������еĲ������趨��Ȼ��ֱ��趨ÿһ�������Ĳ�����
	{
		pattern_op* vop = &pi->ops[i];
		if (vop->type == pattern_op_none)
		{
			ins.ops[i].type = O_NONE;
			continue;
		}
		if (vop->type == pattern_op_explicit)
			continue;
		pasm_operand* iop = 0;
		//������ڴ��������������Ҫʵʱ���㣬��Ϊ���ڴ����������Ҳ����ʹ�ñ���
		if (vop->type == pattern_op_memexpr)
		{
			iop = seq->crt.opvars.find(vop->id);
			if (!iop)
				return -2;
			resolveVar(seq,iop,vop);	//��������
		} else
		{	//���ֻ�ǵ�һ�ı�����ֱ��ȡ������ֵ�Ϳ����ˣ���Ϊ��ƥ������У��������ѳ�ʼ��
			int vid = varid(vop->type,vop->id);
			iop = seq->crt.vars.find(vid);
			if (!iop)
				return -1;
		}//���ݱ���趨ָ���С�����������ǿ��ָ���˴�С
		int szop = iop->size;
		if (vop->cflags & PATTERN_CFLAG_8BIT)
			szop = 8;
		else if (vop->cflags & PATTERN_CFLAG_16BIT)
			szop = 16;
		else if (vop->cflags & PATTERN_CFLAG_32BIT)
			szop = 32;
		else if (vop->cflags & PATTERN_CFLAG_64BIT)
			szop = 64;
		
		m_x86asm.set_op(&ins,i,(ncasm::x86::optype)iop->type,iop->imm,szop,iop->base,iop->index,iop->scale);
	}
	//����滻ָ���е�ָ���С��ʹ�ñ�����ָ���ģ���ô���������ﴦ��
	if (pi->szid > 0)
	{
		pasm_operand* pop = seq->crt.vars.find(varid(pattern_op_size,pi->szid));
		if (pop)
		{
			ins.flags &= ~(7 << 8);
			switch (pop->size)
			{
			case 16:FLAG_SET_OPSIZE((&ins),0);
			case 32:FLAG_SET_OPSIZE((&ins),1);
			case 64:FLAG_SET_OPSIZE((&ins),2);
			}
		}
	}
	//���ָ��ʹ���﷨ǿ��ָ���˴�С����������������
	if (pi->cflags & PATTERN_CFLAG_16BIT)
	{
		ins.flags &= ~(7 << 8);
		ins.size = 0;
		FLAG_SET_OPSIZE((&ins),0);
	} else if (pi->cflags & PATTERN_CFLAG_32BIT)
	{
		ins.flags &= ~(7 << 8);
		ins.size = 1;
		FLAG_SET_OPSIZE((&ins),1);
	} else if (pi->cflags & PATTERN_CFLAG_64BIT)
	{
		ins.flags &= ~(7 << 8);
		ins.size = 2;
		FLAG_SET_OPSIZE((&ins),2);
	}
	//��������ȡ��ָ������Ȼ���趨
	if (pi->mnemonicid > 0)
	{
		pasm_operand* pop = seq->crt.vars.find(varid(pattern_op_mnemonic,pi->mnemonicid));
		if (pop)
			m_x86asm.set_inst_mnemonic(&ins,(ncasm::x86::mnemonic)pop->opcode);
	}
	//һ��ָ����Ϣ�趨��Ϻ�����ֱ��ͨ��������潫ָ�����Ϊ�ֽ���
	int len = m_x86asm.inst_to_bc(&ins,lpop,szop);
	return len;
}

int patternAsm::checkAndSaveVar(patternSequence* seq,pattern_op* pop,ncasm::x86::inst* ins,ncasm::x86::operand* iop)
{
	int vid;
	switch (iop->type)
	{
	case O_REG:
	{
		vid = varid(pop->type,pop->id);
		pasm_operand* op = seq->crt.vars.find(vid);
		if (!op)
		{
			//��������û���δ���ñ��({!r} or {r})��ô������δ��ʼ��ʱʧ��
			if ((pop->cflags & PATTERN_CFLAG_NOREF) || (pop->cflags & PATTERN_CFLAG_REF))
				return -5;
			op = seq->crt.vars.new_back(vid);
			seq->crt.nvars[seq->crt.navar++] = vid;
			op->type = O_REG;
			op->base = iop->index;
			op->size = iop->size;
		} else
		{
			if (pop->cflags & PATTERN_CFLAG_NOREF)
			{
				if (ncasm::x86::get_relate_reg(op->base,32) == ncasm::x86::get_relate_reg(iop->index,32))
					return -5;
			} else if (pop->cflags & PATTERN_CFLAG_REF)
			{
				if (ncasm::x86::get_relate_reg(op->base,32) != ncasm::x86::get_relate_reg(iop->index,32))
					return -6;
			} else
			{
				if (iop->index != op->base || iop->size != op->size)
					return -3;
			}
		}
	}break;
	case O_IMM:case O_IMM1:case O_IMM2:case O_PC:case O_PTR:
	{
		vid = varid(pop->type,pop->id);
		pasm_operand* op = seq->crt.vars.find(vid);
		if (!op)
		{
			if ((pop->cflags & PATTERN_CFLAG_NOREF) || (pop->cflags & PATTERN_CFLAG_REF))
				return -5;
			op = seq->crt.vars.new_back(vid);
			seq->crt.nvars[seq->crt.navar++] = vid;
			op->type = O_IMM;
			op->imm = ins->imm.sqword;
			op->size = iop->size;
		} else
		{
			if (pop->cflags & PATTERN_CFLAG_REF)
			{
					return -8;
			} else if (pop->cflags & PATTERN_CFLAG_NOREF)
			{
			}else
			{
				if (op->imm != ins->imm.sqword || op->size != iop->size)
					return -4;
			}

		}
	}break;
	case O_MEM:case O_SMEM:case O_DISP:
	{
		pasm_operand* op = 0;
		if (pop->type == pattern_op_memexpr)
		{
			op = seq->crt.opvars.find(pop->id);
			if (!op) return -21;
			pasm_operand opan = *op;	//�����ȳ��Խ���һ��opan����
			resolveVar(seq,&opan,pop);
			op = &opan;
		} else
		{
			vid = varid(pop->type,pop->id);
			op = seq->crt.vars.find(vid);
			if (!op)
			{
				if ((pop->cflags & PATTERN_CFLAG_NOREF) || (pop->cflags & PATTERN_CFLAG_REF))
					return -5;
				op = seq->crt.vars.new_back(vid);
				seq->crt.nvars[seq->crt.navar++] = vid;
				op->type = iop->type;
				op->imm = ins->disp;
				op->size = iop->size;
				op->base = (iop->type == O_MEM) ? ins->base : iop->index;
				op->index = (iop->type == O_MEM) ? iop->index : R_NONE;
				op->scale = (iop->type == O_MEM) ? ins->scale : 0;
			}
		}
		if (op)
		{
			if (pop->cflags & PATTERN_CFLAG_NOREF)
			{
				if (iop->type == O_MEM)
				{
					if (op->base != R_NONE && op->base < op_varid_base)
					{
						if (ncasm::x86::get_relate_reg(op->base,32) == ncasm::x86::get_relate_reg(ins->base,32))
							return -5;
					}
					if (op->index != R_NONE && op->index < op_varid_base)
					{
						if (ncasm::x86::get_relate_reg(op->index,32) == ncasm::x86::get_relate_reg(iop->index,32))
							return -6;
					}
				} else if (iop->type == O_SMEM)
				{
					if (op->base != R_NONE && op->base < op_varid_base)
					{
						if (ncasm::x86::get_relate_reg(op->base,32) == ncasm::x86::get_relate_reg(iop->index,32))
							return -5;
					}
				}
			} else if (pop->cflags & PATTERN_CFLAG_REF)
			{
				bool hasref = false;
				if (iop->type == O_MEM)
				{
					if (op->base != R_NONE && op->base < op_varid_base)
					{
						if (ncasm::x86::get_relate_reg(op->base,32) == ncasm::x86::get_relate_reg(ins->base,32))
							hasref = true;
					}
					if (op->index != R_NONE && op->index < op_varid_base)
					{
						if (ncasm::x86::get_relate_reg(op->index,32) != ncasm::x86::get_relate_reg(iop->index,32))
							hasref = true;
					}
				} else if (iop->type == O_SMEM)
				{
					if (op->base != R_NONE && op->base < op_varid_base)
					{
						if (ncasm::x86::get_relate_reg(op->base,32) != ncasm::x86::get_relate_reg(iop->index,32))
							hasref = true;
					}
				}
				if (!hasref)
					return -8;
			} else
			{
				if (op->type != iop->type)
					return -4;
				if (op->size != 0 && op->size != iop->size)
					return -19;
				if ((op->dispvar < op_varid_base) && op->imm != ins->disp)
					return -7;
				if (op->type == O_MEM)
				{
					if (op->scale != ins->scale)
						return -6;
					if ((op->base < op_varid_base) && op->base != ins->base)
						return -5;
					if ((op->index < op_varid_base) && op->index != iop->index)
						return -4;
				} else
				{
					if (ins->scale != 0)
						return -6;
					if (ins->base != R_NONE)
						return -5;
					if ((op->base < op_varid_base) && op->base != iop->index)
						return -4;
				}
			}
		}
	}break;
	}
	return 1;
}

int patternAsm::isMatchInstBody(patternSequence* seq,patternInst* pi,pattern_inst_body* ib,ncasm::x86::inst* ins)
{
	int vid;

	if (ib->ins.opcode != I_UNDEFINED)
	{
		if (ib->cflags & PATTERN_CFLAG_NOT)
		{
			if (ins->opcode == ib->ins.opcode)
				return -1;
		} else
		{
			if (ins->opcode != ib->ins.opcode)
				return -1;
		}
	}
	for (int i = 0; i < OPERANDS_NO; i++)
	{
		pattern_op* pop = &ib->ops[i];
		if (pop->type == pattern_op_none && !(ib->cflags & PATTERN_CFLAG_STRICT))
			continue;
		ncasm::x86::operand* iop = &ins->ops[i];
		if (pop->cflags & PATTERN_CFLAG_INALLOP)
		{
			bool matched = true;
			for (int j = 0; j < OPERANDS_NO; j++)
			{
				iop = &ins->ops[j];
				if (iop->type == O_NONE)
					break;
				matched = false;
				int st = isMatchInstOperand(seq,pi,ib,pop,i,iop,j,ins);
				if (st < 0)
				{
					if (pop->cflags & PATTERN_CFLAG_NOT)
					{
						matched = true;
					}
				} else
				{
					if (!(pop->cflags & PATTERN_CFLAG_NOT))
					{
						matched = true;
					}
				}
				if (!matched) break;
			}
			if (!matched)
				return -12;
		} else
		{
			int st = isMatchInstOperand(seq,pi,ib,pop,i,iop,i,ins);
			if (st < 0)
			{
				if (!(pop->cflags & PATTERN_CFLAG_NOT))
					return st;
			} else
			{
				if (pop->cflags & PATTERN_CFLAG_NOT)
					return -11;
			}
		}
	}
	return 1;
}

int patternAsm::testForInstruction(patternSequence* seq,patternInst* pi,ncasm::x86::inst* ins,bool isEspChanged,bool isEflagsChanged,void* userdata)
{
	int state = isMatchInst(seq,pi,ins,isEspChanged,isEflagsChanged);
	if (state >= 0)
	{
		pi->userdata = userdata;
		if (pi->matched_count < pi->minc)
			return 0;
		if (pi->matched_count <= pi->maxc)
		{
			if (pi->matched_count == pi->maxc)
			{
				seq->crt.inst = seq->insts.next(pi);
			} else if (pi->flags & PATTERN_FLAG_LESSMATCH)
			{
				pi = seq->insts.next(pi);
				if (pi)
				{
					state = isMatchInst(seq,pi,ins,isEspChanged,isEflagsChanged);
					if (state >= 0)
					{
						pi->userdata = userdata;
						seq->crt.inst = seq->insts.next(pi);
					} else
					{	//����������ƥ�䣬��ô��ƥ��ʧ�ܵ����������Ҫ�������ı���
						for (int i = 0; i < seq->crt.navar;i++)
							seq->crt.vars.remove(seq->crt.nvars[i]);
					}
				}
			}
			return 0;	//̰��ģʽ��ƣ������ƥ�䣬����ƥ��
		}
		state = -7;
	}
	if (pi->matched_count < pi->minc)
		return -2;
	seq->crt.inst = pi = seq->insts.next(pi);
	//��������һ��ƥ��
	if (pi)
	{
		state = isMatchInst(seq,pi,ins,isEspChanged,isEflagsChanged);
		if (state >= 0)
		{
			pi->userdata = userdata;
			if (pi->matched_count < pi->minc)
				return 0;
			if (pi->matched_count <= pi->maxc)
			{
				if (pi->matched_count == pi->maxc)
				{
					seq->crt.inst = seq->insts.next(pi);
				} else if (pi->flags & PATTERN_FLAG_LESSMATCH)
				{
					pi = seq->insts.next(pi);
					if (pi)
					{
						state = isMatchInst(seq,pi,ins,isEspChanged,isEflagsChanged);
						if (state >= 0)
						{
							pi->userdata = userdata;
							seq->crt.inst = seq->insts.next(pi);
						} else
						{	//����������ƥ�䣬��ô��ƥ��ʧ�ܵ����������Ҫ�������ı���
							for (int i = 0; i < seq->crt.navar; i++)
								seq->crt.vars.remove(seq->crt.nvars[i]);
						}
					}
				}
				return 0;	//̰��ģʽ��ƣ������ƥ�䣬����ƥ��
			}
		}
	}
	return -1;
}

int patternAsm::isMatchInstOperand(patternSequence* seq,patternInst* pi,pattern_inst_body* ib,pattern_op* pop,int ivop,
									ncasm::x86::operand* iop,int ix86op,ncasm::x86::inst* ins)
{
	if ((pop->cflags & PATTERN_CFLAG_8BIT) && iop->size != 8)
		return -10;
	if ((pop->cflags & PATTERN_CFLAG_16BIT) && iop->size != 16)
		return -10;
	if ((pop->cflags & PATTERN_CFLAG_32BIT) && iop->size != 32)
		return -10;
	if ((pop->cflags & PATTERN_CFLAG_64BIT) && iop->size != 64)
		return -10;
	if (pop->szid > 0)
	{
		int vid = varid(pattern_op_size,pop->szid);
		pasm_operand* op = seq->crt.vars.find(vid);
		if (!op)
		{
			op = seq->crt.vars.new_back(vid);
			seq->crt.nvars[seq->crt.navar++] = vid;
			op->size = iop->size;
		} else
		{
			if (iop->size != op->size)
				return -9;
		}
	}

	switch (pop->type)
	{
	case pattern_op_none:
	{
		if (pop->cflags & PATTERN_CFLAG_REF)
		{
			return -8;
		} else if (pop->cflags & PATTERN_CFLAG_NOREF)
		{
		} else
		{
			if (iop->type != O_NONE)
				return -2;
		}
	}break;
	case pattern_op_any:
	{
		pattern_op opany = *pop;
		int state = checkAndSaveVar(seq,pop,ins,iop);
		if (state < 0) return state;
	}break;
	case pattern_op_reg:
	{
		if (!((pop->cflags & PATTERN_CFLAG_REF) || (pop->cflags & PATTERN_CFLAG_NOREF)))
		{
			if (iop->type != O_REG)
				return -3;
		}
		int state = checkAndSaveVar(seq,pop,ins,iop);
		if (state < 0) return state;
	}break;
	case pattern_op_imm:
	{
		if (!((pop->cflags & PATTERN_CFLAG_REF) || (pop->cflags & PATTERN_CFLAG_NOREF)))
		{
			if (iop->type != O_IMM && iop->type != O_IMM1 && iop->type != O_IMM2
				&& iop->type != O_PC && iop->type != O_PTR)
				return -4;
		}
		int state = checkAndSaveVar(seq,pop,ins,iop);
		if (state < 0) return state;
	}break;
	case pattern_op_mem:
	{
		if (!((pop->cflags & PATTERN_CFLAG_REF) || (pop->cflags & PATTERN_CFLAG_NOREF)))
		{
			if (iop->type != O_MEM && iop->type != O_SMEM && iop->type != O_DISP)
				return -5;
		}
		int state = checkAndSaveVar(seq,pop,ins,iop);
		if (state < 0) return state;
	}break;
	case pattern_op_explicit:
	{
		ncasm::x86::operand* op1 = &ib->ins.ops[ivop];
		ncasm::x86::operand* op2 = &ins->ops[ix86op];
		if (pop->cflags & PATTERN_CFLAG_REF)
		{
			if (op1->index == O_NONE && ib->ins.base == O_NONE)
				return -9;
			bool hasref = false;
			if (op1->index != O_NONE)
			{
				if (iop->index != O_NONE)
				{
					if (ncasm::x86::get_relate_reg(op1->index,32) == ncasm::x86::get_relate_reg(iop->index,32))
						hasref = true;
				}
				if (ins->base != O_NONE)
				{
					if (ncasm::x86::get_relate_reg(op1->index,32) == ncasm::x86::get_relate_reg(ins->base,32))
						hasref = true;
				}
			}

			if (ib->ins.base != O_NONE)
			{
				if (iop->index != O_NONE)
				{
					if (ncasm::x86::get_relate_reg(ib->ins.base,32) == ncasm::x86::get_relate_reg(iop->index,32))
						hasref = true;
				}
				if (ins->base != O_NONE)
				{
					if (ncasm::x86::get_relate_reg(ib->ins.base,32) == ncasm::x86::get_relate_reg(ins->base,32))
						hasref = true;
				}
			}
			if (!hasref)
				return -10;
		} else if (pop->cflags & PATTERN_CFLAG_NOREF)
		{
			if (op1->index != O_NONE)
			{
				if (iop->index != O_NONE)
				{
					if (ncasm::x86::get_relate_reg(op1->index,32) == ncasm::x86::get_relate_reg(iop->index,32))
						return -5;
				}
				if (ins->base != O_NONE)
				{
					if (ncasm::x86::get_relate_reg(op1->index,32) == ncasm::x86::get_relate_reg(ins->base,32))
						return -6;
				}
			}
			if (ib->ins.base != O_NONE)
			{
				if (iop->index != O_NONE)
				{
					if (ncasm::x86::get_relate_reg(ib->ins.base,32) == ncasm::x86::get_relate_reg(iop->index,32))
						return -5;
				}
				if (ins->base != O_NONE)
				{
					if (ncasm::x86::get_relate_reg(ib->ins.base,32) == ncasm::x86::get_relate_reg(ins->base,32))
						return -6;
				}
			}
		} else
		{
			if (pop->szid > 0)
			{
				if (op1->type != op2->type)
					return -6;
				switch (op1->type)
				{
				case O_SMEM:{
								if (op1->index != op2->index || ib->ins.disp != ins->disp)
									return -7;
				}break;
				case O_MEM:{
								if (op1->index != op2->index || ib->ins.disp != ins->disp || ib->ins.base != ins->base)
									return -8;
				}break;
				case O_DISP:
				{
								if (ib->ins.disp != ins->disp)
									return -9;
				}break;
				default:{
							if (!ncasm::x86::is_same_operand(&ib->ins,ivop,ins,ix86op))
								return -6;
				}break;
				}
			} else
			{
				if (!ncasm::x86::is_same_operand(&ib->ins,ivop,ins,ix86op))
					return -6;
			}
		}

	}break;
	case pattern_op_memexpr:
	{
		if (!((pop->cflags & PATTERN_CFLAG_REF) || (pop->cflags & PATTERN_CFLAG_NOREF)))
		{
			if (iop->type != O_MEM && iop->type != O_SMEM && iop->type != O_DISP)
				return -5;
		}
		int state = checkAndSaveVar(seq,pop,ins,iop);
		if (state < 0) return state;
		//��ƥ�������£����Ǳ���δ��ʼ���������±���
		saveMemoryExprVar(seq,pop,ins,iop);
	}break;
	}

	return 0;
}

int patternAsm::calculateExpress(patternSequence* seq,nexpr_expr* expr,int64_t& val)
{
	switch (expr->type)
	{
	case nexpr_expr_imm:
	{
		nexpr_value_imm* imm = reinterpret_cast<nexpr_value_imm*>(expr);
		val = imm->imm;
	}break;
	case nexpr_expr_float:{
		nexpr_value_float* floatv = reinterpret_cast<nexpr_value_float*>(expr);
		val = floatv->doubleValue;
	}break;
	case nexpr_expr_var:
	{
		nexpr_value_var* var = reinterpret_cast<nexpr_value_var*>(expr);
		int vid = 0;
		if (strnicmp(var->name.data(),"imm",3) == 0)
		{
			vid = varid(pattern_op_imm,atoi(var->name.data() + 3));
			pasm_operand* op = seq->crt.vars.find(vid);
			if (!op) return -1;
			val = op->imm;
		} else if (strnicmp(var->name.data(),"size",4) == 0)
		{
			vid = varid(pattern_op_size,atoi(var->name.data() + 4));
			pasm_operand* op = seq->crt.vars.find(vid);
			if (!op) return -1;
			val = op->size;
		}
	}break;
	default:{
		int64_t valleft = 0;
		int64_t valright = 0;
		if (expr->left)
			calculateExpress(seq,expr->left,valleft);
		if (expr->right)
			calculateExpress(seq,expr->right,valright);
		switch (expr->oper)
		{
		case nexpr_oper_not:{

		}break;
		case nexpr_oper_and:{
								val = valleft & valright;
		}break;
		case nexpr_oper_or:{
							   val = valleft | valright;
		}break;
		case nexpr_oper_xor:{
								val = valleft ^ valright;
		}break;
		case nexpr_oper_shl:{
								val = valleft << valright;
		}break;
		case nexpr_oper_shr:{
								val = valleft >> valright;
		}break;
		case nexpr_oper_add:{
								val = valleft + valright;
		}break;
		case nexpr_oper_sub:{
								val = valleft - valright;
		}break;
		case nexpr_oper_mul:{
								val = valleft * valright;
		}break;
		case nexpr_oper_div:{
								val = valleft / valright;
		}break;
		case nexpr_oper_mod:{
								val = valleft % valright;
		}break;
		}

	}break;
	}
	if (expr->flags & nexpr_oper_flag_not)
		val = ~val;
	if (expr->flags & nexpr_oper_flag_neg)
		val = -val;
	return 0;
}

int patternAsm::calculateMemoryExpress(patternSequence* seq,pasm_operand* expr,pasm_operand* op)
{
	op->type = expr->type;
	op->scale = expr->scale;
	pasm_operand* opasm = seq->crt.vars.find(expr->base);
	if (opasm)
	{
		op->base = opasm->base;
	} else
	{
		if (expr->base >= op_varid_base)
			return -1;
		op->base = expr->base;
	}
		
	opasm = seq->crt.vars.find(expr->index);
	if (opasm)
	{
		op->index = opasm->base;
	} else
	{
		if (expr->index >= op_varid_base)
			return -2;
		op->index = expr->index;
	}
		
	opasm = seq->crt.vars.find(expr->size);
	if (opasm)
	{
		op->size = opasm->size;
	} else
	{
		if (expr->size >= op_varid_base)
			return -3;
		op->size = expr->size;
	}
	opasm = seq->crt.vars.find(expr->dispvar);
	if (opasm)
	{
		op->imm = opasm->imm;
	} else
	{
		if (expr->dispvar >= op_varid_base)
			return -4;
		op->imm = expr->imm;
	}
	

	return 0;
}

int patternAsm::resolveVar(patternSequence* seq,pasm_operand* op,pattern_op* pop)
{
	while (op->size >= op_varid_base)
	{
		pasm_operand* asmop = seq->crt.vars.find(op->size);
		if (asmop)
			op->size = asmop->size;
		else
			break;
	}
	if (pop->szid > 0)
	{
		//���������ﳢ���Ƿ����ñ���ָ���˴�С�����û���½�
		int vid = varid(pattern_op_size,pop->szid);
		pasm_operand* op1 = seq->crt.vars.find(vid);
		if (!op1)
		{
			if (op1->size < op_varid_base)
			{
				op1 = seq->crt.vars.new_back(vid);
				seq->crt.nvars[seq->crt.navar++] = vid;
				op1->size = op->size;
			}

		} else
		{
			op->size = op1->size;
		}
	}
	while (op->base >= op_varid_base)
	{
		pasm_operand* asmop = seq->crt.vars.find(op->base);
		if (asmop)
			op->base = asmop->base;
		else
			break;
	}
	while (op->index >= op_varid_base)
	{
		pasm_operand* asmop = seq->crt.vars.find(op->index);
		if (asmop)
			op->index = asmop->index;
		else
			break;
	}
	while (op->dispvar >= op_varid_base)
	{
		pasm_operand* asmop = seq->crt.vars.find(op->dispvar);
		if (asmop)
		{
			op->dispvar = asmop->dispvar;
			op->imm = asmop->imm;
		}
		else
			break;
	}

	return 0;
}

int patternAsm::saveMemoryExprVar(patternSequence* seq,pattern_op* pop,ncasm::x86::inst* ins,ncasm::x86::operand* iop)
{
	pasm_operand* op = seq->crt.opvars.find(pop->id);
	if (!op) return -1;
	op->type = iop->type;
	op->scale = (iop->type == O_MEM) ? ins->scale : 0;

	if (op->dispvar >= op_varid_base)
	{
		pasm_operand* asmop = seq->crt.vars.find(op->dispvar);
		if (!asmop)
		{
			asmop = seq->crt.vars.new_back(op->dispvar);
			seq->crt.nvars[seq->crt.navar++] = op->dispvar;
		}
		asmop->type = O_IMM;
		asmop->imm = ins->disp;
	}
	if (op->size >= op_varid_base)
	{
		pasm_operand* asmop = seq->crt.vars.find(op->size);
		if (!asmop)
		{
			asmop = seq->crt.vars.new_back(op->size);
			seq->crt.nvars[seq->crt.navar++] = op->size;
		}
		asmop->type = O_IMM;
		asmop->imm = iop->size;
	}
	if (op->base >= op_varid_base)
	{
		pasm_operand* asmop = seq->crt.vars.find(op->base);
		if (!asmop)
			asmop = seq->crt.vars.new_back(op->base);
		asmop->type = O_REG;
		asmop->base = (iop->type == O_MEM) ? ins->base : iop->index;
	}
	if (op->index >= op_varid_base)
	{
		pasm_operand* asmop = seq->crt.vars.find(op->index);
		if (!asmop)
		{
			asmop = seq->crt.vars.new_back(op->index);
			seq->crt.nvars[seq->crt.navar++] = op->index;
		}
			
		asmop->type = O_REG;
		asmop->base = (iop->type == O_MEM) ? iop->index : R_NONE;
	}
	resolveVar(seq,op,pop);
	return 0;
}

patternSequence* patternAsm::getPattern(int pid)
{
	return m_Patterns.find(pid);
}


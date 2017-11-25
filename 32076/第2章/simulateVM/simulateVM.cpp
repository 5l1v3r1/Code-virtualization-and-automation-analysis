// simulateVM.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include "Q:/Projects/nSafeSuite/ncvm/nxfmt/nxfmt86.h"
#include "../VMCrt/VMCrt.h"
/************************************************************************/
/* ���⻯ý�������+������                                              */
/************************************************************************/
// Parameter: long rva	ָ�������rva��ʵ������������û��ʹ��
// Parameter: const unsigned char * lpCode	ָ�����⻯Ŀ������ڴ�λ��
// Parameter: int szCode	���⻯Ŀ������С
// Parameter: char * lpOpcode	���������⻯ý��(opcode)�ڴ��ַ
//************************************
int encodeOpcode(long rva,const unsigned char* lpCode,int szCode,char* lpOpcode)
{	
	int oppos = 0;
	const unsigned char* lpend = lpCode + szCode;
	while (lpCode < lpend)
	{
		//�Ա����⻯ָ��������룬�������������������û��ʹ�÷�����������ֱ��Ӳ��
		switch (*lpCode)	
		{
		case 0xB8://mov eax,imm	//������Ϊ������֪�����⻯ָ����Ӳ��
		{
			lpOpcode[oppos++] = op_mov;	//��������opcodeΪmov�Ķ���
			lpOpcode[oppos++] = VREG_RAX;	//��ԭʼָ��еĲ������뵽opcode����
			*(long*)(lpOpcode + oppos) = *(long*)(lpCode+1);	//�����������ȥ
			oppos += sizeof(long);
			lpCode += 5;
		}break;
		case 0x05://add eax,imm
		{
			lpOpcode[oppos++] = op_add;	//����Ϊadd
			lpOpcode[oppos++] = VREG_RAX;	//�������
			*(long*)(lpOpcode + oppos) = *(long*)(lpCode+1);	//���볣��
			oppos += sizeof(long);
			lpCode += 5;
		}break;
		}
	}
	lpOpcode[oppos++] = op_exit;	//������Ǳ���һ���˳��������˳������
	*(long*)(lpOpcode + oppos) = 0x401010;	//ָ���˳���ַ
	oppos += sizeof(long);
	return oppos;
}

int _tmain(int argc, _TCHAR* argv[])
{
	nxfmt::pe xpesrc; //�ֱ�������PE�ļ������࣬ʵ���ϸ����PE�ж�ȡ����д������
	nxfmt::pe xpedst;
	nxfmt::pe xpevmcrt;
		//���⻯Ŀ���ʽ
	if (xpesrc.open(_T("Q:\\Documents\\Books\\third\\code\\ģ�����⻯\\vm1.exe"),nxfmt::open_readonly) < 0)
		return -1;
	//���ǵ�����ʱ�����ģ���ļ�
	if (xpevmcrt.open(_T("Q:\\Documents\\Books\\third\\code\\ģ�����⻯\\simulateVM\\Release\\VMCrt.dll"),nxfmt::open_readonly) < 0)
		return -2;
	//���ɵ��³�ʽ�ļ�
	if (xpedst.open(_T("Q:\\Documents\\Books\\third\\code\\ģ�����⻯\\vm1_out.exe"),nxfmt::open_readwrite) < 0)
		return -3;
	//������ȡ���������⻯Ŀ�������ڴ��ַ��0x1006������������֪�ı����⻯ָ���RVA��ַ
	const unsigned char* lptc = (const unsigned char*)xpesrc.data(0x1006);
	//����һ���ռ��ڻ������⻯ý��
	char opbuf[0x100];
	//��������ͱ���
	int oplen = encodeOpcode(0x1006,lptc,10,opbuf);
	//����ԭʼ�ṩ�ĳ�ʽ���Ƴ�һ��ֻ��������PE���ݵ��ļ���Ҳ���Ǹ������ݱ��޳�
	xpedst.copyFromPE(xpesrc.data(),xpesrc.size());
	//�ڸ��Ƴ������ļ��������һ�����ε�ĩβ�����ҽ������趨Ϊ��ִ�У�0x10000��С
	long secrva = xpedst.add_section(0x10000,0,0x10000,0xE0000020,".vm");
	if (secrva == 0)
		return -4;
	//ȡ����������ε��ڴ��ַ���������ǾͿ���д��������
	char* lpsec = xpedst.data(secrva);
	//���������Ƚ�opcodeд�����ӵ�����
	memcpy(lpsec,opbuf,oplen);
	//�������Ƕ�ȡ������ʱ���������
	const char* lpvmcrt = xpevmcrt.data(0x1000);
	long vmcrtentry = xpevmcrt.entry();
	//���������������ʱ���벻����1000�ֽڴ�С
	char crtbuf[1000];
	memcpy(crtbuf,lpvmcrt,sizeof(crtbuf));
	//��������Ҫ��������ʱ�������ھ�������ƫ�ƣ�Ȼ����������������ʱ�������ʱ������opcode��ַ
	long oftentry = vmcrtentry + 1 - 0x1000;
	//��������Ҫ��opcode��ַ����������ʱ���뵱��ȥ
	*(long*)(crtbuf + oftentry) = secrva+ 0x400000;
	//������ʱ����д��Ŀ���ļ�����
	memcpy(lpsec + oplen,crtbuf,sizeof(crtbuf));
	//����������������ʱ��������
	long addrentry = vmcrtentry - 0x1000 + oplen + secrva;
	//�������Ƕ�λ�������⻯����
	char* lpOrg = xpedst.data(0x1006);
	//���ڴ����Ѿ������⻯��������ǿ��Խ�ԭʼ�Ĵ��������
	memset(lpOrg,0x90,10);
	//������ԭʼ����ִ�д����һ������ת��ָ��
	*lpOrg++ = 0xE9;
	*(long*)lpOrg = addrentry - 0x1006 - 5;
	//�����չ�
	xpedst.close();
	return 0;
}


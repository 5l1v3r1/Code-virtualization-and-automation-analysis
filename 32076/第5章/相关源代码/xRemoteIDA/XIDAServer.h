#pragma once

#include "../../../../nNetLib/nNetBase.h"
#include "../../../../nCom/npacketbaseT.h"
#include "../../../../nCom/nsafelocker.h"
struct remote_ollydbg{
	nnet_inst pipe;
	NCLStringA	sid;
	struct remote_ollydbg()
	{
		pipe = 0;
	}
};


class XIDAServer :
	public NNetBase
{
public:
	XIDAServer();
	virtual ~XIDAServer();
	void broadcast(NPacketBuffer* pk);
protected:
	//ʵ��������ͨ�ſ�Զ�����ӽ���֪ͨ�麯��
	virtual	BOOL	onInstOpen(nnet_inst inst, s_base_inst* binst);
	//ʵ��������ͨ�ſ�Զ�����ӹر�֪ͨ�麯��
	virtual void	onInstClose(nnet_inst inst, s_base_inst* binst);
	//ʵ��������ͨ�ſ�Զ�������쳣֪ͨ�麯��
	virtual void	onInstError(nnet_inst inst, s_base_inst* binst, int err);
	//ʵ��������ͨ�ſ�Զ���������ݽ���֪ͨ�麯��
	virtual int	onInstRead(nnet_inst inst, s_base_inst* binst, const char* pdata, size_t pszdata);

	NSafeLocker		m_odlk;
	NAvlList<NCLStringA,remote_ollydbg>	m_odbgs;
};


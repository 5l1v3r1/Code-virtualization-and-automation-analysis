#ifndef xstate_h__
#define xstate_h__
//����һ��״̬ʵ��
void*	xst_allocstate();
//����ĳ����Χ�ڵ�״̬
int		xst_set(void* lpst,int oft,int size,int state);
//ȡ��ĳ����Χ�ڵ�״̬
int		xst_unset(void* lpst,int oft,int size);
//����״̬
int		xst_hit(void* lpst,int* oft,int size,int* state);
#endif // xstate_h__

// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>

#define __NT__
#define  __IDP__
#include <ida.hpp>
#include <idp.hpp>
#include <err.h>
#include <md5.h>
#include <dbg.hpp>
#include <auto.hpp>
#include <name.hpp>
#include <frame.hpp>
#include <loader.hpp>
#include <diskio.hpp>
#include <struct.hpp>
#include <typeinf.hpp>
#include <demangle.hpp>
#include <nalt.hpp>
#include <bytes.hpp>
#pragma comment(lib,"ida.lib")
#pragma comment(lib,"pro.lib")


#include "../../../../nNetLib/npipeclient.h"
#include "../../../../nCom/npacketbaseT.h"


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�

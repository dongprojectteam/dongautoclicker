#pragma once

#include <vector>

class CMouseInformation
{
public:
	CMouseInformation(void) : m_bDelayOnly(FALSE), m_dwRepeat(1) {}
	~CMouseInformation(void) {}

public:
	POINT m_stPoint;
	DWORD m_dwDelayMilliSec;
	UINT m_unMsg;
	BOOL m_bDelayOnly;
	DWORD m_dwRepeat;
};


#pragma once

#include <vector>

class CMouseInformation
{
public:
	CMouseInformation(void) {}
	~CMouseInformation(void) {}

public:
	POINT m_stPoint;
	DWORD m_dwDelayMilliSec;
	UINT m_unMsg;
};


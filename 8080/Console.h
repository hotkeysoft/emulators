#pragma once
#include "Common.h"

#include "windows.h"
#include "wincon.h"

class Console
{
public:
	Console();
	~Console();

	void OutChar(BYTE ch);

private:
	void BeginControlSequence();
	void EndControlSequence();
	void ProcessControlSequence();

	// Actions
	void PutChar(BYTE);
	void CLS();
	void SetColor(BYTE);
	void ScrollUp();
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void GotoXY(BYTE, BYTE);
	void HomeX();
	void HomeY();
	void HomeXY();
	void EndX();
	void EndY();
	void EndXY();
	void Insert();
	void Backspace();
	void Delete();
	void Tab();
	void InsertLine();
	
	bool m_inControlSequence;
	int m_controlSequenceLength;
	int m_currControlArg;
	BYTE m_controlArgs[3];

	COORD m_currPos;
	WORD m_currAttr;
	HANDLE m_hConsole;
};

#include "Console.h"
#include <stdio.h>
#include <fstream>
#include <conio.h>

Console::Console() : m_inControlSequence(false), m_currControlArg(0), m_currAttr(0x07)
{
	m_currPos.X = 0;
	m_currPos.Y = 0;

	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

Console::~Console()
{
}

void Console::BeginControlSequence()
{
	m_inControlSequence = true;
	m_currControlArg = 0;
	m_controlSequenceLength = 1;
}

void Console::EndControlSequence()
{
	m_inControlSequence = false;
	m_currControlArg = 0;
}

void Console::ProcessControlSequence()
{
	switch (m_controlArgs[0])
	{
	case 1: 	CLS(); 		break;
	case 2:		SetColor(m_controlArgs[1]);	break;
	case 3:		ScrollUp();	break;
	case 4:		MoveUp();	break;
	case 5:		MoveDown();	break;
	case 6:		MoveLeft();	break;
	case 7:		MoveRight();break;
	case 11:   	GotoXY(m_controlArgs[1], m_controlArgs[2]);	break;
	case 12:	HomeX();	break;
	case 13:	HomeY();	break;
	case 14:	HomeXY();	break;
	case 15:	EndX();		break;
	case 16:	EndY();		break;
	case 17:	EndXY();	break;
	case 32:	Insert();	break;
	case 33:	Backspace();break;
	case 34:	Delete();	break;

//	case 64:	immediateMode = true;	immediatePos = NULL; break;
//	case 65:	immediateMode = false;	immediatePos = fb_currpos; break;
	}

}

void Console::OutChar(BYTE ch)
{
	if (m_inControlSequence)
	{
		if (m_currControlArg == 0)
		{
			// Some commands have extra parameters
			switch (ch) 
			{
			case 2: m_controlSequenceLength = 2; break;
			case 11: m_controlSequenceLength = 3; break;
			}
		}
		m_controlArgs[m_currControlArg++] = ch;
		if (m_currControlArg >= m_controlSequenceLength)
		{
			EndControlSequence();
			ProcessControlSequence();
		}
	}
	else if (ch == 9) // tab
	{
		Tab();
	}
	else if (ch == 1)
	{
		BeginControlSequence();
	}
	else
	{
		if (ch == 13)
		{
			HomeX();
			MoveDown();
		}
		else
		{
			PutChar(ch);
		}
	}
}

void Console::PutChar(BYTE ch)
{
	bool insertLine = false;

	DWORD dummy;
	const char data = ch;
	WriteConsoleOutputCharacter(m_hConsole, &data, 1, m_currPos, &dummy);
	WriteConsoleOutputAttribute(m_hConsole, &m_currAttr, 1, m_currPos, &dummy);
	++m_currPos.X;

	if (m_currPos.X == 79)
	{
//		if (*fb_currpos != 27)
		{
//			*fb_currpos = 27;
//			++fb_currpos;

//			*fb_currpos = fb_defattr;
//			++fb_currpos;

//			insertLine = true;
		}
//		else
		{
			m_currPos.X = 0;
			m_currPos.Y += 1;
		}
	}

	if (m_currPos.X == 79 && (m_currPos.Y == 24))
	{
		ScrollUp();
		m_currPos.Y -= 1;
	}
	else if (insertLine == true)
	{
		InsertLine();
	}

	SetConsoleCursorPosition(m_hConsole, m_currPos);
}

void Console::CLS()
{
	m_currPos.X = 0;
	m_currPos.Y = 0;

	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwConSize;

	// Get the number of character cells in the current buffer. 
	if (!GetConsoleScreenBufferInfo(m_hConsole, &csbi))
	{
		return;
	}

	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	// Fill the entire screen with blanks.
	if (!FillConsoleOutputCharacter(m_hConsole,        // Handle to console screen buffer 
		(TCHAR) ' ',     // Character to write to the buffer
		dwConSize,       // Number of cells to write 
		m_currPos,     // Coordinates of first cell 
		&cCharsWritten))// Receive number of characters written
	{
		return;
	}

	// Set the buffer's attributes accordingly.
	if (!FillConsoleOutputAttribute(m_hConsole,         // Handle to console screen buffer 
		m_currAttr, // Character attributes to use
		dwConSize,        // Number of cells to set attribute 
		m_currPos,      // Coordinates of first cell 
		&cCharsWritten)) // Receive number of characters written
	{
		return;
	}

	HomeXY();
}
void Console::SetColor(BYTE color)
{
	m_currAttr = color;
}
void Console::ScrollUp()
{
	SMALL_RECT srctScrollRect;
	COORD coordDest;
	CHAR_INFO chiFill;

	srctScrollRect.Top = 1;
	srctScrollRect.Left = 0;
	srctScrollRect.Right = 79;
	srctScrollRect.Bottom = 25;

	coordDest.X = 0;
	coordDest.Y = 0;

	chiFill.Attributes = m_currAttr;
	chiFill.Char.AsciiChar = (char)' ';

	ScrollConsoleScreenBuffer(
		m_hConsole,
		&srctScrollRect, // scrolling rectangle 
		NULL,   // clipping rectangle 
		coordDest,       // top left destination cell 
		&chiFill);
}
void Console::MoveUp()
{
	if (m_currPos.Y > 0)
	{
		m_currPos.Y -= 1;
		SetConsoleCursorPosition(m_hConsole, m_currPos);
	}
}
void Console::MoveDown()
{
	if (m_currPos.Y < 25)
	{
		m_currPos.Y += 1;
		SetConsoleCursorPosition(m_hConsole, m_currPos);
	}
	else 
	{
		ScrollUp();
	}
}
void Console::MoveLeft()
{
	if (m_currPos.X == 0 && m_currPos.Y > 0)
	{
		m_currPos.X = 78;
		m_currPos.Y -= 1;
	}
	else
	{
		m_currPos.X -= 1;
	}

	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::MoveRight()
{
	m_currPos.X += 1;
	if (m_currPos.X == 79)
	{
		m_currPos.X = 0;
		m_currPos.Y += 1;
	}

	if (m_currPos.Y > 24)
	{
		ScrollUp();
		m_currPos.X = 0;
		m_currPos.Y = 24;
	}
	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::GotoXY(BYTE x, BYTE y)
{
	m_currPos.X = x;
	m_currPos.Y = y;
	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::HomeX()
{
	m_currPos.X = 0;
	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::HomeY()
{
	m_currPos.Y = 0;
	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::HomeXY()
{
	m_currPos.X = 0;
	m_currPos.Y = 0;
	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::EndX()
{
	m_currPos.X = 79;
	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::EndY()
{
	m_currPos.Y = 24;
	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::EndXY()
{
	m_currPos.X = 79;
	m_currPos.Y = 24;
	SetConsoleCursorPosition(m_hConsole, m_currPos);
}
void Console::Insert()
{
}
void Console::Backspace()
{
//	fprintf(stderr, "CON: Backspace\n");
}
void Console::Delete()
{
//	fprintf(stderr, "CON: Delete\n");
}
void Console::Tab()
{
	m_currPos.X = (m_currPos.X / 8 + 1) * 8;
	if (m_currPos.X > 79)
	{
		m_currPos.X = 0;
		m_currPos.Y += 1;
	}

	if (m_currPos.Y > 24)
	{
		ScrollUp();
		m_currPos.Y -= 1;
	}
	else 
	{
		SetConsoleCursorPosition(m_hConsole, m_currPos);
	}
}
void Console::InsertLine()
{
	fprintf(stderr, "CON: InsertLine\n");
}
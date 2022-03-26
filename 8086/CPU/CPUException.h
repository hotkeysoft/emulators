#pragma once

#include <exception>

namespace emul
{

	enum class CPUExceptionType
	{
		EX_DIVIDE = 0, // Divide error interrupt
		EX_STEP = 1, // Single step interrupt
		EX_NMI = 2, // NMI interrupt
		EX_BREAKPOINT = 3, // Breakpoint interrupt
		EX_OVERFLOW = 4, // INTO detected overflow exception
		EX_BOUND = 5, // BOUND range exceeded exception
		EX_UNDEFINED_OPCODE = 6, // Undefined opcode
		EX_NO_MATH_UNIT = 7, // No math unit available
		EX_DOUBLE_FAULT = 8, // Double Fault
		EX_MATH_PROTECTION = 9, // Math unit protection fault
		EX_NOT_PRESENT = 11, // Not Present
		EX_STACK_FAULT = 12, // Stack fault
		EX_GENERAL_PROTECTION = 13, // General Protection fault
		EX_MATH_FAULT = 16, // Processor extension error input

		EX_NONE = 0xFF
	};

	class CPUException : public std::runtime_error
	{
	public:
		CPUException(CPUExceptionType type, int errorCode = 0) :
			std::runtime_error("CPU Exception"),
			m_type(type),
			m_errorCode(errorCode)
		{
		}

		CPUExceptionType GetType() const { return m_type; }

	protected:
		CPUExceptionType m_type = CPUExceptionType::EX_NONE;
		int m_errorCode = 0;
	};
}

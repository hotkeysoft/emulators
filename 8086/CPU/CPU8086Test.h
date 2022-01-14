#pragma once
#include "CPU8086.h"

namespace emul
{
    class CPU8086Test : public CPU8086
    {
    public:
        CPU8086Test(Memory& memory, MemoryMap& mmap) : CPU8086(memory, mmap), Logger("CPUtest") {}
        void Test() { TestShiftRotate(); TestArithmetic(); }

    protected:
        void TestShiftRotate();
        void TestArithmetic();
    };
}

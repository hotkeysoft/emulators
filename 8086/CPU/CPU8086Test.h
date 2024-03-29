#pragma once
#include "CPU8086.h"

namespace emul
{
    class CPU8086Test : public CPU8086
    {
    public:
        CPU8086Test(Memory& memory) : CPU8086(memory), Logger("CPUtest") {}
        void Test() { TestRegisters(); TestShiftRotate(); TestArithmetic(); }

    protected:
        void TestRegisters();
        void TestShiftRotate();
        void TestArithmetic();
    };
}

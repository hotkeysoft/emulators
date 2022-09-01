#pragma once
#include "CPUZ80.h"

namespace emul
{
    class CPUZ80Test : public CPUZ80
    {
    public:
        CPUZ80Test(Memory& memory) : CPUZ80(memory), Logger("CPUtest") {}
        void Test() { TestDAA(); }

    protected:
        void TestDAA();

        bool TestNCHA(int num, bool in_n, bool in_c, bool in_h, BYTE in_a, 
            bool out_n, bool out_c, bool out_h, BYTE out_a, 
            std::function<void()>func);
    };
}


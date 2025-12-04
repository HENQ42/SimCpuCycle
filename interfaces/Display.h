#pragma once
#include "IMemoryDevice.h"
#include <iostream>

class Display : public IMemoryDevice
{
public:
    Word read(Address addr) const override { return 0; }

    void write(Address addr, Word value) override
    {
        // Mapeado em 0xE000
        if (addr == 0xE000)
        {
            // Imprime o caractere imediatamente
            std::cout << "[DISPLAY] Teclado: " << (char)value << std::endl
                      << std::flush;
        }
    }
};
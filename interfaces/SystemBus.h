#pragma once
#include "IMemoryDevice.h"
#include "Keyboard.h"
#include <iostream>

class SystemBus : public IMemoryDevice
{
private:
    IMemoryDevice *ram; // Pode ser a Cache ou a RAM direta
    Keyboard *keyboard;

public:
    SystemBus(IMemoryDevice *mainMemory, Keyboard *kbd)
        : ram(mainMemory), keyboard(kbd) {}

    Word read(Address addr) const override
    {
        // Mapa de Memória:
        // 0x0000 - 0xEFFF: RAM
        // 0xF000 - 0xFFFF: I/O (Teclado)

        if (addr >= 0xF000)
        {
            return keyboard->read(addr); // Redireciona para o teclado
        }
        return ram->read(addr); // Redireciona para RAM
    }

    void write(Address addr, Word value) override
    {
        if (addr >= 0xF000)
        {
            keyboard->write(addr, value);
        }
        else
        {
            ram->write(addr, value);
        }
    }
};
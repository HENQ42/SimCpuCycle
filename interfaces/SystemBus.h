#pragma once
#include "IMemoryDevice.h"
#include "Keyboard.h"
#include <iostream>
#include "Display.h"

class SystemBus : public IMemoryDevice
{
private:
    IMemoryDevice *ram; // Pode ser a Cache ou a RAM direta
    Keyboard *keyboard;
    Display *display;

public:
    SystemBus(IMemoryDevice *mainMem, Keyboard *kbd, Display *dsp)
        : ram(mainMem), keyboard(kbd), display(dsp) {}

    Word read(Address addr) const override
    {
        // Mapa de Memória:
        // 0x0000 - 0xEFFF: RAM
        // 0xF000 - 0xFFFF: I/O (Teclado)

        if (addr >= 0xF000)
            return keyboard->read(addr); // Redireciona para o teclado
        if (addr >= 0xE000)
            return display->read(addr);

        return ram->read(addr); // Redireciona para RAM
    }

    void write(Address addr, Word value) override
    {
        if (addr >= 0xF000)
        {
            keyboard->write(addr, value);
        }
        else if (addr >= 0xE000)
        {
            display->write(addr, value); // Escreve no Display
        }
        else
        {
            ram->write(addr, value);
        }
    }
};

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
        if (addr >= 0xF000)
            return keyboard->read(addr);
        if (addr >= 0xE000)
            return display->read(addr); // Display cuida de E000 e E001
        return ram->read(addr);
    }

    void write(Address addr, Word value) override
    {
        if (addr >= 0xF000)
        {
            keyboard->write(addr, value);
        }
        else if (addr >= 0xE000)
        {
            display->write(addr, value); // Display cuida de E000 e E001
        }
        else
        {
            ram->write(addr, value);
        }
    }
};

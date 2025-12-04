#pragma once
#include "IMemoryDevice.h"
#include "Colors.h"
#include <iostream>
#include <string>

class Display : public IMemoryDevice
{
private:
    std::string internalBuffer; // A memória interna do Display

public:
    Word read(Address addr) const override
    {
        // Em hardware real, ler o COMMAND register poderia retornar
        // se o display está ocupado (BUSY flag).
        return 0;
    }

    void write(Address addr, Word value) override
    {
        // --- DATA REGISTER (0xE000) ---
        // Acumula o caractere, mas não mostra nada ainda.
        if (addr == 0xE000)
        {
            internalBuffer += (char)value;
        }

        // --- COMMAND REGISTER (0xE001) ---
        // Executa uma ação baseada no número recebido.
        else if (addr == 0xE001)
        {
            switch (value)
            {
            case 1: // FLUSH (Imprimir)
                if (!internalBuffer.empty())
                {
                    std::cout << Color::CYAN << "[DISPLAY] " << internalBuffer << Color::RESET << std::endl
                              << std::flush;
                    internalBuffer.clear(); // Limpa após mostrar
                }
                break;

            case 2: // CLEAR (Limpar Buffer silenciosamente)
                internalBuffer.clear();
                break;

            case 3: // NEWLINE (Facilitador: Pula linha)
                std::cout << std::endl;
                break;
            }
        }
    }
};
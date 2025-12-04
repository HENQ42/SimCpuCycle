#pragma once
#include "IMemoryDevice.h"
#include "PIC.h"
#include <queue>
#include <iostream>

class Keyboard : public IMemoryDevice
{
private:
    PIC *pic;
    std::queue<char> keyBuffer;
    int cycleCount = 0;

public:
    Keyboard(PIC *interruptController) : pic(interruptController) {}

    // Simula o tempo passando. A cada 20 ticks, "alguém" digita uma tecla.
    void tick()
    {
        cycleCount++;
        if (cycleCount % 20 == 0)
        { // A cada 20 ciclos simulados
            // Simulando entrada: Letra 'A' (65)
            keyBuffer.push('A');
            std::cout << "[KEYBOARD] Tecla 'A' pressionada. Gerando IRQ 1..." << std::endl;
            pic->requestIRQ(1); // Dispara interrupção 1
        }
    }

    // MMIO: A CPU lê daqui como se fosse memória RAM
    // Endereço base simulado: 0xF000 (DATA), 0xF001 (STATUS)
    Word read(Address addr) const override
    {
        // Como o método é const, precisamos de um hack ou mutable para alterar a fila.
        // Para simplificar didaticamente, vamos assumir acesso direto:
        Keyboard *self = const_cast<Keyboard *>(this);

        if (addr == 0xF000)
        { // Registrador de DADOS
            if (!self->keyBuffer.empty())
            {
                char c = self->keyBuffer.front();
                self->keyBuffer.pop();
                return (Word)c;
            }
            return 0;
        }
        else if (addr == 0xF001)
        {                                           // Registrador de STATUS
            return self->keyBuffer.empty() ? 0 : 1; // 1 = Tem dados
        }
        return 0;
    }

    void write(Address addr, Word value) override
    {
        // Teclados geralmente são Read-Only para dados,
        // mas poderíamos escrever comandos de controle (ex: acender LEDs)
    }
};
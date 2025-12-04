#pragma once
#include <cstdint>

class PIC
{
private:
    bool interruptPending = false;
    uint8_t irqVector = 0; // O "ID" da interrupção (ex: 1 = Teclado)

public:
    // Chamado por um dispositivo (Teclado) para avisar "Tenho dados!"
    void requestIRQ(uint8_t vector)
    {
        interruptPending = true;
        irqVector = vector;
    }

    // Chamado pela CPU para verificar se precisa parar
    bool isPending() const
    {
        return interruptPending;
    }

    // Chamado pela CPU para dizer "Entendi, vou tratar agora"
    uint8_t ackIRQ()
    {
        interruptPending = false;
        return irqVector;
    }
};
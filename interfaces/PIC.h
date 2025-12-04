#pragma once
#include <cstdint>
#include "Stats.h" // Incluir

class PIC
{
private:
    bool interruptPending = false;
    uint8_t irqVector = 0;
    Stats *stats; // Referência às estatísticas

public:
    // Recebe stats no construtor
    PIC(Stats *s = nullptr) : stats(s) {}

    void requestIRQ(uint8_t vector, unsigned long long currentCycle)
    {
        interruptPending = true;
        irqVector = vector;

        // Registra o momento exato do pedido
        if (stats)
            stats->irqRequestTimestamp = currentCycle;
    }

    bool isPending() const { return interruptPending; }

    uint8_t ackIRQ()
    {
        interruptPending = false;
        return irqVector;
    }
};
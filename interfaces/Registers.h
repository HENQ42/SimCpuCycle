#pragma once
#include "Types.h"
#include <iostream>
#include <iomanip> // Para formatar o debug

class Registers
{
private:
    // --- Registradores de Controle ---
    Address pc; // Program Counter: Endereço da próxima instrução
    Word ir;    // Instruction Register: A instrução atual sendo executada

    // --- Registradores de Dados ---
    int32_t acc; // Accumulator: Onde ocorrem as operações matemáticas

    // --- Flags de Estado (Status Register) ---
    bool zeroFlag;     // Z: True se o último resultado foi 0
    bool negativeFlag; // N: True se o último resultado foi negativo

public:
    Registers()
    {
        reset();
    }

    // Reinicia o estado da CPU (Power on / Reset)
    void reset()
    {
        pc = 0;
        ir = 0;
        acc = 0;
        zeroFlag = false;
        negativeFlag = false;
    }

    // --- Manipulação do PC ---
    Address getPC() const { return pc; }

    // Usado para JUMP (Desvios)
    void setPC(Address addr) { pc = addr; }

    // Avança para a próxima instrução (Fetch cycle)
    // Nota: Incrementa em 1 pois sua memória é indexada por Words, não Bytes.
    void incrementPC() { pc++; }

    // --- Manipulação do IR ---
    Word getIR() const { return ir; }

    // Apenas a CPU deve chamar isso durante o Fetch
    void setIR(Word instruction) { ir = instruction; }

    // --- Manipulação do ACC ---
    int32_t getACC() const { return acc; }

    // Atualiza o ACC e automaticamente recalcula as flags
    // Isso garante consistência: nunca teremos ACC=0 com zeroFlag=false
    void setACC(int32_t value)
    {
        acc = value;
        updateFlags(acc);
    }

    // --- Acesso às Flags (Leitura apenas) ---
    // Usado pela Unidade de Controle para instruções condicionais (JUMP IF ZERO)
    bool isZero() const { return zeroFlag; }
    bool isNegative() const { return negativeFlag; }

    // --- Debug ---
    // Essencial para ver o que está acontecendo dentro da simulação
    void dump() const
    {
        std::cout << "--- Registers ---" << std::endl;
        std::cout << "PC:  " << std::dec << pc << std::endl;
        std::cout << "IR:  0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << ir << std::endl;
        std::cout << "ACC: " << std::dec << acc << " (0x" << std::hex << acc << ")" << std::endl;
        std::cout << "Flags: [Z: " << (zeroFlag ? "1" : "0") << "] [N: " << (negativeFlag ? "1" : "0") << "]" << std::endl;
        std::cout << "-----------------" << std::endl;
    }

private:
    // Helper privado para atualizar flags
    void updateFlags(int32_t value)
    {
        zeroFlag = (value == 0);
        negativeFlag = (value < 0);
    }
};
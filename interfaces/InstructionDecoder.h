#pragma once
#include "Types.h"

// Estrutura de Transferência de Dados (DTO)
struct DecodedInstruction
{
    Opcode opcode;
    bool isAddressMode; // true = Modo 1 (Endereço), false = Modo 0 (Imediato)
    uint32_t operand;   // 23 bits
};

class InstructionDecoder
{
public:
    // Método estático pois não guarda estado, é apenas uma função utilitária pura
    static DecodedInstruction decode(Word rawInstruction)
    {
        DecodedInstruction inst;

        // 1. Extrair Opcode (Bits 24 a 31)
        // Deslocamos 24 bits para a direita e pegamos os 8 bits
        inst.opcode = static_cast<Opcode>((rawInstruction >> 24) & 0xFF);

        // 2. Extrair Modo (Bit 23)
        // Deslocamos 23 bits e pegamos 1 bit
        inst.isAddressMode = (rawInstruction >> 23) & 0x01;

        // 3. Extrair Operando (Bits 0 a 22)
        // Máscara para pegar os primeiros 23 bits: 0x7FFFFF
        // (Binário: 0000 0000 0111 1111 1111 1111 1111 1111)
        inst.operand = rawInstruction & 0x7FFFFF;

        return inst;
    }
};
#pragma once
#include <cstdint>

// Mantendo exatamente a mesma estrutura definida anteriormente
using Address = uint32_t;
using Word = uint32_t;  // 32 bits conforme especificado
using Opcode = uint8_t; // 8 bits

// Enumeração expandida para suportar as novas operações
enum class InstructionType : uint8_t
{
    HALT = 0x00,
    LOAD = 0x01,  // Carrega da Memória para ACC
    STORE = 0x02, // Salva do ACC para Memória
    ADD = 0x03,   // Soma (Serve para ADD e ADDI dependendo do Modo)
    SUB = 0x04,   // Subtração
    AND = 0x05,   // Bitwise AND
    XOR = 0x06,   // Bitwise XOR
    SLT = 0x07,   // Set Less Than (Se ACC < Operando, ACC = 1, senão 0)
    JUMP = 0x08,  // Salto incondicional (necessário para loops)
    JEQ = 0x09    // Jump Equal (Salto se ZeroFlag for true)
};
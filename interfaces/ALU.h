#pragma once
#include "Types.h"
#include <iostream>

class ALU
{
public:
    /**
     * Executa a operação lógica ou aritmética.
     * @param op O código da operação (Opcode)
     * @param accVal O valor atual do Acumulador (Lado Esquerdo)
     * @param operandVal O valor do operando (Lado Direito - vindo da memória ou imediato)
     * @return O resultado da operação para ser armazenado de volta no ACC
     */
    int32_t execute(Opcode op, int32_t accVal, int32_t operandVal)
    {
        // Convertemos o uint8_t op para o enum para facilitar o switch
        InstructionType type = static_cast<InstructionType>(op);

        switch (type)
        {
        case InstructionType::ADD:
            // Funciona para ADD (Memória) e ADDI (Imediato)
            return accVal + operandVal;

        case InstructionType::SUB:
            return accVal - operandVal;

        case InstructionType::AND:
            return accVal & operandVal;

        case InstructionType::XOR:
            return accVal ^ operandVal;

        case InstructionType::SLT:
            // Set Less Than: Se acc < operand, retorna 1, senão 0
            return (accVal < operandVal) ? 1 : 0;

        case InstructionType::LOAD:
            // No caso de LOAD, a ULA apenas passa o valor do operando direto para o ACC
            // "ACC = 0 + Operando" ou apenas "ACC = Operando"
            return operandVal;

        case InstructionType::HALT:
        case InstructionType::STORE:
        case InstructionType::JUMP:
        case InstructionType::JEQ:
            // Estas instruções não usam a ULA para alterar o ACC matematicamente.
            // Retornamos o próprio accVal para não alterar nada por acidente.
            return accVal;

        default:
            std::cerr << "[ALU] Opcode desconhecido ou não aritmético: " << (int)op << std::endl;
            return accVal;
        }
    }
};
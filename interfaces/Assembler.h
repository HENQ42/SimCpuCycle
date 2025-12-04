#pragma once
#include "Types.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <iostream>

class Assembler
{
private:
    std::unordered_map<std::string, Opcode> opcodes;

public:
    Assembler()
    {
        // Inicializa o dicionário de instruções
        opcodes["HALT"] = (Opcode)InstructionType::HALT;
        opcodes["LOAD"] = (Opcode)InstructionType::LOAD;
        opcodes["STORE"] = (Opcode)InstructionType::STORE;
        opcodes["ADD"] = (Opcode)InstructionType::ADD;
        opcodes["SUB"] = (Opcode)InstructionType::SUB;
        opcodes["AND"] = (Opcode)InstructionType::AND;
        opcodes["XOR"] = (Opcode)InstructionType::XOR;
        opcodes["SLT"] = (Opcode)InstructionType::SLT;
        opcodes["JUMP"] = (Opcode)InstructionType::JUMP;
        opcodes["JEQ"] = (Opcode)InstructionType::JEQ;
    }

    /**
     * Remove comentários e espaços em branco desnecessários.
     */
    std::string cleanLine(std::string line)
    {
        // 1. Remove comentários (tudo depois de ';')
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        // 2. Remove espaços em branco (trim)
        // Adicionei \r para segurança em arquivos Windows/DOS
        const std::string whitespace = " \t\r";
        size_t start = line.find_first_not_of(whitespace);

        if (start == std::string::npos)
            return ""; // Linha vazia ou só espaços

        size_t end = line.find_last_not_of(whitespace);
        return line.substr(start, end - start + 1);
    }

    /**
     * Traduz uma linha de texto assembly JÁ LIMPA para código de máquina.
     */
    Word assembleLine(std::string line)
    {
        std::stringstream ss(line);
        std::string mnemonic;
        std::string operandStr;

        // 1. Ler o Mnemônico
        ss >> mnemonic;

        // Converter para maiúsculo
        std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);

        // Verificar se existe no mapa
        if (opcodes.find(mnemonic) == opcodes.end())
        {
            std::cerr << "[Assembler Error] Instrução desconhecida: " << mnemonic << std::endl;
            return 0;
        }

        Opcode op = opcodes[mnemonic];

        // Se for HALT, retorna logo (bits superiores preenchidos, resto zero)
        if (op == (Opcode)InstructionType::HALT)
        {
            return (Word)op << 24;
        }

        // 2. Ler o Operando
        ss >> operandStr;

        bool isImmediate = false;
        uint32_t operandValue = 0;

        if (!operandStr.empty())
        {
            // Verifica se começa com '#'
            if (operandStr[0] == '#')
            {
                isImmediate = true;                // Modo 0
                operandStr = operandStr.substr(1); // Remove o '#'
            }
            else
            {
                isImmediate = false; // Modo 1 (Endereço)
            }

            try
            {
                operandValue = std::stoi(operandStr);
            }
            catch (...)
            {
                std::cerr << "[Assembler Error] Operando inválido: " << operandStr << std::endl;
            }
        }

        // 3. Montar a Instrução Binária
        Word machineCode = 0;
        machineCode |= ((Word)op << 24); // Opcode (bits 31-24)

        if (!isImmediate)
        {
            machineCode |= (1 << 23); // Modo (bit 23): 1=Addr, 0=Imm
        }

        machineCode |= (operandValue & 0x7FFFFF); // Operando (bits 22-0)

        return machineCode;
    }
};
#pragma once
#include "Types.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <vector>

class Assembler
{
private:
    std::unordered_map<std::string, Opcode> opcodes;
    std::unordered_map<std::string, Address> symbolTable; // Guarda: "LOOP" -> 2

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

    // Método Utilitário de Limpeza
    std::string cleanLine(std::string line)
    {
        // 1. Remove comentários
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }
        // 2. Trim (remove espaços do inicio e fim)
        const std::string whitespace = " \t\r\n";
        size_t start = line.find_first_not_of(whitespace);
        if (start == std::string::npos)
            return ""; // Linha vazia
        size_t end = line.find_last_not_of(whitespace);
        return line.substr(start, end - start + 1);
    }

    /**
     * O GRANDE MÉTODO: Recebe o código fonte completo e retorna o binário.
     * Realiza dois passos: 1. Mapear Labels, 2. Gerar Código.
     */
    std::vector<Word> assembleProgram(const std::vector<std::string> &sourceLines)
    {
        symbolTable.clear();
        std::vector<Word> binaryProgram;
        std::vector<std::string> cleanLines;

        // --- Pré-processamento: Limpar linhas e remover vazias ---
        for (const auto &raw : sourceLines)
        {
            std::string clean = cleanLine(raw);
            if (!clean.empty())
            {
                cleanLines.push_back(clean);
            }
        }

        // --- PASSO 1: Mapear Labels ---
        Address currentAddress = 0;
        for (const auto &line : cleanLines)
        {
            // Verifica se a linha é uma declaração de Label (ex: "LOOP:")
            if (line.back() == ':')
            {
                std::string label = line.substr(0, line.size() - 1); // Remove ':'
                symbolTable[label] = currentAddress;
                // Labels não ocupam espaço na memória, são apenas marcadores
            }
            else
            {
                // É uma instrução, ocupa 1 espaço na memória
                currentAddress++;
            }
        }

        // --- PASSO 2: Gerar Código ---
        currentAddress = 0;
        for (const auto &line : cleanLines)
        {
            if (line.back() == ':')
                continue; // Pula definições de label

            binaryProgram.push_back(assembleLineWithSymbols(line));
            currentAddress++;
        }

        return binaryProgram;
    }

private:
    // Versão interna que consulta a tabela de símbolos
    Word assembleLineWithSymbols(std::string line)
    {
        std::stringstream ss(line);
        std::string mnemonic;
        std::string operandStr;

        ss >> mnemonic;
        std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);

        if (opcodes.find(mnemonic) == opcodes.end())
        {
            std::cerr << "[Assembler Error] Instrução desconhecida: " << mnemonic << std::endl;
            return 0;
        }

        Opcode op = opcodes[mnemonic];

        // Se for HALT, retorna logo
        if (op == (Opcode)InstructionType::HALT)
            return (Word)op << 24;

        ss >> operandStr;

        bool isImmediate = false;
        uint32_t operandValue = 0;

        if (!operandStr.empty())
        {
            if (operandStr[0] == '#')
            {
                // Imediato Literal (#10)
                isImmediate = true;
                try
                {
                    operandValue = std::stoi(operandStr.substr(1));
                }
                catch (...)
                {
                }
            }
            else if (isdigit(operandStr[0]))
            {
                // Endereço Numérico Direto (100)
                isImmediate = false;
                try
                {
                    operandValue = std::stoi(operandStr);
                }
                catch (...)
                {
                }
            }
            else
            {
                // LABEL (LOOP)
                // Aqui está a mágica: trocamos o texto pelo endereço gravado no Passo 1
                isImmediate = false; // Labels são sempre endereços (Modo 1)

                if (symbolTable.find(operandStr) != symbolTable.end())
                {
                    operandValue = symbolTable[operandStr];
                }
                else
                {
                    std::cerr << "[Assembler Error] Label nao encontrado: " << operandStr << std::endl;
                }
            }
        }

        Word machineCode = 0;
        machineCode |= ((Word)op << 24);
        if (!isImmediate)
            machineCode |= (1 << 23); // Modo Endereço (Bit 23 = 1)
        machineCode |= (operandValue & 0x7FFFFF);

        return machineCode;
    }
};
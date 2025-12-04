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
    std::unordered_map<std::string, Address> symbolTable;

public:
    Assembler()
    {
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
        opcodes["PUSH"] = (Opcode)InstructionType::PUSH;
        opcodes["POP"] = (Opcode)InstructionType::POP;
        opcodes["CALL"] = (Opcode)InstructionType::CALL;
        opcodes["RET"] = (Opcode)InstructionType::RET;
    }

    std::string cleanLine(std::string line)
    {
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);
        const std::string whitespace = " \t\r\n";
        size_t start = line.find_first_not_of(whitespace);
        if (start == std::string::npos)
            return "";
        size_t end = line.find_last_not_of(whitespace);
        return line.substr(start, end - start + 1);
    }

    std::vector<Word> assembleProgram(const std::vector<std::string> &sourceLines)
    {
        symbolTable.clear();
        std::vector<Word> binaryProgram;
        std::vector<std::string> cleanLines;

        // Pré-processamento
        for (const auto &raw : sourceLines)
        {
            std::string clean = cleanLine(raw);
            if (!clean.empty())
                cleanLines.push_back(clean);
        }

        // --- PASSO 1: Mapear Labels (Considerando ORG) ---
        Address currentAddress = 0;
        for (const auto &line : cleanLines)
        {
            // Diretiva ORG (Muda o cursor de endereço atual)
            if (line.substr(0, 3) == "ORG")
            {
                try
                {
                    currentAddress = std::stoi(line.substr(4));
                }
                catch (...)
                {
                    std::cerr << "Erro ORG invalido\n";
                }
                continue;
            }

            if (line.back() == ':')
            {
                std::string label = line.substr(0, line.size() - 1);
                symbolTable[label] = currentAddress;
            }
            else
            {
                currentAddress++;
            }
        }

        // --- PASSO 2: Gerar Binário ---
        binaryProgram.clear(); // Limpa para garantir
        for (const auto &line : cleanLines)
        {
            // Tratamento de ORG no Passo 2
            if (line.substr(0, 3) == "ORG")
            {
                Address targetAddr = 0;
                try
                {
                    targetAddr = std::stoi(line.substr(4));
                }
                catch (...)
                {
                }

                // Preenche o "buraco" com zeros até chegar no endereço desejado
                while (binaryProgram.size() < targetAddr)
                {
                    binaryProgram.push_back(0); // 0 = HALT/NOP
                }
                continue;
            }

            if (line.back() == ':')
                continue;

            binaryProgram.push_back(assembleLine(line));
        }

        return binaryProgram;
    }

    Word assembleLine(std::string line)
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
        if (op == (Opcode)InstructionType::HALT)
            return (Word)op << 24;

        ss >> operandStr;
        bool isImmediate = false;
        uint32_t operandValue = 0;

        if (!operandStr.empty())
        {
            if (operandStr[0] == '#')
            {
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
                isImmediate = false;
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
            machineCode |= (1 << 23);
        machineCode |= (operandValue & 0x7FFFFF);

        return machineCode;
    }
};
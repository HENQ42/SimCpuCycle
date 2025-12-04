#pragma once
#include "Types.h"
#include "IMemoryDevice.h"
#include "Registers.h"
#include "ALU.h"
#include "InstructionDecoder.h"
#include <iostream>

class CPU
{
private:
    // --- Componentes da Arquitetura ---
    Registers registers; // O estado interno (PC, ACC, IR, Flags)
    ALU alu;             // O cérebro matemático
    IMemoryDevice *bus;  // O barramento para o mundo externo (Memória)

    bool halted; // Flag para parar a execução

public:
    // Injeção de Dependência: A CPU recebe o barramento, não cria a memória.
    CPU(IMemoryDevice *memoryBus) : bus(memoryBus), halted(false)
    {
        registers.reset();
    }

    // --- Ciclo Principal (Fetch-Decode-Execute) ---
    void step()
    {
        if (halted)
            return;

        // 1. FETCH (Busca)
        fetch();

        // 2. DECODE (Decodificação)
        DecodedInstruction decoded = decode();

        // 3. EXECUTE (Execução)
        execute(decoded);
    }

    // Método para executar todo o programa até encontrar HALT
    void run()
    {
        while (!halted)
        {
            step();
        }
        std::cout << "CPU Halted." << std::endl;
    }

    // Acesso aos registradores para Debug/Testes
    const Registers &getRegisters() const { return registers; }

private:
    // Fase 1: Busca a instrução na memória usando o PC
    void fetch()
    {
        Address currentPC = registers.getPC();
        Word instructionRaw = bus->read(currentPC);

        registers.setIR(instructionRaw); // Guarda no IR
        registers.incrementPC();         // Prepara para a próxima
    }

    // Fase 2: Usa o Decodificador estático
    DecodedInstruction decode()
    {
        return InstructionDecoder::decode(registers.getIR());
    }

    // Fase 3: Roteia a execução e resolve o Modo de Endereçamento
    void execute(const DecodedInstruction &instr)
    {
        InstructionType type = static_cast<InstructionType>(instr.opcode);

        // Tratamento especial para HALT antes de qualquer lógica
        if (type == InstructionType::HALT)
        {
            halted = true;
            return;
        }

        // --- Resolução de Operando (Lógica do Modo) ---
        // A maioria das instruções precisa de um valor ("B").
        // Vamos descobrir qual é esse valor antes de chamar a ULA.
        int32_t operandValue = 0;

        // Nota: STORE e JUMP usam o operando como endereço de destino,
        // então não buscamos o valor na memória para eles aqui.
        bool isStoreOrJump = (type == InstructionType::STORE ||
                              type == InstructionType::JUMP ||
                              type == InstructionType::JEQ);

        if (!isStoreOrJump)
        {
            if (instr.isAddressMode)
            {
                // MODO 1: O operando é um endereço. Buscamos o valor na RAM.
                operandValue = bus->read(instr.operand);
            }
            else
            {
                // MODO 0: O operando é o valor imediato (Literal).
                operandValue = instr.operand;
            }
        }

        // --- Execução Específica ---
        switch (type)
        {
        // Operações que usam a ULA e salvam no ACC
        case InstructionType::ADD:
        case InstructionType::SUB:
        case InstructionType::AND:
        case InstructionType::XOR:
        case InstructionType::SLT:
        case InstructionType::LOAD:
        {
            // Pega ACC atual
            int32_t currentAcc = registers.getACC();
            // Pede pra ULA calcular
            int32_t result = alu.execute(instr.opcode, currentAcc, operandValue);
            // Salva resultado e atualiza Flags
            registers.setACC(result);
        }
        break;

        // Operação de Memória (Escrita)
        case InstructionType::STORE:
            // STORE pega o valor do ACC e escreve no endereço indicado pelo operando
            bus->write(instr.operand, registers.getACC());
            break;

        // Operações de Controle de Fluxo (Desvios)
        case InstructionType::JUMP:
            registers.setPC(instr.operand);
            break;

        case InstructionType::JEQ: // Jump if Equal (Zero)
            if (registers.isZero())
            {
                registers.setPC(instr.operand);
            }
            break;

        default:
            std::cerr << "Opcode desconhecido: " << (int)instr.opcode << std::endl;
            break;
        }
    }
};
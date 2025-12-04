#pragma once
#include "Types.h"
#include "IMemoryDevice.h"
#include "Registers.h"
#include "ALU.h"
#include "InstructionDecoder.h"
#include "PIC.h"
#include <iostream>

class CPU
{
private:
    Registers registers;
    ALU alu;
    IMemoryDevice *bus;
    PIC *pic;

    bool interruptsEnabled;
    bool halted;

public:
    // Construtor com Injeção de Dependência
    CPU(IMemoryDevice *memoryBus, PIC *interruptController = nullptr)
        : bus(memoryBus), pic(interruptController), halted(false)
    {
        registers.reset();
        interruptsEnabled = true; // Começa ouvindo
    }

    // --- Métodos Auxiliares de Pilha ---
    void push(Word value)
    {
        // 1. Escreve no endereço atual do SP
        bus->write(registers.getSP(), value);
        // 2. Decrementa o SP (Pilha cresce para baixo: 1023 -> 1022)
        registers.decSP();
    }

    Word pop()
    {
        // 1. Incrementa o SP (Volta para o último dado válido)
        registers.incSP();
        // 2. Lê o valor
        return bus->read(registers.getSP());
    }

    // --- Ciclo Principal ---
    void step()
    {
        if (halted)
            return;

        // 1. CHECAGEM DE INTERRUPÇÃO
        checkInterrupts();

        // 2. FETCH (Busca)
        fetch();

        // 3. DECODE (Decodificação)
        DecodedInstruction decoded = decode();

        // 4. EXECUTE (Execução)
        execute(decoded);
    }

    void run()
    {
        while (!halted)
        {
            step();
        }
    }

    bool isHalted() const { return halted; }

    const Registers &getRegisters() const { return registers; }

private:
    // --- Tratamento de Interrupções ---
    void checkInterrupts()
    {
        if (!interruptsEnabled || pic == nullptr || !pic->isPending())
            return;

        // A CPU aceita a interrupção
        uint8_t vector = pic->ackIRQ();

        // 1. DESATIVA NOVAS INTERRUPÇÕES (Modo "Não Perturbe")
        interruptsEnabled = false;

        std::cout << "[CPU] INTERRUPT DETECTED! Vector: " << (int)vector << std::endl;

        // --- CONTEXT SWITCH (Usando a Pilha) ---
        // Agora salvamos o PC na pilha. Isso permite interrupções aninhadas.
        push(registers.getPC());

        // Desvio baseado no vetor (Tabela de Vetores Simplificada)
        if (vector == 1)
        {
            registers.setPC(500); // Endereço do Driver de Teclado
            std::cout << "[CPU] PUSH PC na Pilha. Saltando para ISR (500)." << std::endl;
        }
    }

    // --- Estágios do Pipeline ---
    void fetch()
    {
        Address currentPC = registers.getPC();
        Word instructionRaw = bus->read(currentPC);
        registers.setIR(instructionRaw);
        registers.incrementPC();
    }

    DecodedInstruction decode()
    {
        return InstructionDecoder::decode(registers.getIR());
    }

    void execute(const DecodedInstruction &instr)
    {
        InstructionType type = static_cast<InstructionType>(instr.opcode);

        if (type == InstructionType::HALT)
        {
            halted = true;
            return;
        }

        // --- Pré-Busca de Operando ---
        int32_t operandValue = 0;

        // Instruções que usam o operando como ENDEREÇO de destino (não buscam valor)
        // Adicionado CALL aqui, pois ele funciona como um JUMP
        bool isJumpLike = (type == InstructionType::STORE ||
                           type == InstructionType::JUMP ||
                           type == InstructionType::JEQ ||
                           type == InstructionType::CALL);

        if (!isJumpLike)
        {
            // Para as demais (ADD, SUB, LOAD, etc.), resolvemos o valor
            if (instr.isAddressMode)
            {
                operandValue = bus->read(instr.operand);
            }
            else
            {
                operandValue = instr.operand;
            }
        }

        // --- Execução da Instrução ---
        switch (type)
        {
        // Operações Aritméticas/Lógicas (Usam a ULA)
        case InstructionType::ADD:
        case InstructionType::SUB:
        case InstructionType::AND:
        case InstructionType::XOR:
        case InstructionType::SLT:
        case InstructionType::LOAD:
        {
            int32_t result = alu.execute(instr.opcode, registers.getACC(), operandValue);
            registers.setACC(result);
        }
        break;

        // Acesso à Memória
        case InstructionType::STORE:
            bus->write(instr.operand, registers.getACC());
            break;

        // Controle de Fluxo Simples
        case InstructionType::JUMP:
            registers.setPC(instr.operand);
            break;

        case InstructionType::JEQ:
            if (registers.isZero())
                registers.setPC(instr.operand);
            break;

            // --- OPERAÇÕES DE PILHA (STACK) ---

        case InstructionType::PUSH:
            // Salva o ACC no topo da pilha
            push(registers.getACC());
            break;

        case InstructionType::POP:
            // Recupera do topo da pilha para o ACC
            registers.setACC(pop());
            break;

        case InstructionType::CALL:
            // 1. Salva o endereço de retorno (PC atual) na pilha
            push(registers.getPC());
            // 2. Pula para o endereço da função (operando)
            registers.setPC(instr.operand);
            break;

        case InstructionType::RET:
            // Recupera o endereço de retorno da pilha e joga no PC
            registers.setPC(pop());
            interruptsEnabled = true; // Reativa interrupções ao retornar
            break;

        default:
            break;
        }
    }
};
#pragma once
#include "Types.h"
#include "IMemoryDevice.h"
#include "Registers.h"
#include "ALU.h"
#include "InstructionDecoder.h"
#include "PIC.h" // <--- Importante: Incluir o PIC
#include <iostream>

class CPU
{
private:
    Registers registers;
    ALU alu;
    IMemoryDevice *bus;
    PIC *pic; // <--- Novo componente: Controlador de Interrupções

    bool halted;

public:
    // Atualizamos o construtor para receber o PIC
    CPU(IMemoryDevice *memoryBus, PIC *interruptController = nullptr)
        : bus(memoryBus), pic(interruptController), halted(false)
    {
        registers.reset();
    }

    void step()
    {
        if (halted)
            return;

        // 1. CHECAGEM DE INTERRUPÇÃO (Antes de buscar instrução)
        checkInterrupts();

        // 2. FETCH
        fetch();

        // 3. DECODE
        DecodedInstruction decoded = decode();

        // 4. EXECUTE
        execute(decoded);
    }

    void run()
    {
        while (!halted)
        {
            step();
        }
    }

    const Registers &getRegisters() const { return registers; }

private:
    // --- Lógica de Interrupção ---
    void checkInterrupts()
    {
        // Se não tiver PIC conectado ou não tiver interrupção pendente, segue o jogo.
        if (pic == nullptr || !pic->isPending())
            return;

        // A CPU aceita a interrupção
        uint8_t vector = pic->ackIRQ();
        std::cout << "[CPU] INTERRUPT DETECTED! Vector: " << (int)vector << std::endl;

        // --- CONTEXT SWITCH (Troca de Contexto) ---
        // Em hardware real, salvamos o PC atual na Pilha (Stack).
        // Aqui, por simplicidade, vamos salvar num endereço fixo reservado (0x03FF)
        // para sabermos onde voltar depois.
        Address returnAddress = registers.getPC();
        bus->write(0x03FF, returnAddress);

        // O Vetor de Interrupção define para onde pulamos.
        // Vamos convencionar:
        // IRQ 1 (Teclado) -> Pula para endereço 500 (Endereço do Driver de Teclado)
        if (vector == 1)
        {
            registers.setPC(500);
            std::cout << "[CPU] Saltando para ISR no endereco 500 (PC salvo: " << returnAddress << ")" << std::endl;
        }
    }

    // --- Métodos Originais (Fetch/Decode/Execute) ---
    // (Mantenha o fetch, decode e execute exatamente como estavam antes)

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

        int32_t operandValue = 0;
        bool isStoreOrJump = (type == InstructionType::STORE ||
                              type == InstructionType::JUMP ||
                              type == InstructionType::JEQ);

        if (!isStoreOrJump)
        {
            if (instr.isAddressMode)
            {
                operandValue = bus->read(instr.operand);
            }
            else
            {
                operandValue = instr.operand;
            }
        }

        switch (type)
        {
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
        case InstructionType::STORE:
            bus->write(instr.operand, registers.getACC());
            break;
        case InstructionType::JUMP:
            registers.setPC(instr.operand);
            break;
        case InstructionType::JEQ:
            if (registers.isZero())
                registers.setPC(instr.operand);
            break;
        default:
            break;
        }
    }
};
#pragma once
#include "Types.h"
#include "IMemoryDevice.h"
#include "Registers.h"
#include "ALU.h"
#include "InstructionDecoder.h"
#include "PIC.h"
#include "Stats.h"  // Necessário para métricas
#include "Colors.h" // Necessário para logs coloridos
#include <iostream>

class CPU
{
private:
    Registers registers;
    ALU alu;
    IMemoryDevice *bus;
    PIC *pic;
    Stats *stats; // Ponteiro para o coletor de estatísticas

    bool interruptsEnabled;
    bool halted;

public:
    // Construtor Atualizado: Recebe Stats*
    CPU(IMemoryDevice *memoryBus, PIC *interruptController, Stats *systemStats)
        : bus(memoryBus), pic(interruptController), stats(systemStats), halted(false)
    {
        registers.reset();
        interruptsEnabled = true; // Começa ouvindo interrupções
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

        // [METRICA] Contabiliza Instrução Executada (IPC)
        if (stats)
            stats->totalInstructions++;

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
        // Se interrupções desligadas, ou sem PIC, ou sem pedido, retorna.
        if (!interruptsEnabled || pic == nullptr || !pic->isPending())
            return;

        // A CPU aceita a interrupção
        uint8_t vector = pic->ackIRQ();

        // [METRICA] Calcular Latência de Serviço da IRQ
        if (stats)
        {
            unsigned long long latency = stats->totalCycles - stats->irqRequestTimestamp;
            stats->totalIrqLatency += latency;
            stats->irqCount++;
        }

        // 1. DESATIVA NOVAS INTERRUPÇÕES (Modo "Não Perturbe")
        interruptsEnabled = false;

        // Log Colorido para destaque
        std::cout << Color::MAGENTA << Color::BOLD
                  << "[CPU] INTERRUPT DETECTED! Vector: " << (int)vector
                  << " (Interrupts Disabled)" << Color::RESET << std::endl;

        // --- CONTEXT SWITCH (Usando a Pilha) ---
        // Salva o PC na pilha para permitir retorno depois
        push(registers.getPC());

        // Desvio baseado no vetor
        if (vector == 1)
        {
            registers.setPC(500); // Endereço do Driver de Teclado
            // std::cout << "[CPU] Saltando para ISR (500)." << std::endl;
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

        // Instruções que usam o operando como ENDEREÇO de destino ou OFFSET
        // Adicionado CALL e PUSH na lista de exceções de leitura automática
        bool isJumpLike = (type == InstructionType::STORE ||
                           type == InstructionType::JUMP ||
                           type == InstructionType::JEQ ||
                           type == InstructionType::CALL ||
                           type == InstructionType::PUSH);

        if (!isJumpLike)
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

        // --- Execução da Instrução ---
        switch (type)
        {
        // Operações Aritméticas/Lógicas
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

        // Controle de Fluxo
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
            // 1. Salva PC atual
            push(registers.getPC());
            // 2. Pula
            registers.setPC(instr.operand);
            break;

        case InstructionType::RET:
            // Recupera PC
            registers.setPC(pop());
            // Reativa interrupções ao retornar da função/ISR
            interruptsEnabled = true;
            break;

        default:
            break;
        }
    }
};
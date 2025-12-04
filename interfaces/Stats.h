#pragma once
#include <iostream>
#include <iomanip>
#include "Colors.h"

struct Stats
{
    // --- Tempo ---
    unsigned long long totalCycles = 0;

    // --- CPU ---
    unsigned long long totalInstructions = 0;

    // --- Memória / Cache ---
    unsigned long long cacheHits = 0;
    unsigned long long cacheMisses = 0;
    unsigned long long busWaitCycles = 0; // Ciclos perdidos esperando RAM

    // --- IRQ ---
    unsigned long long irqRequestTimestamp = 0; // Ciclo que o IRQ chegou
    unsigned long long totalIrqLatency = 0;     // Soma das latências
    unsigned long long irqCount = 0;            // Quantas IRQs atendidas

    // --- DMA (Placeholder para futuro) ---
    unsigned long long dmaBytesCopied = 0;
    unsigned long long cpuBytesCopied = 0; // Via LOAD/STORE

    // --- Métodos de Cálculo ---

    double getIPC()
    {
        return (totalCycles == 0) ? 0.0 : (double)totalInstructions / totalCycles;
    }

    double getHitRate()
    {
        unsigned long long totalAccess = cacheHits + cacheMisses;
        return (totalAccess == 0) ? 0.0 : (double)cacheHits / totalAccess * 100.0;
    }

    double getMissRate()
    {
        unsigned long long totalAccess = cacheHits + cacheMisses;
        return (totalAccess == 0) ? 0.0 : (double)cacheMisses / totalAccess; // 0.0 a 1.0
    }

    double getMPKI()
    {
        // Misses Per Kilo-Instruction
        return (totalInstructions == 0) ? 0.0 : ((double)cacheMisses / totalInstructions) * 1000.0;
    }

    double getAMAT()
    {
        // Average Memory Access Time = Hit Time + (Miss Rate * Miss Penalty)
        // Assumindo: Hit = 1 ciclo, Miss Penalty = 10 ciclos (busca na RAM)
        const double HIT_TIME = 1.0;
        const double MISS_PENALTY = 10.0;
        return HIT_TIME + (getMissRate() * MISS_PENALTY);
    }

    void printReport()
    {
        std::cout << "\n"
                  << Color::WHITE << Color::BOLD
                  << "=== RELATÓRIO DE DESEMPENHO (PÓS-MORTEM) ===" << Color::RESET << std::endl;

        std::cout << std::fixed << std::setprecision(2);

        std::cout << Color::CYAN << "--- CPU Core ---" << Color::RESET << std::endl;
        std::cout << "Ciclos Totais:      " << totalCycles << std::endl;
        std::cout << "Instruções (Ret.):  " << totalInstructions << std::endl;
        std::cout << "IPC (Alto Nível):   " << Color::YELLOW << getIPC() << Color::RESET << " instr/ciclo" << std::endl;

        std::cout << "\n"
                  << Color::CYAN << "--- Memória & Cache ---" << Color::RESET << std::endl;
        std::cout << "Cache Hits:         " << Color::GREEN << cacheHits << Color::RESET << std::endl;
        std::cout << "Cache Misses:       " << Color::RED << cacheMisses << Color::RESET << std::endl;
        std::cout << "Taxa de Hit:        " << getHitRate() << "%" << std::endl;
        std::cout << "MPKI:               " << getMPKI() << " misses/1k instr" << std::endl;

        std::cout << "AMAT:               " << getAMAT() << " ciclos" << std::endl;
        std::cout << "Ciclos de Espera:   " << busWaitCycles << " (Stall por memória)" << std::endl;

        std::cout << "\n"
                  << Color::CYAN << "--- Interrupções (IRQ) ---" << Color::RESET << std::endl;
        std::cout << "IRQs Atendidas:     " << irqCount << std::endl;
        if (irqCount > 0)
        {
            std::cout << "Latência Média:     " << (double)totalIrqLatency / irqCount << " ciclos" << std::endl;
        }

        std::cout << "\n"
                  << Color::CYAN << "--- Transferência de Dados ---" << Color::RESET << std::endl;
        std::cout << "Cópia via CPU:      " << cpuBytesCopied << " bytes (Load/Store)" << std::endl;
        std::cout << "Cópia via DMA:      " << dmaBytesCopied << " bytes (N/A)" << std::endl;

        std::cout << "============================================" << std::endl;
    }
};
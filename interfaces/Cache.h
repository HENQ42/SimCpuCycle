#pragma once
#include "IMemoryDevice.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include "Colors.h"
#include "Stats.h" // Necessário para contabilizar métricas

struct CacheLine
{
    bool valid = false;
    uint32_t tag = 0;
    std::vector<Word> dataBlock; // O Bloco de dados (ex: 4 palavras)
};

class Cache : public IMemoryDevice
{
private:
    IMemoryDevice *ramReal;
    std::vector<CacheLine> lines;
    Stats *stats; // Ponteiro para o coletor de métricas

    size_t numLines;  // Quantas linhas a cache tem (ex: 8)
    size_t blockSize; // Quantas palavras cabem numa linha (ex: 4)
    bool verbose;     // Controla se deve imprimir logs

public:
    // Atualizado construtor para receber Stats* e agora o booleano verbose
    Cache(IMemoryDevice *ram, Stats *s, size_t linesCount = 8, size_t wordsPerLine = 4, bool verboseMode = true)
        : ramReal(ram), stats(s), numLines(linesCount), blockSize(wordsPerLine), verbose(verboseMode)
    {
        // Inicializa as linhas com vetores vazios do tamanho correto
        lines.resize(numLines);
        for (auto &line : lines)
        {
            line.dataBlock.resize(blockSize, 0);
        }
    }

    Word read(Address addr) const override
    {
        // --- MATEMÁTICA DE ENDEREÇAMENTO ---
        // 1. Em qual bloco da memória universal este endereço está?
        uint32_t blockAddr = addr / blockSize;

        // 2. Qual é o deslocamento (offset) dentro desse bloco? (0 a 3)
        uint32_t offset = addr % blockSize;

        // 3. Mapeamento Direto: Qual linha da cache cuida desse bloco?
        uint32_t index = blockAddr % numLines;

        // 4. Tag: Identificador único do bloco
        uint32_t tag = blockAddr / numLines;

        // Acesso à linha (const_cast para permitir update em read)
        CacheLine &line = const_cast<Cache *>(this)->lines[index];

        // --- VERIFICAÇÃO (HIT/MISS) ---
        if (line.valid && line.tag == tag)
        {
            // [METRICA] Hit
            if (stats)
                stats->cacheHits++;

            // [HIT] O bloco inteiro já está aqui!
            if (verbose)
            {
                std::cout << Color::GREEN << "[CACHE HIT]  Addr: " << addr << Color::RESET << std::endl;
            }
            return line.dataBlock[offset];
        }
        else
        {
            // [METRICA] Miss
            if (stats)
            {
                stats->cacheMisses++;
                stats->busWaitCycles += 10; // Penalidade simulada de latência da RAM
            }

            // [MISS] Precisamos buscar o BLOCO INTEIRO na RAM
            if (verbose)
            {
                std::cout << Color::RED << "[CACHE MISS] Addr: " << addr
                          << " -> Buscando Bloco [" << (blockAddr * blockSize)
                          << " a " << ((blockAddr * blockSize) + blockSize - 1) << "]..." << Color::RESET << std::endl;
            }

            // Endereço base do bloco na RAM
            Address baseAddress = blockAddr * blockSize;

            // Busca sequencial (Simula o Burst Mode da RAM)
            for (size_t i = 0; i < blockSize; i++)
            {
                line.dataBlock[i] = ramReal->read(baseAddress + i);
            }

            // Atualiza Metadados
            line.valid = true;
            line.tag = tag;

            return line.dataBlock[offset];
        }
    }

    void write(Address addr, Word value) override
    {
        // Write-Through: Escreve na RAM sempre, e atualiza Cache se houver Hit

        ramReal->write(addr, value);

        uint32_t blockAddr = addr / blockSize;
        uint32_t index = (blockAddr) % numLines;
        uint32_t tag = blockAddr / numLines;
        uint32_t offset = addr % blockSize;

        if (lines[index].valid && lines[index].tag == tag)
        {
            // Se o bloco está na cache, atualizamos a palavra específica nele
            lines[index].dataBlock[offset] = value;
            if (verbose)
            {
                std::cout << "[CACHE UPDATE] Addr: " << addr << " (Write-Through)" << std::endl;
            }
        }
        else
        {
            if (verbose)
            {
                std::cout << "[CACHE BYPASS] Addr: " << addr << " (Write-Through)" << std::endl;
            }
        }
    }
};
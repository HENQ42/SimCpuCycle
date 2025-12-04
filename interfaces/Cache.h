#pragma once
#include "IMemoryDevice.h"
#include <vector>
#include <iostream>
#include <iomanip>

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

    size_t numLines;  // Quantas linhas a cache tem (ex: 8)
    size_t blockSize; // Quantas palavras cabem numa linha (ex: 4)

public:
    Cache(IMemoryDevice *ram, size_t linesCount = 8, size_t wordsPerLine = 4)
        : ramReal(ram), numLines(linesCount), blockSize(wordsPerLine)
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
            // [HIT] O bloco inteiro já está aqui!
            std::cout << "[CACHE HIT]  Addr: " << addr
                      << " (Line: " << index << " | Offset: " << offset << ")" << std::endl;
            return line.dataBlock[offset];
        }
        else
        {
            // [MISS] Precisamos buscar o BLOCO INTEIRO na RAM
            std::cout << "[CACHE MISS] Addr: " << addr
                      << " -> Buscando Bloco [" << (blockAddr * blockSize)
                      << " a " << ((blockAddr * blockSize) + blockSize - 1) << "]..." << std::endl;

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
            std::cout << "[CACHE UPDATE] Addr: " << addr << " (Write-Through)" << std::endl;
        }
        else
        {
            std::cout << "[CACHE BYPASS] Addr: " << addr << " (Write-Through)" << std::endl;
        }
    }
};
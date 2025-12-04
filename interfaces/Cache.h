#pragma once
#include "IMemoryDevice.h"
#include <vector>
#include <iostream>

struct CacheLine
{
    bool valid = false;
    uint32_t tag = 0;
    uint32_t data = 0;
};

class Cache : public IMemoryDevice
{
private:
    IMemoryDevice *ramReal; // A RAM lenta "atrás" da cache
    std::vector<CacheLine> lines;
    size_t size; // Tamanho da cache (ex: 16 posições)

public:
    // Injetamos a RAM real no construtor da Cache
    Cache(IMemoryDevice *ram, size_t cacheSize = 16) : ramReal(ram), size(cacheSize)
    {
        lines.resize(size);
    }

    Word read(Address addr) const override
    {
        // Mapeamento Direto Simples:
        // Index = Endereço % Tamanho da Cache
        // Tag = Endereço / Tamanho da Cache
        uint32_t index = addr % size;
        uint32_t tag = addr / size;

        // O const_cast é um "mal necessário" aqui pois queremos atualizar a cache
        // numa operação de leitura (que é marcada como const na interface),
        // ou removemos o const da interface. Por simplicidade didática, vamos acessar direto:
        CacheLine &line = const_cast<Cache *>(this)->lines[index];

        if (line.valid && line.tag == tag)
        {
            // --- CACHE HIT ---
            std::cout << "[CACHE HIT]  Addr: " << addr << std::endl;
            return line.data;
        }
        else
        {
            // --- CACHE MISS ---
            std::cout << "[CACHE MISS] Addr: " << addr << " -> Buscando na RAM..." << std::endl;

            // Busca na RAM lenta
            Word val = ramReal->read(addr);

            // Salva na Cache para a próxima vez
            line.valid = true;
            line.tag = tag;
            line.data = val;

            return val;
        }
    }

    void write(Address addr, Word value) override
    {
        // Política "Write-Through": Escreve na Cache E na RAM ao mesmo tempo
        // para garantir que nada se perca.

        uint32_t index = addr % size;
        uint32_t tag = addr / size;

        // Atualiza RAM
        ramReal->write(addr, value);

        // Atualiza Cache
        lines[index].valid = true;
        lines[index].tag = tag;
        lines[index].data = value;

        std::cout << "[CACHE WRITE] Addr: " << addr << std::endl;
    }
};
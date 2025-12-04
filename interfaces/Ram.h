#pragma once
#include <vector>
#include <iostream>
#include "IMemoryDevice.h"

class Ram : public IMemoryDevice
{
private:
    // Usamos vector para memória dinâmica ou array fixo.
    // O requisito dizia 1024 posições.
    std::vector<Word> dados;
    const size_t SIZE = 1024;

public:
    Ram()
    {
        dados.resize(SIZE, 0); // Inicializa tudo com 0
    }

    Word read(Address addr) const override
    {
        if (addr >= SIZE)
        {
            std::cerr << "[Erro de Barramento] Leitura fora dos limites: " << addr << std::endl;
            return 0;
        }
        return dados[addr];
    }

    void write(Address addr, Word value) override
    {
        if (addr >= SIZE)
        {
            std::cerr << "[Erro de Barramento] Escrita fora dos limites: " << addr << std::endl;
            return;
        }
        dados[addr] = value;
    }

    // Método extra apenas para debug (não faz parte da interface IMemoryDevice)
    void loadProgram(const std::vector<Word> &program)
    {
        for (size_t i = 0; i < program.size() && i < SIZE; ++i)
        {
            dados[i] = program[i];
        }
    }
};
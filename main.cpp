#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Incluindo nossas bordas e núcleo
#include "interfaces/Types.h"
#include "interfaces/Ram.h"
#include "interfaces/Assembler.h"
#include "interfaces/CPU.h"

int main()
{
    std::cout << "=== Simulador de CPU Von Neumann ===" << std::endl;

    // 1. Instanciar Componentes
    Ram ram;
    Assembler assembler;
    std::vector<Word> programBinary;

    // 2. Ler o arquivo de código fonte
    std::ifstream file("program.txt");
    if (!file.is_open())
    {
        std::cerr << "Erro: Nao foi possivel abrir 'program.txt'" << std::endl;
        return 1;
    }

    std::cout << "[Assembler] Iniciando compilacao..." << std::endl;
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line))
    {
        lineNum++;

        // Agora o Assembler é responsável pela limpeza
        std::string clean = assembler.cleanLine(line);

        if (clean.empty())
            continue; // Pula linhas vazias ou comentários puros

        // Converte texto para binário
        Word instruction = assembler.assembleLine(clean);
        programBinary.push_back(instruction);

        std::cout << "Linha " << lineNum << ": " << clean << " -> Hex: " << std::hex << instruction << std::endl;
    }
    file.close();

    std::cout << "[Loader] Carregando " << std::dec << programBinary.size() << " instrucoes na RAM." << std::endl;
    ram.loadProgram(programBinary);

    // 3. Inicializar e Rodar a CPU
    // Injetamos a dependência da RAM na CPU
    CPU cpu(&ram);

    std::cout << "\n[CPU] Iniciando execucao...\n"
              << std::endl;
    cpu.run();

    // 4. Verificação de Resultado (Pós-Mortem)
    std::cout << "\n=== Estado Final ===" << std::endl;

    cpu.getRegisters().dump();

    std::cout << "Memoria[50] (Var A): " << std::dec << ram.read(50) << std::endl;
    std::cout << "Memoria[100] (Resultado): " << std::dec << ram.read(100) << std::endl;

    std::cout << "\nSimulacao concluida." << std::endl;

    return 0;
}
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring> // Para memcpy se necessário

#include "interfaces/Types.h"
#include "interfaces/Ram.h"
#include "interfaces/Assembler.h"
#include "interfaces/CPU.h"
#include "interfaces/Cache.h"

// --- Simula o ambiente de Desenvolvimento ---
void compilarPrograma(const std::string &inputTxt, const std::string &outputBin)
{
    std::cout << "[Host PC] Iniciando Assembler..." << std::endl;

    Assembler assembler;
    std::vector<std::string> sourceCode;
    std::ifstream file(inputTxt);

    if (!file.is_open())
    {
        std::cerr << "Erro: Arquivo fonte nao encontrado." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        sourceCode.push_back(line);
    }
    file.close();

    // Gera o vetor de Words (os dados brutos)
    std::vector<Word> binary = assembler.assembleProgram(sourceCode);

    // Salva em disco como arquivo binário puro
    std::ofstream outFile(outputBin, std::ios::binary);
    if (outFile.is_open())
    {
        // Escreve o vetor inteiro de uma vez
        // reinterpret_cast converte o ponteiro de Word* para char* (bytes)
        outFile.write(reinterpret_cast<const char *>(binary.data()), binary.size() * sizeof(Word));
        outFile.close();
        std::cout << "[Host PC] Sucesso! Binario gerado: " << outputBin << " (" << binary.size() * sizeof(Word) << " bytes)" << std::endl;
    }
    else
    {
        std::cerr << "Erro ao salvar binario." << std::endl;
    }
}

// --- Simula o Hardware Real ---
void ligarMaquina(const std::string &firmwareFile)
{
    std::cout << "\n[Hardware] Ligando sistema..." << std::endl;

    // 1. Hardware Físico
    Ram ram;

    // 2. Simulação do "Bootloader" ou "Circuito de Carga"
    // O hardware lê o arquivo binário do "disco/flash" para a RAM volátil
    std::ifstream binFile(firmwareFile, std::ios::binary | std::ios::ate); // ate = at the end (para pegar tamanho)

    if (!binFile.is_open())
    {
        std::cerr << "[Hardware Error] Disco de boot não encontrado ou corrompido!" << std::endl;
        return;
    }

    std::streamsize size = binFile.tellg();
    binFile.seekg(0, std::ios::beg);

    std::vector<Word> buffer(size / sizeof(Word));
    if (binFile.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        std::cout << "[Bootloader] Copiando firmware para a RAM..." << std::endl;
        ram.loadProgram(buffer);
    }

    // 3. O "Reset" da CPU
    Cache cache(&ram, 8); // Cache pequena de 8 posições para forçar conflitos
    CPU cpu(&cache);      // A CPU nem sabe que existe uma cache!

    std::cout << "[CPU] RESET signal high. Executando..." << std::endl;
    cpu.run();

    // Pós-mortem
    std::cout << "\n=== Debug Hardware ===" << std::endl;
    cpu.getRegisters().dump();
    std::cout << "Memoria[100]: " << ram.read(100) << std::endl;
}

int main(int argc, char *argv[])
{
    // Vamos criar uma interface simples de linha de comando
    // Uso:
    // ./cpu build program_labels.txt firmware.bin
    // ./cpu run firmware.bin

    if (argc < 2)
    {
        std::cout << "Uso:\n  main build <fonte.txt> <saida.bin>\n  main run <entrada.bin>" << std::endl;

        // Para facilitar seus testes no IDE sem argumentos, vamos forçar um fluxo padrão:
        std::cout << "\n--- Modo Automatico de Teste ---" << std::endl;
        compilarPrograma("program_labels.txt", "rom.bin");
        ligarMaquina("rom.bin");
        return 0;
    }

    std::string command = argv[1];

    if (command == "build" && argc == 4)
    {
        compilarPrograma(argv[2], argv[3]);
    }
    else if (command == "run" && argc == 3)
    {
        ligarMaquina(argv[2]);
    }
    else
    {
        std::cout << "Comando invalido." << std::endl;
    }

    return 0;
}
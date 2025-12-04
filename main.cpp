#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include "interfaces/Types.h"
#include "interfaces/Ram.h"
#include "interfaces/Cache.h"
#include "interfaces/PIC.h"
#include "interfaces/Keyboard.h"
#include "interfaces/SystemBus.h"
#include "interfaces/CPU.h"
#include "interfaces/Assembler.h"
#include "interfaces/Display.h"

// --- COMPILADOR (Host) ---
void build(const std::string &inputTxt, const std::string &outputBin)
{
    std::cout << "[BUILD] Compilando " << inputTxt << " para " << outputBin << "..." << std::endl;

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

    // Gera o vetor binário (com os zeros do ORG preenchidos)
    std::vector<Word> binary = assembler.assembleProgram(sourceCode);

    std::ofstream outFile(outputBin, std::ios::binary);
    if (outFile.is_open())
    {
        outFile.write(reinterpret_cast<const char *>(binary.data()), binary.size() * sizeof(Word));
        outFile.close();
        std::cout << "[BUILD] Sucesso! Tamanho do firmware: " << binary.size() << " palavras." << std::endl;
    }
    else
    {
        std::cerr << "Erro ao salvar binario." << std::endl;
    }
}

// --- MAQUINA VIRTUAL (Target) ---
void run(const std::string &firmwareFile)
{
    std::cout << "[RUN] Iniciando Maquina..." << std::endl;

    // 1. Instancia Hardware
    Ram ram;
    Cache cache(&ram, 8);
    PIC pic;
    Keyboard keyboard(&pic);
    Display display;
    SystemBus bus(&cache, &keyboard, &display);
    CPU cpu(&bus, &pic);

    // 2. Carrega Firmware do Disco
    std::ifstream binFile(firmwareFile, std::ios::binary | std::ios::ate);
    if (!binFile.is_open())
    {
        std::cerr << "Erro: Firmware nao encontrado: " << firmwareFile << std::endl;
        return;
    }

    std::streamsize sizeBytes = binFile.tellg();
    binFile.seekg(0, std::ios::beg);

    std::vector<Word> buffer(sizeBytes / sizeof(Word));
    binFile.read(reinterpret_cast<char *>(buffer.data()), sizeBytes);

    std::cout << "[BOOT] Carregando " << buffer.size() << " instrucoes na Memória Principal." << std::endl;
    ram.loadProgram(buffer);

    // 3. Executa
    std::cout << "[SYSTEM] Power On." << std::endl;

    // Loop Infinito Interativo
    // A simulação roda até que o firmware execute HALT (acionado pelo 'z')
    while (!cpu.isHalted())
    {
        // 1. Verifica entrada real do terminal
        keyboard.tick();

        // 2. Avança a CPU
        cpu.step();

        // 3. Pequena pausa (1ms) para não usar 100% da CPU do seu computador
        usleep(700000);
    }

    std::cout << "\n[SYSTEM] Shutdown (Comando 'z' recebido ou HALT executado)." << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Uso:\n  ./cpu_sim build <fonte.txt> <saida.bin>\n  ./cpu_sim run <entrada.bin>" << std::endl;
        return 0;
    }

    std::string command = argv[1];

    if (command == "build" && argc == 4)
    {
        build(argv[2], argv[3]);
    }
    else if (command == "run" && argc == 3)
    {
        run(argv[2]);
    }
    else
    {
        std::cout << "Comando invalido." << std::endl;
    }

    return 0;
}
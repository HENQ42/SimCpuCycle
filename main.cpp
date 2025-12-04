#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h> // Para usleep

// Mantendo o padrão de pastas que você forneceu
#include "interfaces/Types.h"
#include "interfaces/Ram.h"
#include "interfaces/Cache.h"
#include "interfaces/PIC.h"
#include "interfaces/Keyboard.h"
#include "interfaces/SystemBus.h"
#include "interfaces/CPU.h"
#include "interfaces/Assembler.h"
#include "interfaces/Display.h"
#include "interfaces/Colors.h" // Arquivo de Cores
#include "interfaces/Stats.h"  // Arquivo de Estatísticas

// --- COMPILADOR (Host) ---
void build(const std::string &inputTxt, const std::string &outputBin)
{
    std::cout << Color::BLUE << Color::BOLD << "[BUILD] Compilando " << inputTxt << " para " << outputBin << "..." << Color::RESET << std::endl;

    Assembler assembler;
    std::vector<std::string> sourceCode;
    std::ifstream file(inputTxt);

    if (!file.is_open())
    {
        std::cerr << Color::RED << "Erro: Arquivo fonte nao encontrado." << Color::RESET << std::endl;
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
        std::cout << Color::GREEN << "[BUILD] Sucesso! Tamanho do firmware: " << binary.size() << " palavras." << Color::RESET << std::endl;
    }
    else
    {
        std::cerr << Color::RED << "Erro ao salvar binario." << Color::RESET << std::endl;
    }
}

// --- MAQUINA VIRTUAL (Target) ---
// Adicionado parâmetro 'quiet'
void run(const std::string &firmwareFile, bool quiet)
{
    std::cout << Color::BLUE << Color::BOLD << "[RUN] Iniciando Maquina..." << Color::RESET << std::endl;
    if (quiet)
    {
        std::cout << Color::YELLOW << "[INFO] Modo Quiet ativado (Logs de Cache ocultos)." << Color::RESET << std::endl;
    }
    std::cout << "DIGITE AGORA (" << Color::RED << "z" << Color::RESET << " para sair):" << std::endl;

    // 1. Objeto Central de Estatísticas
    Stats stats;

    // 2. Instancia Hardware (Injetando dependência de Stats)
    Ram ram;

    // Cache recebe RAM e Stats.
    // Atualizado: passamos 4 explicitamente (tamanho da linha) para poder passar !quiet no 5º argumento (verbose)
    Cache cache(&ram, &stats, 8, 4, !quiet);

    // PIC recebe Stats (para latência de IRQ)
    PIC pic(&stats);

    // Keyboard recebe PIC e ponteiro para o ciclo atual (para timestamp do IRQ)
    Keyboard keyboard(&pic, &stats.totalCycles);

    Display display;

    // Barramento conecta tudo
    SystemBus bus(&cache, &keyboard, &display);

    // CPU recebe Barramento, PIC e Stats
    CPU cpu(&bus, &pic, &stats);

    // 3. Carrega Firmware do Disco
    std::ifstream binFile(firmwareFile, std::ios::binary | std::ios::ate);
    if (!binFile.is_open())
    {
        std::cerr << Color::RED << "Erro: Firmware nao encontrado: " << firmwareFile << Color::RESET << std::endl;
        return;
    }

    std::streamsize sizeBytes = binFile.tellg();
    binFile.seekg(0, std::ios::beg);

    std::vector<Word> buffer(sizeBytes / sizeof(Word));
    binFile.read(reinterpret_cast<char *>(buffer.data()), sizeBytes);

    std::cout << Color::BLUE << "[BOOT] Carregando " << buffer.size() << " instrucoes na Memória Principal." << Color::RESET << std::endl;
    ram.loadProgram(buffer);

    // 4. Executa
    std::cout << Color::GREEN << Color::BOLD << "[SYSTEM] Power On." << Color::RESET << std::endl;

    // Loop Infinito Interativo
    // A simulação roda até que o firmware execute HALT (acionado pelo 'z')
    while (!cpu.isHalted())
    {
        // Atualiza relógio global para estatísticas
        stats.totalCycles++;

        // 1. Verifica entrada real do terminal
        keyboard.tick();

        // 2. Avança a CPU
        cpu.step();

        // 3. Pequena pausa (1ms) para não usar 100% da CPU do seu computador
        // 700000 (0.7s) é muito lento para digitação em tempo real.
        // 1000 (1ms) é fluido.
        if (!quiet)
        {
            usleep(200000);
        }
    }

    std::cout << "\n"
              << Color::RED << Color::BOLD << "[SYSTEM] Shutdown (Comando 'z' recebido ou HALT executado)." << Color::RESET << std::endl;

    // 5. Imprime Relatório Final
    stats.printReport();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Uso:\n  ./cpu_sim build <fonte.txt> <saida.bin>\n  ./cpu_sim run <entrada.bin> [-q|--quiet]" << std::endl;
        return 0;
    }

    std::string command = argv[1];

    if (command == "build" && argc == 4)
    {
        build(argv[2], argv[3]);
    }
    // Alterado para aceitar argumentos opcionais (argc >= 3)
    else if (command == "run" && argc >= 3)
    {
        std::string firmwareFile;
        bool quiet = false;

        // Parser simples de argumentos para o comando run
        for (int i = 2; i < argc; i++)
        {
            std::string arg = argv[i];
            if (arg == "-q" || arg == "--quiet")
            {
                quiet = true;
            }
            else
            {
                firmwareFile = arg;
            }
        }

        if (!firmwareFile.empty())
        {
            run(firmwareFile, quiet);
        }
        else
        {
            std::cout << "Erro: Arquivo de firmware nao especificado." << std::endl;
        }
    }
    else
    {
        std::cout << "Comando invalido ou argumentos incorretos." << std::endl;
    }

    return 0;
}
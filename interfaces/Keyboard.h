#pragma once
#include "IMemoryDevice.h"
#include "PIC.h"
#include <queue>
#include <iostream>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h> // Biblioteca para controlar o terminal

class Keyboard : public IMemoryDevice
{
private:
    PIC *pic;
    std::queue<char> internalBuffer;
    struct termios originalTermios; // Para salvar a config original

    // Ponteiro para o relógio global (para métricas de latência)
    unsigned long long *globalCycle;

public:
    // Construtor atualizado para receber o ponteiro de ciclos
    Keyboard(PIC *interruptController, unsigned long long *cyclePtr)
        : pic(interruptController), globalCycle(cyclePtr)
    {
        enableRawMode();
    }

    ~Keyboard()
    {
        disableRawMode();
    }

    // --- Configuração do Terminal (Raw Mode) ---
    void enableRawMode()
    {
        // 1. Pega a configuração atual
        tcgetattr(STDIN_FILENO, &originalTermios);

        // 2. Cria cópia
        struct termios raw = originalTermios;

        // 3. Desativa ICANON (Buffer de linha) e ECHO
        raw.c_lflag &= ~(ICANON | ECHO);

        // 4. Aplica
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    void disableRawMode()
    {
        // Restaura o terminal ao normal ao sair
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
    }

    // --- Tick do Hardware ---
    void tick()
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0; // Não bloqueante

        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

        if (ret > 0)
        {
            char buffer[1];
            // Usa ::read global para evitar conflito de nome
            int bytesRead = ::read(STDIN_FILENO, buffer, 1);

            if (bytesRead > 0)
            {
                internalBuffer.push(buffer[0]);
            }
        }

        // Se tem dados e o PIC não está ocupado, pede IRQ
        if (!internalBuffer.empty() && !pic->isPending())
        {
            // Passamos o Ciclo Atual para o PIC calcular a latência
            if (globalCycle != nullptr)
            {
                pic->requestIRQ(1, *globalCycle);
            }
            else
            {
                // Fallback caso não tenha métricas (previne crash)
                pic->requestIRQ(1, 0);
            }
        }
    }

    // --- Leitura via MMIO (0xF000) ---
    Word read(Address addr) const override
    {
        Keyboard *self = const_cast<Keyboard *>(this);

        // Se a CPU ler 0xF000, entregamos a tecla
        if (addr == 0xF000 && !self->internalBuffer.empty())
        {
            char c = self->internalBuffer.front();
            self->internalBuffer.pop();
            return (Word)c;
        }
        return 0;
    }

    void write(Address addr, Word value) override {}
};
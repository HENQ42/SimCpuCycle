#pragma once
#include "IMemoryDevice.h"
#include "PIC.h"
#include <queue>
#include <iostream>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h> // <--- Biblioteca para controlar o terminal

class Keyboard : public IMemoryDevice
{
private:
    PIC *pic;
    std::queue<char> internalBuffer;
    struct termios originalTermios; // Para salvar a config original

public:
    Keyboard(PIC *interruptController) : pic(interruptController)
    {
        enableRawMode();
    }

    ~Keyboard()
    {
        disableRawMode();
    }

    // --- Configuração do Terminal (A Mágica) ---
    void enableRawMode()
    {
        // 1. Pega a configuração atual e salva
        tcgetattr(STDIN_FILENO, &originalTermios);

        // 2. Cria uma cópia para modificar
        struct termios raw = originalTermios;

        // 3. Desativa ICANON (Buffer de linha) e ECHO (Letra repetida)
        // ICANON: Permite ler byte a byte sem Enter
        // ECHO: Desliga o print automático do Linux (nossa CPU fará o print)
        raw.c_lflag &= ~(ICANON | ECHO);

        // 4. Aplica as mudanças
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    void disableRawMode()
    {
        // Restaura o terminal como era antes do programa rodar
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
    }

    // --- O Tick continua quase igual ---
    void tick()
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

        if (ret > 0)
        {
            char buffer[1]; // Lendo 1 byte por vez agora

            // Lê 1 byte instantâneo
            int bytesRead = ::read(STDIN_FILENO, buffer, 1);

            if (bytesRead > 0)
            {
                internalBuffer.push(buffer[0]);
            }
        }

        if (!internalBuffer.empty() && !pic->isPending())
        {
            pic->requestIRQ(1);
        }
    }

    Word read(Address addr) const override
    {
        Keyboard *self = const_cast<Keyboard *>(this);
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
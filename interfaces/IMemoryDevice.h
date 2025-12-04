#pragma once
#include "Types.h"

// Interface pura (classe abstrata em C++)
class IMemoryDevice
{
public:
    virtual ~IMemoryDevice() = default;

    // Métodos virtuais puros
    virtual Word read(Address addr) const = 0;
    virtual void write(Address addr, Word value) = 0;
};
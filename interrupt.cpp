#include "interrupt.h"

Interrupt::Interrupt()
{
    isPending = false;
    cycles = 0;
}

void Interrupt::activate(int cycles)
{
    isPending = true;
    this->cycles = cycles;
}

void Interrupt::cancel()
{
    isPending = false;
}


#include "interrupt.h"

Interrupt::Interrupt()
{
    isPending = false;
    cycles = 0;
}

void Interrupt::activate()
{
    isPending = true;
    cycles = 7;
}

void Interrupt::cancel()
{
    isPending = false;
}


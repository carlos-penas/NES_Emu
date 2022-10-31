#ifndef INTERRUPT_H
#define INTERRUPT_H


class Interrupt
{
public:
    Interrupt();

    bool isPending;
    int cycles;

    void activate(int cycles);
    void cancel();
};

#endif // INTERRUPT_H

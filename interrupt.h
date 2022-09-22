#ifndef INTERRUPT_H
#define INTERRUPT_H


class Interrupt
{
public:
    Interrupt();

    bool isPending;
    int cycles;

    void activate();
    void cancel();
};

#endif // INTERRUPT_H

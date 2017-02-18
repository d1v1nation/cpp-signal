#include <iostream>
#include "signal.h"

signal<int(void)>* backref;
signal<int(int, int)>* facref;

template <int Num>
int printNum() {
    std::cout << "printNum " << Num << '\n';
    return 42;
}

int reentreg() {
    backref->connect(printNum<3>);
    return -42;
}

struct funobj {
    signal<int(void)>::conref conn;
    int num;

    funobj(signal<int(void)>::conref o, int n) : conn(o), num(n) {};
    int operator()() {
        std::cout << "funobj<" << num << ">()" << '\n';

        std::cout << "trying to disconnect " << conn.get_uniq_id() << '\n';

        conn.disconnect();
        return 24;
    }
};

int factorisig(int arg, int acc) {
    if (arg == 1)
        return acc;
    else
        return (*facref)(arg - 1, arg * acc);
}


int main() {
    signal<int(void)> s;
    backref = &s;

    s.connect(printNum<1>);
    auto s1 = s.connect(printNum<2>);

    s();
    std::cout << '\n';

    s1.disconnect();

    s();
    std::cout << '\n';

    s.connect(reentreg);

    s();
    std::cout << '\n';

    s();
    std::cout << '\n';


    signal<int(void)> new_sig;

    auto d = new_sig.connect(printNum<14>);

    funobj o1(d, -1);

    auto clast = new_sig.connect(o1);

    for (int i = 20; i < 40; i++) {
        clast = new_sig.connect(funobj(clast, i));
    }

    new_sig();
    new_sig();
    new_sig();

    std::cout << '\n';

    new_sig();

    std::cout << '\n';


    signal<int(int, int)> fs;
    facref = &fs;

    fs.connect(factorisig);
    std::cout << "factorisig! " << fs(15, 1) << '\n';

}
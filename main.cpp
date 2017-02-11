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

template <int Num>
struct funobj {
    signal<int(void)>::connection* conn;

    funobj() : conn(nullptr) {};
    funobj(signal<int(void)>::connection* o) : conn(o) {};
    int operator()() {
        std::cout << "funobj<" << Num << ">()" << '\n';

        if (conn != nullptr) {
            std::cout << "trying to disconnect " << conn->get_uniq_id() << '\n';

            conn->disconnect();
        }
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

    funobj<6> o1;

    auto c1 = new_sig.connect(o1);

    funobj<7> o2 {&c1};

    auto c2 = new_sig.connect(o2);

    funobj<8> o3 {&c2};

    auto c3 = new_sig.connect(o3);


    new_sig();

    std::cout << '\n';

    new_sig();


    signal<int(int, int)> fs;
    facref = &fs;

    fs.connect(factorisig);
    std::cout << "factorisig! " << fs(8, 1) << '\n';

}
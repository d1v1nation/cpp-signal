//
// Created by cat on 18.02.17.
//

#ifndef SIGNAL_SMALL_CONT_H
#define SIGNAL_SMALL_CONT_H

#include <list>
#include <variant>


struct unit_tp {};
using unit = std::monostate;


template <typename T>
class small_cont;

template <typename T>
class small_cont {

    using long_tp = std::list<T>;
    using vartype = std::variant<unit, T, long_tp>;
    vartype cont;


public:
    small_cont() : cont() {};
    small_cont(small_cont const &other) = default;
    small_cont(small_cont&& other) = default;



    class iterator { // only over shorts, else just pull out list's iterator
        small_cont* parent;
        typename long_tp::iterator inner_iter;
        bool endbool;

        using value_type = T;

        iterator(small_cont* p, bool endbool) : endbool(endbool), parent(p->cont.index() == 2 ? nullptr : p) {
            if (p->cont.index() == 2) {
                if (!endbool)
                    inner_iter = std::get<2>(p->cont).begin();
                else
                    inner_iter = std::get<2>(p->cont).end();
            }
        }
        friend class small_cont<T>;

    public:
        iterator& operator=(iterator const &other) = default;
        iterator(iterator const &other) = default;
        iterator& operator=(iterator&& other) = default;

        bool operator==(iterator const &other) const {
            if (parent == nullptr && other.parent == nullptr) {
                return inner_iter == other.inner_iter;
            } else {
                return (parent == other.parent) && (endbool == other.endbool);
            }
        }

        bool operator!=(iterator const &other) const {
            return !(*this == other);
        }

        iterator& operator++() {
            if (parent == nullptr)
                ++inner_iter;
            else
                endbool = true;

            return *this;
        };

        iterator operator++(int) {
            iterator copy = *this;
            ++(*this);

            return copy;
        }

        T& operator*() {
            if (parent != nullptr) {
                return std::get<T>((*parent).cont);
            }
            else
                return *inner_iter;
        }
    };

    void push_back(T obj) {
        int i = cont.index();
        if (i == 0) {
            cont = obj;
        } else {
            if (i == 1) {
                T temp = std::move(std::get<1>(cont));
                cont = long_tp();
                std::get<long_tp>(cont).emplace_back(std::move(temp));
            }

            std::get<2>(cont).emplace_back(obj);
        }

        return;
    }


    template <typename... Tps>
    void emplace_back(Tps&&... args) {
        int i = cont.index();
        if (i == 0) {
//            cont.~vartype();
//            new(&cont) vartype(T(std::forward<Tps>(args)...));

            cont.template emplace<T>(std::forward<Tps>(args)...);
        } else {
            if (i == 1) {
                T temp = std::move(std::get<1>(cont));
                cont = long_tp();
                std::get<long_tp>(cont).emplace_back(std::move(temp));
            }

            std::get<long_tp>(cont).emplace_back(std::forward<Tps>(args)...);
        }
    }

    void clear() {
        cont = unit{};
    }
    
    void erase(T obj) {
        int i = cont.index();
        if (i == 0)
            return;

        if (i == 1) {
            T& o = std::get<T>(cont);
            if (obj == o)
                clear();
            return;
        }

        if (i == 2) {
            long_tp& l = std::get<long_tp>(cont);
            auto it = std::find(l.begin(), l.end(), obj);
            if (it != l.end()) {
                l.erase(it);
            }

            return;
        }
    }

    void erase(iterator it) {
        int i = cont.index();
        if (i == 0)
            return;

        if (i == 1) {
            T& o = std::get<T>(cont);
            if (*it == o)
                clear();
            return;
        }

        if (i == 2) {
            long_tp& l = std::get<long_tp>(cont);
            if (it.inner_iter != l.end())
                l.erase(it.inner_iter);

            return;
        }
    }

    iterator begin() {
        return iterator(this, cont.index() == 0);
    }

    iterator end() {
        return iterator(this, true);
    }
};



#endif //SIGNAL_SMALL_CONT_H

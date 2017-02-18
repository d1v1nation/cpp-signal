//
// Created by cat on 11.02.17.
//

#ifndef SIGNAL_SIGNAL_H
#define SIGNAL_SIGNAL_H

#include <list>
#include <functional>
#include <queue>
#include "small_cont.h"

template <typename R>
struct signal;

template <typename R, typename... Args>
struct signal<R(Args...)> {

    template <typename T>
    using list = small_cont<T>;

    typedef typename std::function<R(Args...)> callback_tp;

    struct conref {
        bool operator==(conref const & other) const {
            return other.uniq_id == uniq_id;
        }

        conref(conref const &conref) = default;
        conref(conref &&conref) = default;

        conref& operator=(conref const &other) = default;
        conref& operator=(conref &&other) = default;

        bool is_alive() {
            return parent->query_liveness(uniq_id);
        }

        void disconnect() {
            parent->enq_dc(uniq_id);
        }

        signal& get_parent() {
            return *parent;
        }

        int get_uniq_id() {
            return uniq_id;
        }

    private:
        signal* parent;
        int uniq_id;

        conref(int lp, signal* sig) : uniq_id(lp), parent(sig) {};

        friend class signal;
    };

    struct connection {
        bool operator==(connection const & other) const {
            return other.uniq_id == uniq_id;
        }

        connection(connection const &other) = default;
    private:
        callback_tp cback; // less copies please
        int uniq_id;

        template <typename F>
        connection(F f, int lp) : cback(std::move(f)), uniq_id(lp) {};

        friend class signal;
    };

    signal() : callbacks(), entrancy(false), cnt(0) {};

    template <typename F>
    conref connect(F func) {
        connection c(func, cnt);

        if (!entrancy) {
            callbacks.emplace_back(c);
        } else {
            add_q.emplace_back(c);
        }

        cnt++;
        return conref(cnt - 1, this);
    }

    // does not work for void
    // one could specify

    R operator()(Args... args) {
        bool prev = entrancy;
        entrancy = true;
        R result;

        auto ae = callbacks.end();
        auto trail = callbacks.begin();
        (*trail).cback(std::forward<Args>(args)...);
        for (auto it = ++callbacks.begin(); it != ae; it++){
            (*it).cback(std::forward<Args>(args)...);
            trail++;
        }

        result = (*trail).cback(std::forward<Args>(args)...);

        entrancy = prev;

        pop_qs();

        return result;
    }

    // connection is either in queue or active
    // single copy of functional object
private:
    list<connection> callbacks;
    list<int> rm_q;
    list<connection> add_q;
    bool entrancy;
    unsigned cnt = 0;

    friend class connection;

    bool query_liveness(int conn_id) {
        for (conref& c : callbacks) {
            if (c.uniq_id == conn_id)
                return true;
        }

        return false;
    }

    void enq_dc(int conn_id) {
        if (!entrancy) {
            find_and_remove(conn_id);
        } else {
            rm_q.push_back(conn_id);
        }
    }

    void find_and_remove(int conn_id) {
        for (auto it = callbacks.begin(); it != callbacks.end(); it++) {
                if ((*it).uniq_id == conn_id) {
                    callbacks.erase(it);
                    return;
                }
        }
    }

    void pop_qs() {
        for (int id : rm_q) {
            find_and_remove(id);
        }

        rm_q.clear();

        for (connection& c : add_q) {
            callbacks.emplace_back(std::move(c));
        }

        add_q.clear();
    }

};

#endif //SIGNAL_SIGNAL_H

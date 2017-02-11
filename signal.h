//
// Created by cat on 11.02.17.
//

#ifndef SIGNAL_SIGNAL_H
#define SIGNAL_SIGNAL_H

#include <list>
#include <functional>
#include <queue>

template <typename R>
struct signal;

template <typename R, typename... Args>
struct signal<R(Args...)> {

    typedef typename std::function<R(Args...)> callback_tp;

    struct connection {
        bool operator==(connection const & other) const {
            return other.uniq_id == uniq_id;
        }

        connection(connection const &other) = default;
        connection(connection &&other) = default;

        bool is_alive() {
            return parent.query_liveness(uniq_id);
        }

        void disconnect() {
            parent.enq_dc(uniq_id);
        }

        signal& get_parent() {
            return parent;
        }

        int get_uniq_id() {
            return uniq_id;
        }

    private:
        callback_tp cback; // less copies please
        signal& parent;
        int uniq_id;

        template <typename F>
        connection(F f, int lp, signal& sig) : cback(std::move(f)), uniq_id(lp), parent(sig) {};

        friend class signal;
    };

    signal() : callbacks(), entrancy(false), cnt(0) {};

    template <typename F>
    connection connect(F func) {
        connection c(func, cnt++, *this);

        if (!entrancy) {
            callbacks.push_back(c);
        } else {
            add_q.push_back(c);
        }

        return c;
    }

    // does not work for void
    // one could specify

    R operator()(Args... args) {
        entrancy = true;
        R result;

        auto ae = --callbacks.end();
        for (auto it = callbacks.begin(); it != ae; it++){
            (*it).cback(std::forward<Args>(args)...);
        }

        result = (*ae).cback(std::forward<Args>(args)...);

        entrancy = false;

        pop_qs();

        return result;
    }

private:
    std::list<connection> callbacks;
    std::vector<int> rm_q;
    std::vector<connection> add_q;
    bool entrancy;
    unsigned cnt = 0;

    friend class connection;

    bool query_liveness(int conn_id) {
        for (connection& c : callbacks) {
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
        for (typename std::list<connection>::iterator it = callbacks.begin(); it != callbacks.end(); it++) {
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

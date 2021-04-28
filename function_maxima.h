#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <iostream>
#include <memory>
#include <set>

class InvalidArg : public std::exception {
public:
    const char *what() const noexcept {
        return "invalid argument value";
    }
};


template<typename A, typename V>
class FunctionMaxima {

public:
    typedef struct point_type {
        friend class FunctionMaxima;

    private:
        std::shared_ptr<A> arg_p;
        std::shared_ptr<V> value_p;

        point_type(A const &a, V const &v) { // niewidoczny z zewnątrz, widoczny dla FunctionMaxima
            arg_p = std::make_shared<A>(a);
            value_p = std::make_shared<V>(v);
        }

    public:
        V const &value() const noexcept {
            return *value_p;
        }

        A const &arg() const noexcept {
            return *arg_p;
        }

        point_type(const point_type &) = default;

        point_type(point_type &&) = default;

        point_type &operator=(const point_type &) = default;

    } point_type;

private:
    // Zwraca true, jeśli punkt mid powinien być maksimum, i false wpp.
    bool is_maximum(point_type left, point_type mid, point_type right) {
        return (!(mid.value() < left.value()) && !(mid.value() < right.value()));
    }

    struct points_comparator {
        bool operator()(const point_type &a, const point_type &b) const {
            return a.arg() < b.arg();
        }
    };

    struct maxima_comparator {
        bool operator()(const point_type &a, const point_type &b) const {
            if (!(a.value() < b.value()) && !(b.value() < a.value())) {
                return a.arg() < b.arg();
            }
            return b.value() < a.value(); // pierwszy punkt w secie ma mieć największą wartość
        }
    };

    std::multiset<point_type, points_comparator> points;
    std::multiset<point_type, maxima_comparator> maxima;

public:
    using iterator = typename std::multiset<point_type, points_comparator>::iterator;
    using mx_iterator = typename std::multiset<point_type, maxima_comparator>::iterator;
    using size_type = size_t;

    FunctionMaxima() = default;

    FunctionMaxima(const FunctionMaxima & other) {
        std::multiset<point_type, points_comparator> new_points(other.points);
        std::multiset<point_type, maxima_comparator> new_maxima(other.maxima);
        swap(this->points, new_points);
        swap(this->maxima, new_maxima);
    }

    FunctionMaxima &operator=(const FunctionMaxima & other) {
        std::multiset<point_type, points_comparator> new_points(other.points);
        std::multiset<point_type, maxima_comparator> new_maxima(other.maxima);
        swap(this->points, new_points);
        swap(this->maxima, new_maxima);
        return *this;
    }

    size_type size() const noexcept {
        return points.size();
    }

    // iterator wskazujący na pierwszy punkt
    iterator begin() const noexcept {
        return points.begin();
    }

    // iterator wskazujący za ostatni punkt
    iterator end() const noexcept {
        return points.end();
    }

    // Iterator, który wskazuje na punkt funkcji o argumencie a lub end(),
    // jeśli takiego argumentu nie ma w dziedzinie funkcji.
    iterator find(A const &a) const {
        if (points.size() == 0) {
            return points.end();
        }
        V v = points.begin()->value();

        return points.find(point_type(a, v));
    }

    // iterator wskazujący na pierwsze lokalne maksimum
    mx_iterator mx_begin() const noexcept {
        return maxima.begin();
    }

    // iterator wskazujący za ostatnie lokalne maksimum
    mx_iterator mx_end() const noexcept {
        return maxima.end();
    }


    // Zwraca wartość w punkcie a, rzuca wyjątek InvalidArg, jeśli a nie
    // należy do dziedziny funkcji
    V const &value_at(A const &a) const {
        auto point_it = find(a);
        if (point_it == points.end()) {
            throw InvalidArg();
        }
        return point_it->value();
    }


    // Zmienia funkcję tak, żeby zachodziło f(a) = v.
    // Jeśli a nie należy do obecnej dziedziny funkcji, jest do niej dodawany
    void set_value(A const &a, V const &v) {

        point_type new_mid = point_type(a, v); // wartość nowego punku
        auto old_mid_it = points.find(new_mid); // stary punkt o tym argumencie
        auto right_it = points.upper_bound(new_mid); // prawy sąsiad punktu

        auto left_it = (old_mid_it != points.end() ? old_mid_it : right_it); // lewy sąsiad punktu

        // czy lewy sąsiad nowowstawionego punktu istnieje
        bool exists_left = false;
        if (left_it != points.begin()) {
            --left_it;
            exists_left = true;
        }

        // czy stary punkt istnieje
        bool exists_old_mid = (old_mid_it != points.end());

        // czy prawy sąsiad nowowstawionego punktu istnieje
        bool exists_right = (right_it != points.end());

        bool is_in_old = false; // czy stary punkt jest w zbiorze maksimów lokalnych
        bool is_max_left = false; // czy lewy sąsiad powinien być w zbiorze maksimów lokalnych
        bool is_in_left = false; // czy lewy sąsiad jest obecnie w zbiorze maksimów lokalnych
        bool is_max_right = false; // czy prawy sąsiad powinien być w zbiorze maksimów lokalnych
        bool is_in_right = false; // czy prawy sąsiad jest obecnie w zbiorze maksimów lokalnych

        mx_iterator max_it_old, max_it_left, max_it_right;
        if (exists_old_mid) {
            max_it_old = maxima.find(*old_mid_it);
            is_in_old = (max_it_old != maxima.end());
        }
        if (exists_left) {
            max_it_left = maxima.find(*left_it);
            is_max_left = is_maximum(left_it != points.begin() ? *((--left_it)++) : *left_it,
                                     *left_it,
                                     new_mid);
            is_in_left = (maxima.find(*left_it) != maxima.end());
        }
        if (exists_right) {
            max_it_right = maxima.find(*right_it);
            is_max_right = is_maximum(new_mid,
                                      *right_it,
                                      ++right_it != points.end() ? *(right_it--) : *(--right_it));
            is_in_right = (maxima.find(*right_it) != maxima.end());
        }

        // czy nowowstawiony punkt powinien być w zbiorze maksimów lokalnych
        bool is_max_new = is_maximum(exists_left ? *left_it : new_mid,
                                     new_mid,
                                     exists_right ? *right_it : new_mid);

        // iterator na elementy dodane do seta maxima
        mx_iterator insert_max_it_left, insert_max_it_right, insert_max_it_new;
        // czy udało się dodać dany element
        bool insert_max_left = false, insert_max_right = false, insert_max_new = false;
        try {
            if (exists_left && is_max_left && !is_in_left) {
                insert_max_it_left = maxima.insert(*left_it);
                insert_max_left = true;
            }
            if (exists_right && is_max_right && !is_in_right) {
                insert_max_it_right = maxima.insert(*right_it);
                insert_max_right = true;
            }
            if (is_max_new) {
                insert_max_it_new = maxima.insert(new_mid);
                insert_max_new = true;
            }
            points.insert(new_mid); // nie trzeba rollbackować
        }
        catch (...) {
            if (insert_max_left) {
                maxima.erase(insert_max_it_left);
            }
            if (insert_max_right) {
                maxima.erase(insert_max_it_right);
            }
            if (insert_max_new) {
                maxima.erase(insert_max_it_new);
            }
            throw;
        }

        if (exists_left && !is_max_left && is_in_left) {
            maxima.erase(max_it_left);
        }
        if (exists_right && !is_max_right && is_in_right) {
            maxima.erase(max_it_right);
        }
        if (exists_old_mid) {
            points.erase(old_mid_it);
            if (is_in_old) {
                maxima.erase(max_it_old);
            }
        }
    }


    // Usuwa a z dziedziny funkcji.
    // Jeśli a nie należało do dziedziny funkcji, nie dzieje się nic
    void erase(A const &a) {
        auto mid_it = find(a); // stary punkt o tym argumencie
        if (mid_it == points.end()) {
            return;
        }
        auto mid = *mid_it;
        auto right_it = points.upper_bound(mid); // prawy sąsiad punktu
        auto left_it = (mid_it != points.end() ? mid_it : right_it); // lewy sąsiad punktu

        // czy lewy sąsiad nowowstawionego punktu istnieje
        bool exists_left = false;
        if (left_it != points.begin()) {
            --left_it;
            exists_left = true;
        }

        // czy prawy sąsiad nowowstawionego punktu istnieje
        bool exists_right = (right_it != points.end());

        mx_iterator max_it_mid = maxima.find(*mid_it);
        mx_iterator max_it_left = (exists_left ? maxima.find(*left_it) : maxima.end());
        mx_iterator max_it_right = (exists_right ? maxima.find(*right_it) : maxima.end());

        // czy stary punkt o wartości jest w zbiorze maksimów lokalnych
        bool is_in_mid = (max_it_mid != maxima.end());

        bool is_max_left = false; // czy lewy sąsiad powinien być w zbiorze maksimów lokalnych
        bool is_in_left = false; // czy lewy sąsiad jest obecnie w zbiorze maksimów lokalnych
        if (exists_left) {
            is_max_left = is_maximum(left_it != points.begin() ? *((--left_it)++) : *left_it,
                                     *left_it,
                                     exists_right ? *right_it : *left_it);
            is_in_left = (maxima.find(*left_it) != maxima.end());
        }

        bool is_max_right = false; // czy prawy sąsiad powinien być w zbiorze maksimów lokalnych
        bool is_in_right = false; // czy prawy sąsiad jest obecnie w zbiorze maksimów lokalnych
        if (exists_right) {
            is_max_right = is_maximum(exists_left ? *left_it : *right_it,
                                      *right_it,
                                      ++right_it != points.end() ? *(right_it--) : *(--right_it));
            is_in_right = (maxima.find(*right_it) != maxima.end());
        }

        // iterator na elementy dodane do seta maxima
        mx_iterator insert_max_it_left, insert_max_it_right;
        // czy udało się dodać dany element
        bool insert_max_left = false, insert_max_right = false;
        try {
            if (exists_left && is_max_left && !is_in_left) {
                insert_max_it_left = maxima.insert(*left_it);
                insert_max_left = true;
            }
            if (exists_right && is_max_right && !is_in_right) {
                insert_max_it_right = maxima.insert(*right_it);
                insert_max_right = true;
            }
        }
        catch (...) {
            if (insert_max_left) {
                maxima.erase(insert_max_it_left);
            }
            if (insert_max_right) {
                maxima.erase(insert_max_it_right);
            }
            throw;
        }

        if (exists_left && !is_max_left && is_in_left) {
            maxima.erase(max_it_left);
        }
        if (exists_right && !is_max_right && is_in_right) {
            maxima.erase(max_it_right);
        }
        if (is_in_mid) {
            maxima.erase(max_it_mid);
        }
        points.erase(mid_it);
    }

};


#endif //FUNCTION_MAXIMA_H

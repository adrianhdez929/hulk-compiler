#include <vector>
#include <unordered_set>
#include <string>
#include <stdexcept>
// #include <set>

using namespace std;

#ifndef CONTAINER_SET_H
#define CONTAINER_SET_H


class ContainerSet {
    public:
        ContainerSet(bool contains_epsilon = false);
        ContainerSet(const vector<int>& values, bool contains_epsilon = false);
        ContainerSet(const unordered_set<int>& values, bool contains_epsilon = false);
        ~ContainerSet();

        bool add(int value);
        bool extend(const vector<int> values);
        bool set_epsilon(bool value = true);
        bool update(const ContainerSet& other);
        bool epsilon_update(const ContainerSet& other);
        bool hard_update(const ContainerSet& other);

        bool contains(int value) const;
        bool find_match(int value) const;

        size_t size() const;
        bool empty() const;

        bool operator==(const ContainerSet& other) const;
        bool operator!=(const ContainerSet& other) const;
        bool operator==(const unordered_set<int>& other) const;
        bool operator!=(const unordered_set<int>& other) const;

        vector<int> get_values() const;
        unordered_set<int> get_set() const;

        string str() const;

        unordered_set<int>::iterator begin();
        unordered_set<int>::iterator end();
        unordered_set<int>::const_iterator begin() const;
        unordered_set<int>::const_iterator end() const;

    private:
        unordered_set<int> set_;
        bool contains_epsilon;
};

inline bool operator==(const ContainerSet& lhs, const unordered_set<int>& rhs) { return lhs.operator==(rhs); } // { return lhs == rhs; }
inline bool operator!=(const ContainerSet& lhs, const unordered_set<int>& rhs) { return lhs.operator!=(rhs); } // { return lhs != rhs; }

#endif
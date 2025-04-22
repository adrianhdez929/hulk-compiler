#include "ContainerSet.h"
#include <sstream>

namespace utils {
    ContainerSet::ContainerSet(bool contains_epsilon) : contains_epsilon(contains_epsilon) {}
    ContainerSet::ContainerSet(const vector<int>& values, bool contains_epsilon) : contains_epsilon(contains_epsilon) {
        for (int value : values) {
            set_.insert(value);
        }
    }

    ContainerSet::ContainerSet(const unordered_set<int>& values, bool contains_epsilon) : contains_epsilon(contains_epsilon) {
        set_.insert(values.begin(), values.end());
    }
    ContainerSet::~ContainerSet() {
        set_.clear();
    }

    bool ContainerSet::add(int value) {
        return set_.insert(value).second;
    }

    bool ContainerSet::extend(const vector<int> values) {
        bool changed = false;
        for (int value : values) {
            changed |= add(value);
        }
        return changed;
    }

    bool ContainerSet::set_epsilon(bool value) {
        bool changed = contains_epsilon != value;
        contains_epsilon = value;
        return changed;
    }

    // Agregar elementos de otro ContainerSet al actual
    bool ContainerSet::update(const ContainerSet& other) {
        const size_t original_size = set_.size();
        set_.insert(other.set_.begin(), other.set_.end());
        return set_.size() != original_size;
    }
    // Actualizar epsilon. Si alguno de los conjuntos contiene epsilon, el actual tambien lo contiene.
    bool ContainerSet::epsilon_update(const ContainerSet& other) {
        bool previous = contains_epsilon;
        contains_epsilon |= other.contains_epsilon;
        return previous != contains_epsilon;
    }
    // Actualizar el ContainerSet con la informacion de otro ContainerSet incluyendo epsilon
    bool ContainerSet::hard_update(const ContainerSet& other) {
        bool set_updated = update(other);
        bool epsilon_updated = epsilon_update(other);
        return set_updated || epsilon_updated;
    }

    bool ContainerSet::contains(int value) const {
        return set_.find(value) != set_.end();
    }

    bool ContainerSet::empty() const {
        return set_.empty();
    }

    bool ContainerSet::find_match(int value) const {
        return contains(value);
    }

    size_t ContainerSet::size() const {
        return set_.size();
    }

    bool ContainerSet::operator==(const ContainerSet& other) const {
        return set_ == other.set_ && contains_epsilon == other.contains_epsilon;
    }

    bool ContainerSet::operator!=(const ContainerSet& other) const {
        return !(*this == other);
    }
    bool ContainerSet::operator==(const unordered_set<int>& other) const {
        return set_ == other;
    }

    std::string ContainerSet::str() const {
        std::stringstream ss;
        ss << "{";
        for (auto it = set_.begin(); it != set_.end(); ++it) {
            if (it != set_.begin()) ss << ", ";
            ss << *it;
        }
        ss << "} - " << (contains_epsilon ? "true" : "false");
        return ss.str();
    }

    unordered_set<int>::iterator ContainerSet::begin() {
        return set_.begin();
    }
    unordered_set<int>::iterator ContainerSet::end() {
        return set_.end();
    }
    unordered_set<int>::const_iterator ContainerSet::begin() const {
        return set_.begin();
    }
    unordered_set<int>::const_iterator ContainerSet::end() const {
        return set_.end();
    }

    vector<int> ContainerSet::get_values() const {
        return vector<int>(set_.begin(), set_.end());
    }
    unordered_set<int> ContainerSet::get_set() const {
        return set_;
    }
}
#pragma once
#include <vector>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <any>
#include <algorithm>  // Para std::lexicographical_compare
// #include <set>

using namespace std;

template <typename T>
class ContainerSet {
    public:
        ContainerSet(bool contains_epsilon = false) : contains_epsilon_(contains_epsilon) {}
        ContainerSet(const vector<T>& values, bool contains_epsilon = false) : contains_epsilon_(contains_epsilon) { 
            for (const T& value : values) {
                if (value == nullptr) {
                    // throw std::invalid_argument("ContainerSet cannot contain null values");
                }
                set_.insert(value);
            }
        }
        ContainerSet(const unordered_set<T, std::hash<T>, std::equal_to<T>>& values, bool contains_epsilon = false) : contains_epsilon_(contains_epsilon) {
            set_.insert(values.begin(), values.end());
        }
        ~ContainerSet() {
            set_.clear();
        }

        bool add(const T& value) {
            return set_.insert(value).second;
        }
        bool extend(const vector<T>& values){
            bool changed = false;
            for (int value : values) {
                changed |= add(value);
            }
            return changed;
        }

        bool set_epsilon(bool value = true) {
            bool changed = contains_epsilon_ != value;
            contains_epsilon_ = value;
            return changed;
        }
        bool update(const ContainerSet& other){
            const size_t original_size = set_.size();
            set_.insert(other.set_.begin(), other.set_.end());
            return set_.size() != original_size;
        }
        bool epsilon_update(const ContainerSet& other) {
            bool previous = contains_epsilon_;
            contains_epsilon_ = contains_epsilon_ || other.contains_epsilon_;
            return previous != contains_epsilon_;
        }
        bool hard_update(const ContainerSet& other) {
            bool set_updated = update(other);
            bool epsilon_updated = epsilon_update(other);
            return set_updated || epsilon_updated;
        }

        bool contains(T value) const {
            return set_.find(value) != set_.end();
        }
        bool find_match(T value) const {
            return contains(value);
        }

        size_t size() const {
            return set_.size();
        }
        bool empty() const {
            return set_.empty();
        }

        bool operator==(const ContainerSet& other) const {
            return set_ == other.set_ && contains_epsilon_ == other.contains_epsilon_;
        }
        bool operator!=(const ContainerSet& other) const {
            return !(*this == other);
        }
        bool operator==(const unordered_set<T>& other) const {
            return set_ == other;
        }
        bool operator!=(const unordered_set<T>& other) const {
            return !(*this == other);
        }
        //operador == para si T es un tipo de puntero inteligente
        

        // Operador < para permitir que ContainerSet funcione en contenedores ordenados
        bool operator<(const ContainerSet& other) const {
            // Primero comparamos por tamaño
            if (set_.size() != other.set_.size()) {
                return set_.size() < other.set_.size();
            }
            
            // Si tienen el mismo tamaño, convertimos a vectores y comparamos lexicográficamente
            std::vector<T> this_vec(set_.begin(), set_.end());
            std::vector<T> other_vec(other.set_.begin(), other.set_.end());
            
            // Ordenamos los vectores para una comparación consistente
            std::sort(this_vec.begin(), this_vec.end(), [](const T& a, const T& b) {
                return a.get() < b.get();  // Asumiendo que T es un tipo de puntero inteligente
            });
            
            std::sort(other_vec.begin(), other_vec.end(), [](const T& a, const T& b) {
                return a.get() < b.get();  // Asumiendo que T es un tipo de puntero inteligente
            });
            
            // Comparamos elemento por elemento
            for (size_t i = 0; i < this_vec.size(); ++i) {
                if (this_vec[i].get() != other_vec[i].get()) {
                    return this_vec[i].get() < other_vec[i].get();
                }
            }
            
            // Si los conjuntos son idénticos, decidimos por contains_epsilon_
            return contains_epsilon_ < other.contains_epsilon_;
        }

        vector<T> get_values() const {
            return vector<T>(set_.begin(), set_.end());
        }
        const std::unordered_set<T, std::hash<T>, std::equal_to<T>>& get_set() const {
            return set_;
        }

        string str() const {
            // std::stringstream ss;
            // ss << "{";
            // for (auto it = set_.begin(); it != set_.end(); ++it) {
            //     if (it != set_.begin()) ss << ", ";
            //     ss << *it; // Requiere que T tenga operator<< implementado
            // }
            // ss << "} - " << (contains_epsilon_ ? "true" : "false");
            // return ss.str();
            std::string result = "{";
            for (auto it = set_.begin(); it != set_.end(); ++it) {
                if (it != set_.begin()) result += ", ";
                result += std::to_string(*it);
            }
            result += "} - " + std::to_string(contains_epsilon_);
            return result;
    
        }

    auto begin() {
        return set_.begin();
    }
    
    auto end() {
        return set_.end();
    }
    
    auto begin() const {
        return set_.begin();
    }
    
    auto end() const {
        return set_.end();
    }
    bool contains_epsilon() const {
        return contains_epsilon_;
    }

    private:
        std::unordered_set<T, std::hash<T>, std::equal_to<T>> set_;
        bool contains_epsilon_;
};

// inline bool operator==(const ContainerSet& lhs, const unordered_set<int>& rhs) { return lhs.operator==(rhs); } // { return lhs == rhs; }
// inline bool operator!=(const ContainerSet& lhs, const unordered_set<int>& rhs) { return lhs.operator!=(rhs); } // { return lhs != rhs; }
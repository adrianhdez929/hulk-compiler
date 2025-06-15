#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

template <typename T>
class DisjointSet {
    private:
        struct DisjointNode {
            T data;
            DisjointNode* parent;

            DisjointNode(T value) : data(value), parent(this) {}
            
            DisjointNode* representative() {
                if (parent != this) {
                    parent = parent->representative();
                }
                return parent;
            }

            void mergeWith(DisjointNode* other) {
                DisjointNode* root1 = representative();
                DisjointNode* root2 = other->representative();

                if (root1 != root2) { // Solo fusionar si son de conjuntos diferentes
                    root2->parent = root1;
                }
            }
            //operador !=
        };

        std::unordered_map<T, std::unique_ptr<DisjointNode>> nodes_;

    public:
        DisjointSet() = default;
        DisjointSet(const DisjointSet&) = delete; // No copy constructor
        DisjointSet& operator=(const DisjointSet&) = delete; // No copy assignment operator
        DisjointSet(DisjointSet&&) = default; // Move constructor
        DisjointSet& operator=(DisjointSet&&) = default; // Move assignment operator
        ~DisjointSet() {

        }
        void addItem(const T& item) {
            if (nodes_.find(item) == nodes_.end()) {
                nodes_[item] = std::make_unique<DisjointNode>(item);
            }
        }

        void merge(const std::vector<T>& items) {
            if (items.empty()) return;
            for (const auto& item : items) {
                if (nodes_.find(item) == nodes_.end()) {
                    throw std::invalid_argument("Item no existe en el conjunto");
                }
            }
            DisjointNode* root = nodes_[items[0]].get();
            for (size_t i = 1; i < items.size(); ++i) {
                DisjointNode* other = nodes_[items[i]].get();
                root->mergeWith(other);
            }
        }

        std::unordered_set<T> getRepresentatives() const {
            std::unordered_set<T> representatives;
            for (const auto& [key, value] : nodes_) {
                representatives.insert(value->representative()->data);
            }
            return representatives;
        }

        std::unordered_map<T, std::vector<T>> getGroups() {
            std::unordered_map<T, std::vector<T>> groups;
            for (const auto& [key, value] : nodes_) {
                groups[value->representative()->data].push_back(key);
            }
            return groups;
        }

        DisjointNode* find(const T& item) const {
            auto it = nodes_.find(item);
            if (it != nodes_.end()) {
                return it->second->representative();
            }
            return nullptr;
        }

        size_t size() const {
            return getRepresentatives().size();
        }
    
};
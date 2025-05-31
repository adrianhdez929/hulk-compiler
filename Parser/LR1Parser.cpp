#include "LR1Parser.h"
#include <cassert>


pair<map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>, map<Sentence, ContainerSet<shared_ptr<Symbol>>>> LR1Parser::compute_firsts() {
    std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>> firsts;
    bool changed = true;
    
    // Inicializar primeros para terminales
    for (const auto& terminal : G_.Terminals()) {
        ContainerSet<shared_ptr<Symbol>> cs;
        cs.add(terminal);
        firsts[terminal] = cs;
    }
    
    // Inicializar primeros para no terminales
    for (const auto& nonterminal : G_.NonTerminals()) {
        firsts[nonterminal] = ContainerSet<shared_ptr<Symbol>>();
    }
    
    std::map<Sentence, ContainerSet<shared_ptr<Symbol>>> sentence_firsts;
    for (const auto& prod : G_.Productions()) {
        const Production* p = &prod;
        sentence_firsts[p->Right()] = ContainerSet<shared_ptr<Symbol>>();
    }
    while (changed == true) {
        changed = false;

        for (const auto& prod : G_.Productions()) {
            const auto& X = prod.Left();
            const auto& alpha = prod.Right();

            // Firsts de X
            auto& first_X = firsts[X];

            // Firsts de alpha
            auto& first_alpha = sentence_firsts[alpha];

            //Compute local first
            ContainerSet<shared_ptr<Symbol>> local_first = ContainerSet<shared_ptr<Symbol>>();
            // If alpha is epsilon, add epsilon to local first
            for (const auto& symbol : alpha.Symbols()) {
                if (symbol->IsEpsilon()) {
                    local_first.add(symbol);
                    local_first.set_epsilon(true);
                }
            }
            if (!local_first.contains_epsilon()){
                local_first.update(firsts[alpha.Symbols()[0]]);
                int i = 0;
                std::shared_ptr<Symbol> s = alpha.Symbols()[i];
                while (firsts[s].contains_epsilon()) {
                    if (i < alpha.Symbols().size() - 1) {
                        i++;
                        s = alpha.Symbols()[i];
                        if (!firsts[s].contains_epsilon()) {
                            local_first.update(firsts[s]);
                            break;
                        }
                    } else {
                        local_first.add(G_.GetEpsilon());
                        local_first.set_epsilon(true);
                        break;
                    }
                }
            }
            bool changed_alpha = first_alpha.hard_update(local_first);
            bool changed_X = first_X.hard_update(local_first);
            changed = changed || changed_alpha || changed_X;
            
        }
    }
    return std::make_pair(firsts, sentence_firsts);
};
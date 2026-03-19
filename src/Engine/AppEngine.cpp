//
// Created by denni on 3/18/2026.
//

#include "Engine/AppEngine.h"

void AppEngine::AddOrder(const std::shared_ptr<Order>& order) {

    auto& Symbol = order->GetSymbol();

    if (!Engines_.contains(Symbol)) {
        OrderBooks_[Symbol] = std::make_unique<MultisetOrderBook>();
        Engines_[Symbol] = std::make_unique<MatchingEngine>(*OrderBooks_[Symbol]);
    }
    Engines_[Symbol]->ProcessOrder(order,HistoryTrades_[Symbol]);
}

void AppEngine::CancelOrder(int order_id, const std::string& Symbol) {
    auto it = OrderBooks_.find(Symbol);
    if (it == OrderBooks_.end()) return;
    it->second->CancelOrder(order_id);
}

std::vector<std::shared_ptr<Trade>> AppEngine::GetTradesHistory(const std::string &Symbol) const {
    auto it = HistoryTrades_.find(Symbol);
    if (it == HistoryTrades_.end() || it->second.empty()) {
        return {};
    }
    return it->second;
}


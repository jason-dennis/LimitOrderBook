//
// Created by Ognean Jason Dennis on 17.03.2026.
//

/*
 *
 */
#ifndef LIMITORDERBOOK_APPENGINE_H
#define LIMITORDERBOOK_APPENGINE_H
#include "MatchingEngine.h"
#include <unordered_map>

#include "Storage/BinaryOrderBookStorage.h"

class AppEngine{
private:

    std::unordered_map<std::string,std::unique_ptr<MatchingEngine>> Engines_;
    std::unordered_map<std::string,std::unique_ptr<IOrderBook>> OrderBooks_;
    std::unordered_map<std::string,std::vector<std::shared_ptr<Trade>>>HistoryTrades_;

public:

    AppEngine() = default;
    ~AppEngine() = default;

    void AddOrder(std::shared_ptr<Order> order);

};

inline void AppEngine::AddOrder(std::shared_ptr<Order> order) {
    auto engineIt = Engines_.find(order->GetSymbol());
    if (engineIt != Engines_.end()) {
        engineIt->second->ProcessOrder(order,HistoryTrades_[order->GetSymbol()]);
    }
    else {
        OrderBooks_[order->GetSymbol()] = std::make_unique<BinaryOrderBook>();
        Engines_[order->GetSymbol()] = std::make_unique<MatchingEngine>(*OrderBooks_[order->GetSymbol()]);
        Engines_[order->GetSymbol()]->ProcessOrder(order,HistoryTrades_[order->GetSymbol()]);
    }
}


#endif //LIMITORDERBOOK_APPENGINE_H

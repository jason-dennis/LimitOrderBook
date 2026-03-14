//
// Created by denni on 3/11/2026.
//

#ifndef LIMITORDERBOOK_MATCHINGENGINE_H
#define LIMITORDERBOOK_MATCHINGENGINE_H
#include "../../include/Storage/IOrderBookStorage.h"
#include "../../include/Domain/trade.h"
#include <vector>
#include <deque>
#include <unordered_map>

class MatchingEngine {
private:
    std::unordered_map<std::string, std::unique_ptr<IOrderBook>>* OrderBook_;
    std::deque<Trade>HistoryTrades_;
public:
    ~MatchingEngine()=default;

    MatchingEngine(std::unordered_map<std::string, std::unique_ptr<IOrderBook>>* OrderBook):OrderBook_(OrderBook){};

    std::vector<Trade> ProcessOrder(Order& NewOrder);
    const std::unordered_map<std::string, std::unique_ptr<IOrderBook>>* GetOrderBook() const { return OrderBook_; }
    void MatchOrderBid(Order &NewOrder, std::vector<Trade>& Trades);
    void MatchOrderAsk(Order &NewOrder, std::vector<Trade>& Trades);

    std::vector<Trade> ProcessBuyLimit(Order& NewOrder);
    std::vector<Trade> ProcessBuyMarket(Order& NewOrder);
    std::vector<Trade> ProcessSellLimit(Order& NewOrder);
    std::vector<Trade> ProcessSellMarket(Order& NewOrder);
    IOrderBook& GetOrderBook(const std::string& Symbol);

};

#endif //LIMITORDERBOOK_MATCHINGENGINE_H
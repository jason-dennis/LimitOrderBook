//
// Created by denni on 3/11/2026.
//

#ifndef LIMITORDERBOOK_MATCHINGENGINE_H
#define LIMITORDERBOOK_MATCHINGENGINE_H
#include "Storage/IOrderBookStorage.h"
#include "Domain/trade.h"
#include <vector>
#include <deque>

class MatchingEngine {
private:
    IOrderBook& OrderBook_;
    std::deque<Trade>HistoryTrades_;
public:
    ~MatchingEngine()=default;

    MatchingEngine(IOrderBook& OrderBook):OrderBook_(OrderBook){};

    std::vector<Trade> ProcessOrder(Order& NewOrder);

    void MatchOrderBid(Order &NewOrder, std::vector<Trade>& Trades);
    void MatchOrderAsk(Order &NewOrder, std::vector<Trade>& Trades);

    std::vector<Trade> ProcessBuyLimit(Order& NewOrder);
    std::vector<Trade> ProcessBuyMarket(Order& NewOrder);
    std::vector<Trade> ProcessSellLimit(Order& NewOrder);
    std::vector<Trade> ProcessSellMarket(Order& NewOrder);

};

#endif //LIMITORDERBOOK_MATCHINGENGINE_H
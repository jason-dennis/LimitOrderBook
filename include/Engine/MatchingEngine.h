//
// Created by denni on 3/11/2026.
//

#ifndef LIMITORDERBOOK_MATCHINGENGINE_H
#define LIMITORDERBOOK_MATCHINGENGINE_H
#include "../../include/Storage/IOrderBookStorage.h"
#include "../../include/Domain/trade.h"
#include <vector>
#include <deque>

/// scoate history trades de aici si pune l in app engine, renunta la vector trade
/// fa in loc de deque, vector<std::shared_ptr<>

class MatchingEngine {
private:
    IOrderBook& OrderBook_;
    std::deque<Trade>HistoryTrades_;
public:
    ~MatchingEngine()=default;

    MatchingEngine(IOrderBook& OrderBook):OrderBook_(OrderBook){};

    std::vector<Trade> ProcessOrder(Order& NewOrder);
    const IOrderBook& GetOrderBook() const { return OrderBook_; }
    void MatchOrderBid(Order &NewOrder, std::vector<Trade>& Trades);
    void MatchOrderAsk(Order &NewOrder, std::vector<Trade>& Trades);

    std::vector<Trade> ProcessBuyLimit(Order& NewOrder);
    std::vector<Trade> ProcessBuyMarket(Order& NewOrder);
    std::vector<Trade> ProcessSellLimit(Order& NewOrder);
    std::vector<Trade> ProcessSellMarket(Order& NewOrder);

};

#endif //LIMITORDERBOOK_MATCHINGENGINE_H
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
public:
    ~MatchingEngine()=default;

    MatchingEngine(IOrderBook& OrderBook):OrderBook_(OrderBook){};

    void ProcessOrder(std::shared_ptr<Order> NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);
    const IOrderBook& GetOrderBook() const { return OrderBook_; }
    void MatchOrderBid(std::shared_ptr<Order> NewOrder, std::vector<std::shared_ptr<Trade>>&Trades);
    void MatchOrderAsk(std::shared_ptr<Order> NewOrderr, std::vector<std::shared_ptr<Trade>>&Trades);

    void ProcessBuyLimit(std::shared_ptr<Order> NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);
    void ProcessBuyMarket(std::shared_ptr<Order> NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);
    void ProcessSellLimit(std::shared_ptr<Order> NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);
    void ProcessSellMarket(std::shared_ptr<Order> NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);

};

#endif //LIMITORDERBOOK_MATCHINGENGINE_H
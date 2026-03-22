//
// Created by denni on 3/11/2026.
//

#ifndef LIMITORDERBOOK_MATCHINGENGINE_H
#define LIMITORDERBOOK_MATCHINGENGINE_H
#include "../../include/Storage/IOrderBookStorage.h"
#include "../../include/Domain/trade.h"
#include <vector>
#include <atomic>

class MatchingEngine {
private:
    IOrderBook& OrderBook_;
    std::atomic<int>& TradeCounter_;
public:
    ~MatchingEngine()=default;

    MatchingEngine(IOrderBook& OrderBook, std::atomic<int>&TradeCounter):OrderBook_(OrderBook),TradeCounter_(TradeCounter){};

    void ProcessOrder(const std::shared_ptr<Order>& NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);
    const IOrderBook& GetOrderBook() const { return OrderBook_; }
    void MatchOrderBid(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>&Trades);
    void MatchOrderAsk(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>&Trades);

    void ProcessBuyLimit(const std::shared_ptr<Order>& NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);
    void ProcessBuyMarket(const std::shared_ptr<Order>& NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);
    void ProcessSellLimit(const std::shared_ptr<Order>& NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);
    void ProcessSellMarket(const std::shared_ptr<Order>& NewOrder,std::vector<std::shared_ptr<Trade>>&Trades);


};

#endif //LIMITORDERBOOK_MATCHINGENGINE_H
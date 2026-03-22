//
// Created by denni on 3/11/2026.
//

#ifndef LIMITORDERBOOK_MATCHINGENGINE_H
#define LIMITORDERBOOK_MATCHINGENGINE_H
#include "../../include/Storage/IOrderBookStorage.h"
#include "../../include/Domain/trade.h"
#include <vector>
#include <atomic>

/**
 * @class MatchingEngine
 * @brief Processes incoming orders and matches them against the order book.
 * Handles all order types (MARKET, LIMIT) and time-in-force policies (GTC, IOC, FOK).
 * Generates trades when orders are matched.
 */
class MatchingEngine {
private:
    IOrderBook& OrderBook_; ///< Reference to the order book for this symbol
    std::atomic<int>& TradeCounter_; ///< Shared trade ID counter

public:
    ~MatchingEngine() = default;

    /**
     * @brief Constructs a matching engine for a specific order book.
     * @param OrderBook Reference to the order book.
     * @param TradeCounter Reference to the shared trade ID counter.
     */
    MatchingEngine(IOrderBook& OrderBook, std::atomic<int>& TradeCounter)
        : OrderBook_(OrderBook), TradeCounter_(TradeCounter) {};

    /**
     * @brief Processes a new order, routing it to the appropriate handler.
     * @param NewOrder The order to process.
     * @param Trades Output vector where generated trades are appended.
     */
    void ProcessOrder(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>& Trades);

    /**
     * @brief Returns a const reference to the order book.
     * @return The order book.
     */
    const IOrderBook& GetOrderBook() const { return OrderBook_; }

    /**
     * @brief Matches a buy order against existing asks.
     * @param NewOrder The buy order to match.
     * @param Trades Output vector for generated trades.
     */
    void MatchOrderBid(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>& Trades);

    /**
     * @brief Matches a sell order against existing bids.
     * @param NewOrder The sell order to match.
     * @param Trades Output vector for generated trades.
     */
    void MatchOrderAsk(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>& Trades);

    /**
     * @brief Processes a buy limit order.
     */
    void ProcessBuyLimit(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>& Trades);

    /**
     * @brief Processes a buy market order.
     */
    void ProcessBuyMarket(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>& Trades);

    /**
     * @brief Processes a sell limit order.
     */
    void ProcessSellLimit(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>& Trades);

    /**
     * @brief Processes a sell market order.
     */
    void ProcessSellMarket(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>& Trades);
};

#endif //LIMITORDERBOOK_MATCHINGENGINE_H
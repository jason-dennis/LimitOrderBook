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
#include "Storage/MultisetOrderBookStorage.h"
#include <atomic>

/**
 * @class AppEngine.
 * @brief Manage orderbook storage, MatchingEngine  and trades history
 * Handles persistence through CSV file loading and saving
 */
class AppEngine{

private:

    std::unordered_map<std::string,std::unique_ptr<MatchingEngine>> Engines_;///< Matching engines per symbol
    std::unordered_map<std::string,std::unique_ptr<IOrderBook>> OrderBooks_; ///< Order books per symbol
    std::unordered_map<std::string,std::vector<std::shared_ptr<Trade>>>HistoryTrades_; ///< Trade history per symbol
    std::atomic<int> TradeCounter_{1};///< Global trade ID counter

public:

    AppEngine() = default;
    ~AppEngine() = default;

    /**
     * @brief Submits a new order to the matching engine.
     * @param order The order to process.
     */
    void AddOrder(const std::shared_ptr<Order>& order);

    /**
     * @brief Cancels an existing order.
     * @param order_id The ID of the order to cancel.
     * @param Symbol The ticker symbol of the order.
     */
    void CancelOrder(int order_id, const std::string& Symbol);

    /**
     * @brief Retrieves trade history for a given symbol.
     * @param Symbol The ticker symbol.
     * @return Vector of trades for that symbol.
     */
    std::vector<std::shared_ptr<Trade>> GetTradesHistory(const std::string& Symbol) const;

    /**
     * @brief Retrieves the best bid orders for a symbol.
     * @param x Number of price levels to return.
     * @param Symbol The ticker symbol.
     * @return Vector of bid orders sorted by price descending.
     */
    std::vector<std::shared_ptr<Order>> GetBestBids(int x, std::string& Symbol);

    /**
     * @brief Retrieves the best ask orders for a symbol.
     * @param x Number of price levels to return.
     * @param Symbol The ticker symbol.
     * @return Vector of ask orders sorted by price ascending.
     */
    std::vector<std::shared_ptr<Order>> GetBestAsks(int x, std::string& Symbol);

    /**
     * @brief Generates a unique trade ID.
     * @return The next available trade ID.
     */
    int GenerateTradeId();

    /**
     * @brief Loads trade history from trades.csv.
     */
    void LoadFromFile();

    /**
     * @brief Saves trade history to trades.csv.
     */
    void SaveToFile();


};



#endif //LIMITORDERBOOK_APPENGINE_H

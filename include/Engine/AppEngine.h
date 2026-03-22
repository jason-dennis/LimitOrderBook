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

class AppEngine{
private:

    std::unordered_map<std::string,std::unique_ptr<MatchingEngine>> Engines_;
    std::unordered_map<std::string,std::unique_ptr<IOrderBook>> OrderBooks_;
    std::unordered_map<std::string,std::vector<std::shared_ptr<Trade>>>HistoryTrades_;
    std::atomic<int> TradeCounter_{1};

public:

    AppEngine() = default;
    ~AppEngine() = default;

    void AddOrder(const std::shared_ptr<Order>& order);
    void CancelOrder(int order_id, const std::string& Symbol);
    std::vector<std::shared_ptr<Trade>> GetTradesHistory(const std::string &Symbol) const;
    std::vector<std::shared_ptr<Order>> GetBestBids(int x,std::string& Symbol);
    std::vector<std::shared_ptr<Order>> GetBestAsks(int x,std::string& Symbol);
    int GenerateTradeId();
    void LoadFromFile();
    void SaveToFile();


};



#endif //LIMITORDERBOOK_APPENGINE_H

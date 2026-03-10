//
// Created by denni on 3/10/2026.
//

#ifndef LIMITORDERBOOK_MULTISETORDERBOOKSTORAGE_H
#define LIMITORDERBOOK_MULTISETORDERBOOKSTORAGE_H
#include "../Storage/IOrderBookStorage.h"
#include <set>
#include<unordered_map>
#include <iterator>

struct BidOrder {
    bool operator()(const Order& lhs, const Order& rhs) const {
        if (lhs.GetPrice() == rhs.GetPrice()) {
            return lhs.GetTime() < rhs.GetTime();
        }
        return lhs.GetPrice() > rhs.GetPrice();
    }
};

struct AskOrder {
    bool operator()(const Order& lhs, const Order& rhs) const {
        if (lhs.GetPrice() == rhs.GetPrice()) {
            return lhs.GetTime() < rhs.GetTime();
        }
        return lhs.GetPrice() < rhs.GetPrice();
    }
};


class MultisetOrderBook: public IOrderBook {
private:
    std::multiset<Order,BidOrder>bids;
    std::multiset<Order,AskOrder>asks;
    std::unordered_map<int,std::multiset<Order,BidOrder>::iterator>bidLocation;
    std::unordered_map<int,std::multiset<Order,AskOrder>::iterator>askLocation;
public:

    MultisetOrderBook() = default;
    ~MultisetOrderBook() override = default;

    void AddOrder(const Order& order) override;
    void CancelOrder(int order_id) override;
    void UpdateQuantity(int order_id,int new_quantity) override;

    const Order* GetBestBid() const override;
    const Order* GetBestAsk() const override;

    bool IsBidEmpty() const override;
    bool IsAskEmpty() const override;

    void PopBestBid() override;
    void PopBestAsk() override;

};
#endif //LIMITORDERBOOK_MULTISETORDERBOOKSTORAGE_H
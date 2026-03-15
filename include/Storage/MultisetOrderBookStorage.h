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
    /**
     * @struct BidOrder
     * @brief Comparator for the Buy side of the order book.
     * Orders are sorted by Price (Descending) and then by Time (Ascending).
     */
    bool operator()(const Order& lhs, const Order& rhs) const {
        if (lhs.GetPrice() == rhs.GetPrice()) {
            return lhs.GetTime() < rhs.GetTime();
        }
        return lhs.GetPrice() > rhs.GetPrice();
    }
};

struct AskOrder {
    /**
     * @struct AskOrder
     * @brief Comparator for the Sell side of the order book.
     * Orders are sorted by Price (Ascending) and then by Time (Ascending).
     */
    bool operator()(const Order& lhs, const Order& rhs) const {
        if (lhs.GetPrice() == rhs.GetPrice()) {
            return lhs.GetTime() < rhs.GetTime();
        }
        return lhs.GetPrice() < rhs.GetPrice();
    }
};


class MultisetOrderBook: public IOrderBook {
    /**
     * @class MultisetOrderBook
     * @brief A high-performance order book storage implementation.
     * Uses std::multiset to keep orders sorted by price-time priority and
     * std::unordered_map to store iterators for O(1) access by Order ID.
     */
private:
    /// Sorted container for Buy orders
    std::multiset<Order,BidOrder>bids;
    /// Sorted container for Sell orders
    std::multiset<Order,AskOrder>asks;

    /// Index for quick access to Bid iterators by ID
    std::unordered_map<int,std::multiset<Order,BidOrder>::iterator>bidLocation;
    /// Index for quick access to Ask iterators by ID
    std::unordered_map<int,std::multiset<Order,AskOrder>::iterator>askLocation;
public:

    MultisetOrderBook() = default;
    ~MultisetOrderBook() override = default;

    void AddOrder(const Order& order) override;
    void CancelOrder(int order_id) override;
    void UpdateQuantity(int order_id,int new_quantity) override;

    const Order* GetBestBid()  override;
    const Order* GetBestAsk()  override;

    bool IsBidEmpty() const override;
    bool IsAskEmpty() const override;
    bool CanFillQuantityAsks(int Quantity, uint64_t Price) const override;
    bool CanFillQuantityBids(int Quantity, uint64_t Price) const override;

    void PopBestBid() override;
    void PopBestAsk() override;


};
#endif //LIMITORDERBOOK_MULTISETORDERBOOKSTORAGE_H
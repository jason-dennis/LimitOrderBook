//
// Created by denni on 3/10/2026.
//

#ifndef LIMITORDERBOOK_MULTISETORDERBOOKSTORAGE_H
#define LIMITORDERBOOK_MULTISETORDERBOOKSTORAGE_H
#include "../Storage/IOrderBookStorage.h"
#include <set>
#include<unordered_map>

struct BidOrder {
    /**
     * @struct BidOrder
     * @brief Comparator for the Buy side of the order book.
     * Orders are sorted by Price (Descending) and then by Time (Ascending).
     */
    bool operator()(const std::shared_ptr<Order>& lhs, const std::shared_ptr<Order>&  rhs) const {
        if (lhs->GetPrice() == rhs->GetPrice()) {
            return lhs->GetTime() < rhs->GetTime();
        }
        return lhs->GetPrice() > rhs->GetPrice();
    }
};

struct AskOrder {
    /**
     * @struct AskOrder
     * @brief Comparator for the Sell side of the order book.
     * Orders are sorted by Price (Ascending) and then by Time (Ascending).
     */
    bool operator()(const std::shared_ptr<Order> & lhs, const std::shared_ptr<Order> &rhs) const {
        if (lhs->GetPrice() == rhs->GetPrice()) {
            return lhs->GetTime() < rhs->GetTime();
        }
        return lhs->GetPrice() < rhs->GetPrice();
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
    std::multiset<std::shared_ptr<Order>,BidOrder>bids;
    /// Sorted container for Sell orders
    std::multiset<std::shared_ptr<Order>,AskOrder>asks;

    /// Index for quick access to Bid iterators by ID
    std::unordered_map<int,std::multiset<std::shared_ptr<Order>,BidOrder>::iterator>bidLocation;
    /// Index for quick access to Ask iterators by ID
    std::unordered_map<int,std::multiset<std::shared_ptr<Order>,AskOrder>::iterator>askLocation;
public:

    MultisetOrderBook() = default;
    ~MultisetOrderBook() override = default;
    /**
         * @brief Adds a new order to the book.
         * @param order The order object to be registered.
         */
    void AddOrder(std::shared_ptr<Order> order) override;

    /**
     * @brief Removes an order from the book based on its ID.
     * @param order_id The unique identifier of the order to be removed.
     */
    void CancelOrder(int order_id) override;

    /**
     * @brief Modifies the volume of an existing order.
     * @param order_id The unique identifier of the order.
     * @param new_quantity The updated quantity.
     */
    void UpdateQuantity(int order_id,int new_quantity) override;

    /**
 * @brief Accesses the highest-priced Buy order.
 * @return Pointer to the best Bid, or nullptr if the side is empty.
 */
    const std::shared_ptr<Order>GetBestBid()  override;

    /**
     * @brief Accesses the lowest-priced Ask order.
     * @return Pointer to the best Ask, or nullptr if the side is empty.
     */
    const std::shared_ptr<Order> GetBestAsk()  override;


    std::vector<std::shared_ptr<Order>> GetBestBids(int x)  override;
    std::vector<std::shared_ptr<Order>> GetBestAsks(int x)  override;

    /**
     * @brief Checks for the presence of Buy orders.
     * @return true if the Bid side is empty.
     */
    bool IsBidEmpty() const override;

    /**
     * @brief Checks for the presence of Sell orders.
     * @return true if the Ask side is empty.
     */
    bool IsAskEmpty() const override;
    bool CanFillQuantityAsks(int Quantity, uint64_t Price)  override;
    bool CanFillQuantityBids(int Quantity, uint64_t Price)  override;

    /**
     * @brief Removes the top-priority Buy order from the book.
     */
    void PopBestBid() override;

    /**
    * @brief Removes the top-priority Buy order from the book.
    */
    void PopBestAsk() override;


};
#endif //LIMITORDERBOOK_MULTISETORDERBOOKSTORAGE_H
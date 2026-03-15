//
// Created by denni on 3/10/2026.
//
#include "../../include/Storage/MultisetOrderBookStorage.h"

void MultisetOrderBook::AddOrder(const Order &order) {
    /**
     * @brief Adds a new order to the storage.
     * Inserts the order into the appropriate side (Bid/Ask) and indexes its location.
     * @param order The order object to be added.
     * @complexity O(log N) for insertion, O(1) for indexing.
    */
    if (order.GetSide()==OrderSide::BUY) {
        auto it = bids.insert(order);
        bidLocation[order.GetOrderID()]=it;
    }
    else {
        auto it=asks.insert(order);
        askLocation[order.GetOrderID()]=it;
    }
}

void MultisetOrderBook::CancelOrder(int order_id) {
    /**
     * @brief Cancel an order from the storage by its id.
     * Update the order status to CANCELED and removes it from all internal structures
     * @param order_id  Unique identifier of the order to cancel.
     * @complexity O(log N).
     */

    auto bidIt = bidLocation.find(order_id);
    if (bidIt != bidLocation.end()) {
        bidIt->second->SetStatus(OrderStatus::CANCELED);
        bids.erase(bidIt->second);
        bidLocation.erase(order_id);
        return;
    }

    auto askIt = askLocation.find(order_id);
    if (askIt != askLocation.end()) {
        askIt->second->SetStatus(OrderStatus::CANCELED);
        asks.erase(askIt->second);
        askLocation.erase(order_id);
    }
}

void MultisetOrderBook::UpdateQuantity(int order_id, int new_quantity) {
    /**
     * @brief Updates the quantity and status of an existing order.
     * If the new quantity is 0, the order is marked as FILLED and removed from storage.
     * Otherwise, the status is updated to PARTIALLY_FILLED.
     * @param order_id Unique identifier of the order to be updated.
     * @param new_quantity The new volume for the order.
     * @complexity O(1) to find the order in the map and O(log N) if removal from the multiset is required.
     */
    auto bidIt = bidLocation.find(order_id);
    if (bidIt != bidLocation.end()) {
        bidIt->second->SetQuantity(new_quantity);
        if (new_quantity>0) {
            bidIt->second->SetStatus(OrderStatus::PARTIALLY_FILLED);
        }
        else {
            bidIt->second->SetStatus(OrderStatus::FILLED);
            bids.erase(bidIt->second);
            bidLocation.erase(order_id);
        }
        return;
    }

    auto askIt = askLocation.find(order_id);
    if (askIt != askLocation.end()) {
        askIt->second->SetQuantity(new_quantity);
        if (new_quantity>0) {
            askIt->second->SetStatus(OrderStatus::PARTIALLY_FILLED);
        }
        else {
            askIt->second->SetStatus(OrderStatus::FILLED);
            asks.erase(askIt->second);
            askLocation.erase(order_id);
        }
    }

}

const Order * MultisetOrderBook::GetBestBid()  {
    /**
     * @brief Returns the best (highest price) Bid order.
     * @return Const pointer to the top Bid, or nullptr if empty.
     */
    if (bids.empty()) {
        return nullptr;
    }
    return &(*bids.begin());
}

const Order * MultisetOrderBook::GetBestAsk()  {
    /**
     * @brief Returns the best (lowest price) Ask order.
     * @return Const pointer to the top Ask, or nullptr if empty.
     */
    if (asks.empty()) {
        return nullptr;
    }
    return &(*asks.begin());
}

bool MultisetOrderBook::IsBidEmpty() const {
    /**
     * @brief Checks if there are any Buy orders in the book.
     */
    return bids.empty();
}

bool MultisetOrderBook::IsAskEmpty() const {
    /**
     * @brief Checks if there are any Sell orders in the book.
     */
    return asks.empty();
}

bool MultisetOrderBook::CanFillQuantityAsks(int Quantity, uint64_t Price) const {

    for (auto &order : asks) {
        if (order.GetPrice() > Price) {
            break;
        }
        Quantity-=std::min(Quantity,order.GetQuantity());
        if (Quantity == 0) {
            return true;
        }
    }

    return false;
}

bool MultisetOrderBook::CanFillQuantityBids(int Quantity, uint64_t Price) const {

    for (auto &order : bids) {
        if (order.GetPrice() < Price) {
            break;
        }
        Quantity-=std::min(Quantity,order.GetQuantity());
        if (Quantity == 0) {
            return true;
        }
    }

    return false;
}

void MultisetOrderBook::PopBestBid() {
    /**
     * @brief Removes the top Bid order from the book.
     * Usually called after a full match. Sets status to FILLED before removal.
     */
    if (bids.empty()) {
        return;
    }
    auto it=bids.begin();
    it->SetStatus(OrderStatus::FILLED);
    int order_id=it->GetOrderID();
    bids.erase(it);
    bidLocation.erase(order_id);
}

void MultisetOrderBook::PopBestAsk() {
    /**
     * @brief Removes the top Ask order from the book.
     * Usually called after a full match. Sets status to FILLED before removal.
     */
    if (asks.empty()){
        return;
    }
    auto it=asks.begin();
    it->SetStatus(OrderStatus::FILLED);
    int order_id=it->GetOrderID();
    asks.erase(it);
    askLocation.erase(order_id);
}

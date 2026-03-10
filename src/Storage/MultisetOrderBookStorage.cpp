//
// Created by denni on 3/10/2026.
//
#include "../../include/Storage/MultisetOrderBookStorage.h"

void MultisetOrderBook::AddOrder(const Order &order) {
    if (order.GetSide()==OrderSide::BUY) {
        auto it=bids.insert(order);
        bidLocation[order.GetOrderID()]=it;
    }
    else {
        auto it=asks.insert(order);
        askLocation[order.GetOrderID()]=it;
    }
}

void MultisetOrderBook::CancelOrder(int order_id) {

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
    auto bidIt = bidLocation.find(order_id);
    if (bidIt != bidLocation.end()) {
        bidIt->second->SetQuantity(new_quantity);
        if (new_quantity>0) {
            bidIt->second->SetStatus(OrderStatus::PARTIALLY_FILLED);
        }
        else if (new_quantity==0) {
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
        else if (new_quantity==0) {
            askIt->second->SetStatus(OrderStatus::FILLED);
            asks.erase(askIt->second);
            askLocation.erase(order_id);
        }
    }

}

const Order * MultisetOrderBook::GetBestBid() const {
    if (bids.empty()) {
        return nullptr;
    }
    return &(*bids.begin());
}

const Order * MultisetOrderBook::GetBestAsk() const {
    if (asks.empty()) {
        return nullptr;
    }
    return &(*asks.begin());
}

bool MultisetOrderBook::IsBidEmpty() const {
    return bids.empty();
}

bool MultisetOrderBook::IsAskEmpty() const {
    return asks.empty();
}

void MultisetOrderBook::PopBestBid() {
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
    if (asks.empty()){
        return;
    }
    auto it=asks.begin();
    it->SetStatus(OrderStatus::FILLED);
    int order_id=it->GetOrderID();
    asks.erase(it);
    askLocation.erase(order_id);
}

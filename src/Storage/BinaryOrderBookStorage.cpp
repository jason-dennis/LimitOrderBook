//
// Created by denni on 3/15/2026.
//
#include "Storage/BinaryOrderBookStorage.h"


uint64_t BinaryOrderBook::CalcInd(uint64_t x) {
    return x/64;
}

uint64_t BinaryOrderBook::CalcBit(uint64_t x) {
    return x%64;
}

void BinaryOrderBook::AddOrder(const std::shared_ptr<Order>order) {

    if (order->GetSide()==OrderSide::BUY) {
        uint64_t Price = order->GetPrice();
        Node* node = BidList_[Price].AddNode(order);
        BidLocation[order->GetOrderID()]=node;

        uint64_t ind_level1 = CalcInd(Price);
        uint64_t bit_level1= CalcBit(Price);
        level1_bid[ind_level1]|=(1ULL<<bit_level1);

        uint64_t ind_level2 = CalcInd(ind_level1);
        uint64_t bit_level2= CalcBit(ind_level1);
        level2_bid[ind_level2]|=(1ULL<<bit_level2);

        uint64_t ind_level3 = CalcInd(ind_level2);
        uint64_t bit_level3= CalcBit(ind_level2);
        level3_bid[ind_level3]|=(1ULL<<bit_level3);

        uint64_t ind_level4 = CalcInd(ind_level3);
        uint64_t bit_level4= CalcBit(ind_level3);
        level4_bid[ind_level4]|=(1ULL<<bit_level4);
    }
    else {
        uint64_t Price = order->GetPrice();
        Node* node = AskList_[Price].AddNode(order);
        AskLocation[order->GetOrderID()]=node;

        uint64_t ind_level1 = CalcInd(Price);
        uint64_t bit_level1= CalcBit(Price);
        level1_ask[ind_level1]|=(1ULL<<bit_level1);

        uint64_t ind_level2 = CalcInd(ind_level1);
        uint64_t bit_level2= CalcBit(ind_level1);
        level2_ask[ind_level2]|=(1ULL<<bit_level2);

        uint64_t ind_level3 = CalcInd(ind_level2);
        uint64_t bit_level3= CalcBit(ind_level2);
        level3_ask[ind_level3]|=(1ULL<<bit_level3);

        uint64_t ind_level4 = CalcInd(ind_level3);
        uint64_t bit_level4= CalcBit(ind_level3);
        level4_ask[ind_level4]|=(1ULL<<bit_level4);
    }


}

void BinaryOrderBook::DeleteBid(int order_id, Node* node, uint64_t Price) {

    BidList_[Price].DeleteNode(node);
    BidLocation.erase(order_id);

    if (BidList_[Price].IsEmpty()) {
        uint64_t ind_level1 = CalcInd(Price);
        uint64_t bit_level1= CalcBit(Price);
        level1_bid[ind_level1] &= ~(1ULL<<bit_level1);

        if (!level1_bid[ind_level1]) {
            uint64_t ind_level2 = CalcInd(ind_level1);
            uint64_t bit_level2= CalcBit(ind_level1);
            level2_bid[ind_level2] &= ~(1ULL<<bit_level2);

            if (!level2_bid[ind_level2]) {
                uint64_t ind_level3 = CalcInd(ind_level2);
                uint64_t bit_level3= CalcBit(ind_level2);
                level3_bid[ind_level3] &= ~(1ULL<<bit_level3);

                if (!level3_bid[ind_level3]) {
                    uint64_t ind_level4 = CalcInd(ind_level3);
                    uint64_t bit_level4= CalcBit(ind_level3);
                    level4_bid[ind_level4] &= ~(1ULL<<bit_level4);
                }
            }
        }
    }
}

void BinaryOrderBook::DeleteAsk(int order_id, Node* node, uint64_t Price) {
    AskList_[Price].DeleteNode(node);
    AskLocation.erase(order_id);

    if (AskList_[Price].IsEmpty()) {
        uint64_t ind_level1 = CalcInd(Price);
        uint64_t bit_level1= CalcBit(Price);
        level1_ask[ind_level1] &= ~(1ULL<<bit_level1);

        if (!level1_ask[ind_level1]) {
            uint64_t ind_level2 = CalcInd(ind_level1);
            uint64_t bit_level2= CalcBit(ind_level1);
            level2_ask[ind_level2] &= ~(1ULL<<bit_level2);

            if (!level2_ask[ind_level2]) {
                uint64_t ind_level3 = CalcInd(ind_level2);
                uint64_t bit_level3= CalcBit(ind_level2);
                level3_ask[ind_level3] &= ~(1ULL<<bit_level3);

                if (!level3_ask[ind_level3]) {
                    uint64_t ind_level4 = CalcInd(ind_level3);
                    uint64_t bit_level4= CalcBit(ind_level3);
                    level4_ask[ind_level4] &= ~(1ULL<<bit_level4);
                }
            }
        }
    }
}

void BinaryOrderBook::CancelOrder(int order_id) {

    auto bidIt = BidLocation.find(order_id);
    if (bidIt != BidLocation.end()) {
        const Order &order = *bidIt->second->GetOrder();
        uint64_t Price = order.GetPrice();
        order.SetStatus(OrderStatus::CANCELED);
        DeleteBid(order_id, bidIt->second, Price);
        return;
    }

    auto askIt =  AskLocation.find(order_id);
    if (askIt != AskLocation.end()) {
        const Order &order = *askIt->second->GetOrder();
        uint64_t Price = order.GetPrice();
        order.SetStatus(OrderStatus::CANCELED);
        DeleteAsk(order_id, askIt->second, Price);
    }
}

void BinaryOrderBook::UpdateQuantity(int order_id, int new_quantity) {

    auto bidIt = BidLocation.find(order_id);
    if (bidIt != BidLocation.end()) {
        const Order &order = *bidIt->second->GetOrder();
        order.SetQuantity(new_quantity);
        if (new_quantity>0) {
            order.SetStatus(OrderStatus::PARTIALLY_FILLED);
        }
        else {
            order.SetStatus(OrderStatus::FILLED);
            DeleteBid(order_id,bidIt->second,order.GetPrice());
        }
        return;
    }

    auto askIt = AskLocation.find(order_id);
    if (askIt != AskLocation.end()) {
        const Order &order = *askIt->second->GetOrder();
        order.SetQuantity(new_quantity);
        if (new_quantity>0) {
            order.SetStatus(OrderStatus::PARTIALLY_FILLED);
        }
        else {
            order.SetStatus(OrderStatus::FILLED);
            DeleteAsk(order_id,askIt->second,order.GetPrice());
        }
    }


}

const std::shared_ptr<Order> BinaryOrderBook::GetBestBid()  {


    for (int i=(int)DIM_level4-1;i>=0;--i) {
        if (level4_bid[i] > 0) {

            uint64_t bit_level4 = (63 - __builtin_clzll(level4_bid[i]));
            uint64_t ind_level3 = i*64 + bit_level4;

            uint64_t bit_level3 = (63 - __builtin_clzll(level3_bid[ind_level3]));
            uint64_t ind_level2 = ind_level3*64 + bit_level3;

            uint64_t bit_level2 = (63 - __builtin_clzll(level2_bid[ind_level2]));
            uint64_t ind_level1 = ind_level2*64 + bit_level2;

            uint64_t bit_level1 = (63 - __builtin_clzll(level1_bid[ind_level1]));

            uint64_t Price = ind_level1*64 + bit_level1;

            Node* node = BidList_[Price].GetHead();
            return node->GetOrder();
        }
    }

    return nullptr;
}

const std::shared_ptr<Order> BinaryOrderBook::GetBestAsk() {
    for (uint64_t i=0;i< DIM_level4; ++i) {
        if (level4_ask[i] > 0) {

            uint64_t bit_level4 = (__builtin_ctzll(level4_ask[i]));
            uint64_t ind_level3 = i*64 + bit_level4;

            uint64_t bit_level3 = (__builtin_ctzll(level3_ask[ind_level3]));
            uint64_t ind_level2 = ind_level3*64 + bit_level3;

            uint64_t bit_level2 = (__builtin_ctzll(level2_ask[ind_level2]));
            uint64_t ind_level1 = ind_level2*64 + bit_level2;

            uint64_t bit_level1 = (__builtin_ctzll(level1_ask[ind_level1]));

            uint64_t Price = ind_level1*64 + bit_level1;

            Node* node = AskList_[Price].GetHead();
            return node->GetOrder();
        }
    }

    return nullptr;

}

bool BinaryOrderBook::IsBidEmpty() const {

    for (int i=0;i<DIM_level4;++i) {
        if (level4_bid[i] > 0) {
            return false;
        }
    }

    return true;
}

bool BinaryOrderBook::IsAskEmpty() const {
    for (uint64_t i=0;i<DIM_level4;++i) {
        if (level4_ask[i] > 0) {
            return false;
        }
    }
    return true;
}

bool BinaryOrderBook::CanFillQuantityAsks(int Quantity, uint64_t Price) const {
    return true;
}

bool BinaryOrderBook::CanFillQuantityBids(int Quantity, uint64_t Price) const {
    return true;
}

void BinaryOrderBook::PopBestBid() {
    for (int i=DIM_level4-1;i>=0;--i) {
        if (level4_bid[i] > 0) {

            uint64_t bit_level4 = (63 - __builtin_clzll(level4_bid[i]));
            uint64_t ind_level3 = i*64 + bit_level4;

            uint64_t bit_level3 = (63 - __builtin_clzll(level3_bid[ind_level3]));
            uint64_t ind_level2 = ind_level3*64 + bit_level3;

            uint64_t bit_level2 = (63 - __builtin_clzll(level2_bid[ind_level2]));
            uint64_t ind_level1 = ind_level2*64 + bit_level2;

            uint64_t bit_level1 = (63 - __builtin_clzll(level1_bid[ind_level1]));

            uint64_t Price = ind_level1*64 + bit_level1;

            Node* node = BidList_[Price].GetHead();
            DeleteBid(node->GetOrder()->GetOrderID(),node,Price);
            return ;
        }
    }
}

void BinaryOrderBook::PopBestAsk() {
    for (uint64_t i=0;i< DIM_level4; ++i) {
        if (level4_ask[i] > 0) {

            uint64_t bit_level4 = ( __builtin_ctzll(level4_ask[i]));
            uint64_t ind_level3 = i*64 + bit_level4;

            uint64_t bit_level3 = (__builtin_ctzll(level3_ask[ind_level3]));
            uint64_t ind_level2 = ind_level3*64 + bit_level3;

            uint64_t bit_level2 = ( __builtin_ctzll(level2_ask[ind_level2]));
            uint64_t ind_level1 = ind_level2*64 + bit_level2;

            uint64_t bit_level1 = (__builtin_ctzll(level1_ask[ind_level1]));

            uint64_t Price = ind_level1*64 + bit_level1;

            Node* node = AskList_[Price].GetHead();
            DeleteAsk(node->GetOrder()->GetOrderID(),node,Price);
            return;
        }
    }

}

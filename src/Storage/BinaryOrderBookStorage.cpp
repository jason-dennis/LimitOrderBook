//
// Created by denni on 3/15/2026.
//
#include "Storage/BinaryOrderBookStorage.h"


int BinaryOrderBook::CalcInd(int x) {
    return x/64;
}

int BinaryOrderBook::CalcBit(int x) {
    return x%64;
}

void BinaryOrderBook::AddOrder(const Order &order) {

    uint64_t Price = order.GetPrice();
    Node* node = PriceList_[Price].AddNode(&order);
    NodeLocation[order.GetOrderID()]=node;

    int ind_level1 = CalcInd(Price);
    int bit_level1= CalcBit(Price);
    level1_[ind_level1]|=(1ULL<<bit_level1);

    int ind_level2 = CalcInd(ind_level1);
    int bit_level2= CalcBit(ind_level1);
    level2_[ind_level2]|=(1ULL<<bit_level2);

    int ind_level3 = CalcInd(ind_level2);
    int bit_level3= CalcBit(ind_level2);
    level3_[ind_level3]|=(1ULL<<bit_level3);

    int ind_level4 = CalcInd(ind_level3);
    int bit_level4= CalcBit(ind_level3);
    level4_[ind_level4]|=(1ULL<<bit_level4);


}

void BinaryOrderBook::CancelOrder(int order_id) {

    Node* node = NodeLocation[order_id];
    const Order &order = *node->GetOrder();
    uint64_t Price = order.GetPrice();

    PriceList_[Price].DeleteNode(node);
    if (PriceList_[Price].IsEmpty()) {

    }

}


//
// Created by denni on 3/10/2026.
//
#ifndef LIMITORDERBOOK_IORDERBOOKSTORAGE_H
#define LIMITORDERBOOK_IORDERBOOKSTORAGE_H
#include "../Domain/order.h"
class IOrderBook {
public:
    virtual ~IOrderBook()=default;

    virtual void AddOrder(const Order& order) = 0;
    virtual void CancelOrder(int order_id) = 0;
    virtual void UpdateQuantity(int order_id,int new_quantity) = 0;

    virtual const Order* GetBestBid() const = 0;
    virtual const Order* GetBestAsk() const = 0;

    virtual  bool IsBidEmpty() const = 0;
    virtual bool IsAskEmpty() const = 0;

    virtual void PopBestBid() = 0;
    virtual void PopBestAsk() = 0;
};
#endif //LIMITORDERBOOK_IORDERBOOKSTORAGE_H
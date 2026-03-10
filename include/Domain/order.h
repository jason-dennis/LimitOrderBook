//
// Created by denni on 3/7/2026.
//
#pragma once
#ifndef LIMITORDERBOOK_ORDER_H
#define LIMITORDERBOOK_ORDER_H
#include <string>
#include <chrono>
#include <ostream>
#include <ctime>

enum class OrderStatus {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELED,
    REJECTED
};

enum class OrderSide {
    BUY,
    SELL
};

enum class TimeInForce {
    GTC,
    IOC,
    FOK
};

enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT
};
constexpr const char* ToString(OrderStatus status) {
    constexpr const char* names[] = {"NEW", "PARTIALLY_FILLED", "FILLED", "CANCELED", "REJECTED"};
    return names[static_cast<int>(status)];
}

constexpr const char* ToString(OrderSide side) {
    constexpr const char* names[] = {"BUY", "SELL"};
    return names[static_cast<int>(side)];
}

constexpr const char* ToString(OrderType type) {
    constexpr const char* names[] = {"MARKET", "LIMIT", "STOP"};
    return names[static_cast<int>(type)];
}

constexpr const char* ToString(TimeInForce tif) {
    constexpr const char* names[] = {"GTC", "IOC", "FOK"};
    return names[static_cast<int>(tif)];
}

class Order {
private:
    const int Order_ID_;
    const int Trader_ID_;
    OrderSide Side_;
    OrderType Type_;
    std::string Symbol_;
    uint64_t Price_;
    mutable int Quantity_;
    std::chrono::system_clock::time_point Timestamp_;
    TimeInForce TIF_;
    mutable OrderStatus Status_;

public:
    virtual ~Order() = default;
    Order(int Order_ID, int Trader_ID, OrderSide Side,
          OrderType Type,
          const std::string& Symbol, uint64_t Price, int Quantity,
          std::chrono::system_clock::time_point Timestamp,
          TimeInForce TIF,
          OrderStatus Status);

    int GetOrderID() const {return Order_ID_;}
    int GetTraderID() const {return Trader_ID_;}
    uint64_t GetPrice() const {return Price_;}
    int GetQuantity() const {return Quantity_;}
    std::chrono::system_clock::time_point GetTime() const {return Timestamp_;}

    const std::string& GetSymbol() const {return Symbol_;}
    OrderType GetType() const {return Type_;}
    TimeInForce GetTIF() const {return TIF_;}
    OrderStatus GetStatus() const {return Status_;}
    OrderSide GetSide() const {return Side_;}

    void SetStatus(OrderStatus New_Status) const {Status_=New_Status;}
    void SetQuantity(int new_quantity)const {Quantity_=new_quantity;}

};

std::ostream& operator<<(std::ostream& os,const Order& order);

#endif //LIMITORDERBOOK_ORDER_H
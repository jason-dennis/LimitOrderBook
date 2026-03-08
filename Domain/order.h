//
// Created by denni on 3/7/2026.
//

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
    int Quantity_;
    std::chrono::system_clock::time_point Timestamp_;
    TimeInForce TIF_;
    OrderStatus Status_;

public:
    virtual ~Order() = default;
    Order(int Order_ID, int Trader_ID, OrderSide Side,
          OrderType Type,
          const std::string& Symbol, uint64_t Price, int Quantity,
          std::chrono::system_clock::time_point Timestamp,
          TimeInForce TIF,
          OrderStatus Status);

    int Get_OrderID() const {return Order_ID_;}
    int Get_TraderID() const {return Trader_ID_;}
    uint64_t Get_Price() const {return Price_;}
    int Get_Quantity() const {return Quantity_;}
    std::chrono::system_clock::time_point Get_Time() const {return Timestamp_;}

    const std::string& Get_Symbol() const {return Symbol_;}
    OrderType Get_Type() const {return Type_;}
    TimeInForce Get_TIF() const {return TIF_;}
    OrderStatus Get_Status() const {return Status_;}
    OrderSide Get_Side() const {return Side_;}

    void Set_Status(OrderStatus New_Status){Status_=New_Status;}

};

std::ostream& operator<<(std::ostream& os,const Order& order);

#endif //LIMITORDERBOOK_ORDER_H
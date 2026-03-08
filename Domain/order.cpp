//
// Created by denni on 3/7/2026.
//

#include "order.h"
#include <stdexcept>
#include <iomanip>

Order::Order(int Order_ID, int Trader_ID, OrderSide Side,
            OrderType Type,
            const std::string& Symbol, uint64_t Price,int Quantity,
            std::chrono::system_clock::time_point Timestamp,
            TimeInForce TIF,
            OrderStatus Status):
    Order_ID_(Order_ID),
    Trader_ID_(Trader_ID),
    Side_(Side),
    Type_(Type),
    Symbol_(Symbol),
    Price_(Price),
    Quantity_(Quantity),
    Timestamp_(Timestamp),
    TIF_(TIF),
    Status_(Status){
    if (Price_ == 0) {
        throw std::invalid_argument("Price must be higher than 0.0");
    }
    if (Quantity_<= 0) {
        throw std::invalid_argument("Quantity must be higher than 0");
    }
    if (Symbol_.empty()) {
        throw std::invalid_argument("Symbol shouldn't be empty");
    }
}

std::ostream& operator<<(std::ostream& os, const Order& order) {
    // Transform in (HH:MM:SS)
    auto tp=order.Get_Time();
    std::time_t t =std::chrono::system_clock::to_time_t(tp);
    std::tm bt;
    #ifdef _WIN32
        localtime_s(&bt,&t);
    #else
        localtime_r(&bt,&t);
    #endif


    // Calculate milliseconds
    auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()%std::chrono::seconds(1)).count();

    os << "Order[ID: " << order.Get_OrderID()
       << " | Trader: " <<order.Get_TraderID()
       << " | Side: " <<ToString(order.Get_Side())
       << " | Time:" << std::put_time(&bt,"%H:%M:%S.")<<std::setfill('0')<<std::setw(3)<<ms
       << " | Symbol: " << order.Get_Symbol()
       << " | Price: " << order.Get_Price()
       << " | Type: " <<ToString(order.Get_Type())
       << " | Quantity: " <<order.Get_Quantity()
       << " | TIF: " << ToString(order.Get_TIF())
       << " | Status: "<< ToString(order.Get_Status())
       << " ]";

    return os;
}
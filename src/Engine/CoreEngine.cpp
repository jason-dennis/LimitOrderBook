//
// Created by denni on 3/17/2026.
//

#include "Engine/CoreEngine.h"
#include <atomic>
int CoreEngine::GenerateID() {
    static std::atomic<int> counter(1);
    return counter++;
}


void CoreEngine::CreateOrder(uint64_t Price, int Quantity, std::string Type,
                            std::string Symbol, std::string TIF,int TraderID,std::string Side) {

    Price*=Tick;
    int OrderID=GenerateID();
    OrderSide side = ToOrderSide(Side);
    OrderType type = ToOrderType(Type);
    TimeInForce tif = ToTimeInForce(TIF);
    auto now = std::chrono::system_clock::now();
    OrderStatus Status = OrderStatus::NEW;

    std::shared_ptr<Order> order = make_shared<Order>(OrderID,TraderID,side,type,Symbol,
                                    Price,Quantity,now,tif,Status);
    /*
     *  Add to user and App engine  !!!!!!!!!!
     */
}



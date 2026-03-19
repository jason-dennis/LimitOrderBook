//
// Created by denni on 3/17/2026.
//

#include "Engine/CoreEngine.h"
#include <atomic>
int CoreEngine::GenerateID() {
    static std::atomic<int> counter(1);
    return counter++;
}


void CoreEngine::CreateOrder(uint64_t Price, int Quantity, const std::string& Type,
                            const std::string& Symbol, const std::string& TIF,int TraderID,const std::string& Side) {

    Price*=Tick;
    int OrderID=GenerateID();
    OrderSide side = ToOrderSide(Side);
    OrderType type = ToOrderType(Type);
    TimeInForce tif = ToTimeInForce(TIF);
    auto now = std::chrono::system_clock::now();
    OrderStatus Status = OrderStatus::NEW;

    std::shared_ptr<Order> order = make_shared<Order>(OrderID,TraderID,side,type,Symbol,
                                    Price,Quantity,now,tif,Status);
    App_.AddOrder(order);
    Orders_.push_back(order);
}

void CoreEngine::CancelOrder(int order_id,const std::string& Symbol) {
    App_.CancelOrder(order_id,Symbol);
}

std::vector<std::shared_ptr<Trade>> CoreEngine::GetTradesHistory(const std::string &Symbol) {
    return App_.GetTradesHistory(Symbol);
}
std::vector<std::shared_ptr<Order>> CoreEngine::GetOrders() {
    return Orders_;
}

std::vector<std::shared_ptr<Order>> CoreEngine::GetBestBids(int x,std::string& Symbol) {
    return App_.GetBestBids(x,Symbol);
}

std::vector<std::shared_ptr<Order>> CoreEngine::GetBestAsks(int x,std::string& Symbol) {
    return App_.GetBestAsks(x,Symbol);
}






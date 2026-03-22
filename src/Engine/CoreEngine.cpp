//
// Created by denni on 3/17/2026.
//

#include "Engine/CoreEngine.h"
#include <fstream>
#include <sstream>
#include <string>

int CoreEngine::GenerateID() {
    return OrderCounter_++;
}

CoreEngine::CoreEngine(): App_() {
    App_.LoadFromFile();
    this->LoadFromFile();
}

void CoreEngine::Save() {
    this->SaveToFile();
    App_.SaveToFile();
}
void CoreEngine ::LoadFromFile() {

    std::ifstream file("orders.csv");
    if (!file.is_open()) return;

    std::string line;

    std::getline(file,line);
    int maxID = 0;
    while (std::getline(file,line)) {
        std::stringstream ss(line);

        std::string item;

        std::getline(ss,item,',');
        int ID = std::stoi(item);

        std::getline(ss,item,',');
        int TraderID =std::stoi(item);

        std::getline(ss,item,',');
        auto Side = ToOrderSide(item);

        std::getline(ss,item,',');
        auto Type = ToOrderType(item);

        std::getline(ss,item,',');
        auto Symbol = item;

        std::getline(ss,item,',');
        uint64_t Price = std::stoull(item);

        std::getline(ss,item,',');
        int Qty = std::stoi(item);

        std::getline(ss,item,',');
        auto ms = std::stoll(item);
        std::chrono::system_clock::time_point TimeStamp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ms));

        std::getline(ss,item,',');
        auto TIF = ToTimeInForce(item);

        std::getline(ss,item,',');
        auto Status = ToOrderStatus(item);

        maxID= std::max(maxID,ID);
        if (Qty > 0) {
            std::shared_ptr<Order> NewOrder = std::make_shared<Order>(ID,TraderID,Side,Type,Symbol,Price,Qty,TimeStamp,TIF,Status);
            Orders_[Symbol].push_back(NewOrder);
            App_.AddOrder(NewOrder);
        }
    }
    OrderCounter_ = maxID+1;
    file.close();
}

void CoreEngine ::SaveToFile() {

    std::ofstream file("orders.csv");

    if (!file.is_open()) return;

    file << "ID,TraderID,Side,Type,Symbol,Price,Qty,Timestamp,TIF,Status" << std::endl;
    for (auto [Symbol,Orders]: Orders_) {
        for (auto order:Orders) {
            if (order->GetQuantity() == 0)
                continue;
            file << order->GetOrderID()<<",";
            file << order->GetTraderID()<<",";
            file << ToString(order->GetSide())<<",";
            file << ToString(order->GetType())<<",";
            file << order->GetSymbol()<<",";
            file << order->GetPrice()<<",";
            file << order->GetQuantity()<<",";
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            order->GetTime().time_since_epoch()).count();
            file << ms<<",";
            file<< ToString(order->GetTIF())<<",";
            file << ToString(order->GetStatus())<<std::endl;
        }
    }
}



void CoreEngine::CreateOrder(float Price, int Quantity, const std::string& Type,
                             const std::string& Symbol, const std::string& TIF,int TraderID,const std::string& Side) {

    uint64_t newPrice = Price * Tick;
    int OrderID=GenerateID();
    OrderSide side = ToOrderSide(Side);
    OrderType type = ToOrderType(Type);
    TimeInForce tif = ToTimeInForce(TIF);
    auto now = std::chrono::system_clock::now();
    OrderStatus Status = OrderStatus::NEW;

    std::shared_ptr<Order> order = make_shared<Order>(OrderID,TraderID,side,type,Symbol,
                                    newPrice,Quantity,now,tif,Status);
    App_.AddOrder(order);
    Orders_[Symbol].push_back(order);
}

void CoreEngine::CancelOrder(int order_id,const std::string& Symbol) {
    App_.CancelOrder(order_id,Symbol);
}

std::vector<std::shared_ptr<Trade>> CoreEngine::GetTradesHistory(const std::string &Symbol) {
    return App_.GetTradesHistory(Symbol);
}
std::vector<std::shared_ptr<Order>> CoreEngine::GetOrders(const std::string& Symbol) {
    return Orders_[Symbol];
}

std::vector<std::shared_ptr<Order>> CoreEngine::GetBestBids(int x,std::string& Symbol) {
    return App_.GetBestBids(x,Symbol);
}

std::vector<std::shared_ptr<Order>> CoreEngine::GetBestAsks(int x,std::string& Symbol) {
    return App_.GetBestAsks(x,Symbol);
}






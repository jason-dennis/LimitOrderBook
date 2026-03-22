//
// Created by denni on 3/18/2026.
//

#include "Engine/AppEngine.h"
#include <fstream>
#include <sstream>
#include <string>

int AppEngine::GenerateTradeId() {
    return TradeCounter_++;
}

void AppEngine::LoadFromFile() {

    std::ifstream file("trades.csv");
    if (!file.is_open()) return;

    std::string line;

    std::getline(file,line);
    int maxID=0;
    while (std::getline(file,line)) {
        std::stringstream ss(line);

        std::string item;

        std::getline(ss,item,',');
        int ID = std::stoi(item);

        std::getline(ss,item,',');
        int MakerID =std::stoi(item);

        std::getline(ss,item,',');
        int TakerID = std::stoi(item);

        std::getline(ss,item,',');
        uint64_t Price =std::stoull(item);

        std::getline(ss,item,',');
        int Qty = std::stoi(item);

        std::getline(ss,item,',');
        std::string Symbol = std::string(item);

        std::getline(ss,item,',');
        auto ms = std::stoll(item);
        std::chrono::system_clock::time_point TimeStamp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ms));

        maxID= std::max(maxID,ID);
        std::shared_ptr<Trade> NewTrade = std::make_shared<Trade>(ID,MakerID,TakerID,Price,Qty,Symbol,TimeStamp);
        HistoryTrades_[Symbol].push_back(NewTrade);
    }
    TradeCounter_= maxID+1;

    file.close();
}

void AppEngine::SaveToFile() {

    std::ofstream file("trades.csv");

    if (!file.is_open()) return;

    file << "ID,MakerID,TakerID,Price,Qty,Symbol,Timestamp" << std::endl;
    for (auto [Symbol,Trades]: HistoryTrades_) {
        for (auto trade:Trades) {

            file << trade->GetID()<<",";
            file << trade->GetMakerID()<<",";
            file << trade->GetTakerID()<<",";
            file << trade->GetPrice()<<",";
            file << trade->GetQuantity()<<",";
            file << trade->GetSymbol()<<",";
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            trade->GetTimestamp().time_since_epoch()).count();
            file << ms<<std::endl;
        }
    }
}


void AppEngine::AddOrder(const std::shared_ptr<Order>& order) {

    auto& Symbol = order->GetSymbol();

    if (!Engines_.contains(Symbol)) {
        OrderBooks_[Symbol] = std::make_unique<BinaryOrderBook>();
        Engines_[Symbol] = std::make_unique<MatchingEngine>(*OrderBooks_[Symbol],TradeCounter_);
    }
    Engines_[Symbol]->ProcessOrder(order,HistoryTrades_[Symbol]);
}

void AppEngine::CancelOrder(int order_id, const std::string& Symbol) {
    auto it = OrderBooks_.find(Symbol);
    if (it == OrderBooks_.end()) return;
    it->second->CancelOrder(order_id);
}

std::vector<std::shared_ptr<Trade>> AppEngine::GetTradesHistory(const std::string &Symbol) const {
    auto it = HistoryTrades_.find(Symbol);
    if (it == HistoryTrades_.end() || it->second.empty()) {
        return {};
    }
    return it->second;
}

std::vector<std::shared_ptr<Order>> AppEngine::GetBestBids(int x, std::string &Symbol) {
    auto it = OrderBooks_.find(Symbol);
    if (it == OrderBooks_.end()){
        return {};
    }
   return it->second->GetBestBids(x);
}

std::vector<std::shared_ptr<Order>> AppEngine::GetBestAsks(int x, std::string &Symbol) {
    auto it = OrderBooks_.find(Symbol);
    if (it == OrderBooks_.end()){
        return {};
    }
    return it->second->GetBestAsks(x);
}




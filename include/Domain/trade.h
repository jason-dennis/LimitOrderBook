//
// Created by denni on 3/11/2026.
//

#ifndef LIMITORDERBOOK_TRADE_H
#define LIMITORDERBOOK_TRADE_H
#include <chrono>
class Trade {
private:
    int MakerID_;
    int TakerID_;
    uint64_t Price_;
    int Quantity_;
    std::chrono::system_clock::time_point Timestamp_;

public:
     ~Trade()=default;

    Trade(int MakerID, int TakerID,
         uint64_t Price,int Quantity,
        std::chrono::system_clock::time_point Timestamp):
        MakerID_(MakerID),
        TakerID_(TakerID),
        Price_(Price),
        Quantity_(Quantity),
        Timestamp_(Timestamp){
        }

    int GetMakerID() const {return MakerID_;}
    int GetTakerID() const {return TakerID_;}
    uint64_t GetPrice() const {return Price_;}
    int GetQuantity() const {return Quantity_;}
    std::chrono::system_clock::time_point GetTimestamp() const {return Timestamp_;}
};
#endif //LIMITORDERBOOK_TRADE_H
//
// Created by denni on 3/17/2026.
//

#ifndef LIMITORDERBOOK_COREENGINE_H
#define LIMITORDERBOOK_COREENGINE_H
#include "Engine/UserEngine.h"
#include "Domain/order.h"
#include "Engine/AppEngine.h"


class CoreEngine {
private:
    UserEngine User_;
    AppEngine App_;

    const int Tick = 100; // 0.01
    int GenerateID();

public:
    CoreEngine(): User_(),App_(){}
    ~CoreEngine() = default;
    void CreateOrder(uint64_t Price,int Quantity,std::string Type, std::string Symbol,
                    std::string TIF,int TraderID,std::string Side);
    void CancelOrder(std::shared_ptr<Order> order);
    void ModifyOrder(std::shared_ptr<Order> order);
    std::vector<std::shared_ptr<Trade>> GetTradesHistory();
    // get x best bids/asks



};
#endif //LIMITORDERBOOK_COREENGINE_H
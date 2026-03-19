//
// Created by denni on 3/17/2026.
//

#ifndef LIMITORDERBOOK_COREENGINE_H
#define LIMITORDERBOOK_COREENGINE_H
#include "Domain/order.h"
#include "Engine/AppEngine.h"



class CoreEngine {
private:

    AppEngine App_;
    std::vector<std::shared_ptr<Order>>Orders_;

    const int Tick = 100; // 0.01
    int GenerateID();

public:
    CoreEngine(): App_(){}
    ~CoreEngine() = default;
    void CreateOrder(uint64_t Price,int Quantity,const std::string& Type, const std::string& Symbol,
                    const std::string& TIF,int TraderID,const std::string& Side);
    void CancelOrder(int order_id,const std::string& Symbol);
    std::vector<std::shared_ptr<Trade>> GetTradesHistory(const std::string& Symbol);
    std::vector<std::shared_ptr<Order>> GetOrders();
    std::vector<std::shared_ptr<Order>> GetBestBids(int x,std::string& Symbol);
    std::vector<std::shared_ptr<Order>> GetBestAsks(int x,std::string& Symbol);



};
#endif //LIMITORDERBOOK_COREENGINE_H
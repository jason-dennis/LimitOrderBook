//
// Created by denni on 3/21/2026.
//

#ifndef LIMITORDERBOOK_ORDERSPANEL_H
#define LIMITORDERBOOK_ORDERSPANEL_H
#include "Engine/CoreEngine.h"
class OrdersPanel {
private:
    CoreEngine& Engine_;
    std::shared_ptr<Order> SelectedOrder_;
    std::shared_ptr<Trade> SelectedTrade_;
public:

    OrdersPanel(CoreEngine& Engine):Engine_(Engine),SelectedOrder_(nullptr),SelectedTrade_(nullptr){}
    ~OrdersPanel() = default;

    void Render(const std::string& Symbol);
};
#endif //LIMITORDERBOOK_ORDERSPANEL_H
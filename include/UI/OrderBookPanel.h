//
// Created by denni on 3/21/2026.
//

#ifndef LIMITORDERBOOK_ORDERBOOKPANEL_H
#define LIMITORDERBOOK_ORDERBOOKPANEL_H
#include "../../include/Engine/CoreEngine.h"

class OrderBookPanel {
private:
    CoreEngine& Engine_;

public:

    OrderBookPanel(CoreEngine& Engine): Engine_(Engine){}
    ~OrderBookPanel() = default;

    void Render(std::string& Symbol);
    void RenderBids(std::string& Symbol);
    void RenderAsks(std::string& Symbol);

};

#endif //LIMITORDERBOOK_ORDERBOOKPANEL_H
//
// Created by denni on 3/20/2026.
//

#ifndef LIMITORDERBOOK_TRADETERMINAL_H
#define LIMITORDERBOOK_TRADETERMINAL_H
#include "../../include/Engine/CoreEngine.h"
#include "UI/OrderBookPanel.h"
#include  "UI/GraphPanel.h"
#include "UI/OrderEntryPanel.h"
#include "UI/OrdersPanel.h"
#include <iostream>


class TradeTerminal {
private:
    CoreEngine& Engine_;
    OrderBookPanel OrderBook_;
    GraphPanel Graph_;
    OrderEntryPanel OrderEntry_;
    OrdersPanel Orders_;
public:

    TradeTerminal(CoreEngine& Engine): Engine_(Engine),
    OrderBook_(Engine),Graph_(Engine),
    OrderEntry_(Engine), Orders_(Engine){}
    ~TradeTerminal() = default;

    void Render();
    bool IsRunning() const {
        return !OrderEntry_.IsExit();
    }
};

#endif //LIMITORDERBOOK_TRADETERMINAL_H
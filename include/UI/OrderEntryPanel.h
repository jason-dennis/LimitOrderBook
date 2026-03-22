//
// Created by denni on 3/21/2026.
//

#ifndef LIMITORDERBOOK_ORDERENTRYPANEL_H
#define LIMITORDERBOOK_ORDERENTRYPANEL_H
#include "Engine/CoreEngine.h"
class OrderEntryPanel {
private:
    CoreEngine& Engine_;
    int SymbolIndex;
    float PriceBuy;
    float PriceSell;
    int QtyBuy;
    int QtySell;
    int TifBuy;
    int TifSell;
    int TypeBuy;
    int TypeSell;
    static constexpr char* SymbolOptions[] = {"AAPL", "NVDA", "MSFT", "GOOGL", "AMZN", "META", "AVGO", "MELI", "ISRG", "TSLA"};
    static constexpr const char* TIFOptions[] = {"GTC", "IOC", "FOK"};
    static constexpr const char* TypeOptions[] = {"LIMIT", "MARKET"};
    bool PriceBuyInitialized_ = false;
    bool PriceSellInitialized_ = false;
    bool Exit;

public:

    OrderEntryPanel(CoreEngine& Engine): Engine_(Engine), SymbolIndex(0),PriceBuy(0),PriceSell(0),
    QtyBuy(0),QtySell(0),TifBuy(0),TifSell(0),TypeBuy(0),TypeSell(0),Exit(false){}
    ~OrderEntryPanel() = default;

    void Render();
    const char* GetSelectedSymbol() const { return SymbolOptions[SymbolIndex]; }
    bool IsExit() const {return Exit;}

};

#endif //LIMITORDERBOOK_ORDERENTRYPANEL_H
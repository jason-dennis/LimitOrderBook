//
// Created by denni on 3/11/2026.
//
#include "Engine/MatchingEngine.h"
void MatchingEngine::ProcessOrder(const std::shared_ptr<Order>& NewOrder,std::vector<std::shared_ptr<Trade>>&Trades) {

    OrderType Type=NewOrder->GetType();
    OrderSide Side=NewOrder->GetSide();

    if (Side == OrderSide::BUY) {

        if (Type == OrderType::LIMIT) {
            ProcessBuyLimit(NewOrder,Trades);
        }
        else {
            ProcessBuyMarket(NewOrder,Trades);
        }
    }
    else  {
        if (Type == OrderType::LIMIT) {
             ProcessSellLimit(NewOrder, Trades);
        }
        else  {
            ProcessSellMarket(NewOrder, Trades);
        }
    }
}

void MatchingEngine::MatchOrderBid(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>&Trades) {
    
    while (!OrderBook_.IsAskEmpty()
          and NewOrder->GetQuantity() > 0
          and (NewOrder->GetPrice() >= OrderBook_.GetBestAsk()->GetPrice()
              or NewOrder->GetType() == OrderType::MARKET)) {
        auto now = std::chrono::system_clock::now();
        int Quantity = std::min(NewOrder->GetQuantity(), OrderBook_.GetBestAsk()->GetQuantity());
        int ID=TradeCounter_++;
        std::shared_ptr<Trade> NewTrade = std::make_shared<Trade>(ID,OrderBook_.GetBestAsk()->GetTraderID(),
                               NewOrder->GetTraderID(),
                               OrderBook_.GetBestAsk()->GetPrice(),
                               Quantity,NewOrder->GetSymbol(),
                               now
        );
        Trades.push_back(std::move(NewTrade));

        NewOrder->SetQuantity(NewOrder->GetQuantity() - Quantity);
        auto BestAsk=OrderBook_.GetBestAsk();
        if (BestAsk->GetQuantity() - Quantity == 0) {
            BestAsk->SetStatus(ToOrderStatus("FILLED"));
            OrderBook_.PopBestAsk();
        } else {
            OrderBook_.UpdateQuantity(BestAsk->GetOrderID(),
                                      BestAsk->GetQuantity() - Quantity);
            BestAsk->SetStatus(ToOrderStatus("PARTIALLY_FILLED"));
        }
    }
}

void MatchingEngine::MatchOrderAsk(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>&Trades) {

    while (!OrderBook_.IsBidEmpty()
           and NewOrder->GetQuantity() > 0
           and (NewOrder->GetPrice() <= OrderBook_.GetBestBid()->GetPrice()
               or NewOrder->GetType() == OrderType::MARKET)) {
        auto now = std::chrono::system_clock::now();
        int Quantity = std::min(NewOrder->GetQuantity(), OrderBook_.GetBestBid()->GetQuantity());
        int ID=TradeCounter_++;
        std::shared_ptr<Trade>NewTrade = std::make_shared<Trade>(ID,OrderBook_.GetBestBid()->GetTraderID(),
                               NewOrder->GetTraderID(),
                               OrderBook_.GetBestBid()->GetPrice(),
                               Quantity,NewOrder->GetSymbol(),
                               now
        );
        Trades.push_back(std::move(NewTrade));

        NewOrder->SetQuantity(NewOrder->GetQuantity() - Quantity);
        auto BestBid =OrderBook_.GetBestBid();
        if (BestBid->GetQuantity() - Quantity == 0) {
            BestBid->SetStatus(ToOrderStatus("FILLED"));
            OrderBook_.PopBestBid();
        } else {
            OrderBook_.UpdateQuantity(BestBid->GetOrderID(),
                                      BestBid->GetQuantity() - Quantity);
            BestBid->SetStatus(ToOrderStatus("PARTIALLY_FILLED"));
        }
    }

}

void MatchingEngine::ProcessBuyLimit(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>&Trades) {

    if (NewOrder->GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityAsks(NewOrder->GetQuantity(),NewOrder->GetPrice())) {
            MatchOrderBid(NewOrder, Trades);
            NewOrder->SetStatus(ToOrderStatus("FILLED"));
        }
        else {
            NewOrder->SetStatus(ToOrderStatus("REJECTED"));
        }
        return;
    }

    int InitialQty = NewOrder->GetQuantity();
    MatchOrderBid(NewOrder, Trades);
    if (NewOrder->GetTIF() == TimeInForce::GTC and NewOrder->GetQuantity() > 0) {
        OrderBook_.AddOrder(NewOrder);
    }
    if (NewOrder->GetQuantity()< InitialQty) {
        NewOrder->SetStatus(ToOrderStatus("PARTIALLY_FILLED"));
    }
    if (NewOrder->GetQuantity() == 0) {
        NewOrder->SetStatus(ToOrderStatus("FILLED"));
    }
}

void MatchingEngine::ProcessBuyMarket(const std::shared_ptr<Order>& NewOrder, std::vector<std::shared_ptr<Trade>>&Trades) {

    if (NewOrder->GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityAsks(NewOrder->GetQuantity(),NewOrder->GetPrice())) {
            MatchOrderBid(NewOrder, Trades);
            NewOrder->SetStatus(ToOrderStatus("FILLED"));
        }
        else {
            NewOrder->SetStatus(ToOrderStatus("REJECTED"));
        }
        return;
    }

    int InitialQty = NewOrder->GetQuantity();
    MatchOrderBid(NewOrder, Trades);
    if (NewOrder->GetQuantity()< InitialQty) {
        NewOrder->SetStatus(ToOrderStatus("PARTIALLY_FILLED"));
    }
    if (NewOrder->GetQuantity() == 0) {
        NewOrder->SetStatus(ToOrderStatus("FILLED"));
    }

}

void MatchingEngine::ProcessSellLimit(const std::shared_ptr<Order>& NewOrder,std::vector<std::shared_ptr<Trade>>&Trades) {


    if (NewOrder->GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityBids(NewOrder->GetQuantity(),NewOrder->GetPrice())) {
            MatchOrderAsk(NewOrder, Trades);
            NewOrder->SetStatus(ToOrderStatus("FILLED"));
        }
        else {
            NewOrder->SetStatus(ToOrderStatus("REJECTED"));
        }
        return;
    }

    int InitialQty = NewOrder->GetQuantity();
    MatchOrderAsk(NewOrder, Trades);
    if (NewOrder->GetTIF() == TimeInForce::GTC and NewOrder->GetQuantity() > 0) {
        OrderBook_.AddOrder(NewOrder);
    }
    if (NewOrder->GetQuantity()< InitialQty) {
        NewOrder->SetStatus(ToOrderStatus("PARTIALLY_FILLED"));
    }
    if (NewOrder->GetQuantity() == 0) {
        NewOrder->SetStatus(ToOrderStatus("FILLED"));
    }

}

void MatchingEngine::ProcessSellMarket(const std::shared_ptr<Order>& NewOrder,std::vector<std::shared_ptr<Trade>>&Trades) {


    if (NewOrder->GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityBids(NewOrder->GetQuantity(),NewOrder->GetPrice())) {
            MatchOrderAsk(NewOrder, Trades);
            NewOrder->SetStatus(ToOrderStatus("FILLED"));
        }
        else {
            NewOrder->SetStatus(ToOrderStatus("REJECTED"));
        }
        return;
    }

    int InitialQty = NewOrder->GetQuantity();
    MatchOrderAsk(NewOrder, Trades);
    if (NewOrder->GetQuantity()< InitialQty) {
        NewOrder->SetStatus(ToOrderStatus("PARTIALLY_FILLED"));
    }
    if (NewOrder->GetQuantity() == 0) {
        NewOrder->SetStatus(ToOrderStatus("FILLED"));
    }
}



//
// Created by denni on 3/11/2026.
//
#include "Engine/MatchingEngine.h"


void MatchingEngine::ProcessOrder(std::shared_ptr<Order> NewOrder,std::vector<std::shared_ptr<Trade>>&Trades) {

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

void MatchingEngine::MatchOrderBid(std::shared_ptr<Order> NewOrder, std::vector<std::shared_ptr<Trade>>&Trades) {
    
    while (!OrderBook_.IsAskEmpty()
          and NewOrder->GetQuantity() > 0
          and (NewOrder->GetPrice() >= OrderBook_.GetBestAsk()->GetPrice()
              or NewOrder->GetType() == OrderType::MARKET)) {
        auto now = std::chrono::system_clock::now();
        int Quantity = std::min(NewOrder->GetQuantity(), OrderBook_.GetBestAsk()->GetQuantity());
        std::shared_ptr<Trade> NewTrade = std::make_shared<Trade>(OrderBook_.GetBestAsk()->GetTraderID(),
                               NewOrder->GetTraderID(),
                               OrderBook_.GetBestAsk()->GetPrice(),
                               Quantity,
                               now
        );
        Trades.push_back(std::move(NewTrade));

        NewOrder->SetQuantity(NewOrder->GetQuantity() - Quantity);
        if (OrderBook_.GetBestAsk()->GetQuantity() - Quantity == 0) {
            OrderBook_.PopBestAsk();
        } else {
            OrderBook_.UpdateQuantity(OrderBook_.GetBestAsk()->GetOrderID(),
                                      OrderBook_.GetBestAsk()->GetQuantity() - Quantity);
        }
    }
}

void MatchingEngine::MatchOrderAsk(std::shared_ptr<Order> NewOrder, std::vector<std::shared_ptr<Trade>>&Trades) {

    while (!OrderBook_.IsBidEmpty()
           and NewOrder->GetQuantity() > 0
           and (NewOrder->GetPrice() <= OrderBook_.GetBestBid()->GetPrice()
               or NewOrder->GetType() == OrderType::MARKET)) {
        auto now = std::chrono::system_clock::now();
        int Quantity = std::min(NewOrder->GetQuantity(), OrderBook_.GetBestBid()->GetQuantity());
        std::shared_ptr<Trade>NewTrade = std::make_shared<Trade>(OrderBook_.GetBestBid()->GetTraderID(),
                               NewOrder->GetTraderID(),
                               OrderBook_.GetBestBid()->GetPrice(),
                               Quantity,
                               now
        );
        Trades.push_back(std::move(NewTrade));

        NewOrder->SetQuantity(NewOrder->GetQuantity() - Quantity);
        if (OrderBook_.GetBestBid()->GetQuantity() - Quantity == 0) {
            OrderBook_.PopBestBid();
        } else {
            OrderBook_.UpdateQuantity(OrderBook_.GetBestBid()->GetOrderID(),
                                      OrderBook_.GetBestBid()->GetQuantity() - Quantity);
        }
    }
}

void MatchingEngine::ProcessBuyLimit(std::shared_ptr<Order> NewOrder, std::vector<std::shared_ptr<Trade>>&Trades) {

    if (NewOrder->GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityAsks(NewOrder->GetQuantity(),NewOrder->GetPrice())) {
            MatchOrderBid(NewOrder, Trades);
        }
        return;
    }

    MatchOrderBid(NewOrder, Trades);
    if (NewOrder->GetTIF() == TimeInForce::GTC and NewOrder->GetQuantity() > 0) {
        OrderBook_.AddOrder(NewOrder);
    }
}

void MatchingEngine::ProcessBuyMarket(std::shared_ptr<Order> NewOrder, std::vector<std::shared_ptr<Trade>>&Trades) {

    if (NewOrder->GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityAsks(NewOrder->GetQuantity(),NewOrder->GetPrice())) {
            MatchOrderBid(NewOrder, Trades);
        }
        return;
    }

    MatchOrderBid(NewOrder, Trades);

}

void MatchingEngine::ProcessSellLimit(std::shared_ptr<Order> NewOrder,std::vector<std::shared_ptr<Trade>>&Trades) {


    if (NewOrder->GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityBids(NewOrder->GetQuantity(),NewOrder->GetPrice())) {
            MatchOrderAsk(NewOrder, Trades);
        }
        return;
    }

    MatchOrderAsk(NewOrder, Trades);
    if (NewOrder->GetTIF() == TimeInForce::GTC and NewOrder->GetQuantity() > 0) {
        OrderBook_.AddOrder(NewOrder);
    }

}

void MatchingEngine::ProcessSellMarket(std::shared_ptr<Order> NewOrder,std::vector<std::shared_ptr<Trade>>&Trades) {


    if (NewOrder->GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityBids(NewOrder->GetQuantity(),NewOrder->GetPrice())) {
            MatchOrderAsk(NewOrder, Trades);
        }
        return;

    }

    MatchOrderAsk(NewOrder, Trades);

}


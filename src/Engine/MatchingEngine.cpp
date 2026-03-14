//
// Created by denni on 3/11/2026.
//
#include "Engine/MatchingEngine.h"


std::vector<Trade> MatchingEngine::ProcessOrder(Order &NewOrder) {

    OrderType Type=NewOrder.GetType();
    OrderSide Side=NewOrder.GetSide();

    if (Side == OrderSide::BUY) {

        if (Type == OrderType::LIMIT) {
            return ProcessBuyLimit(NewOrder);
        }
        else {
            return ProcessBuyMarket(NewOrder);
        }
    }
    else  {
        if (Type == OrderType::LIMIT) {
            return ProcessSellLimit(NewOrder);
        }
        else  {
            return ProcessSellMarket(NewOrder);
        }
    }
}

void MatchingEngine::MatchOrderBid(Order &NewOrder, std::vector<Trade> &Trades) {
    
    while (!OrderBook_.IsAskEmpty()
          and NewOrder.GetQuantity() > 0
          and (NewOrder.GetPrice() >= OrderBook_.GetBestAsk()->GetPrice()
              or NewOrder.GetType() == OrderType::MARKET)) {
        auto now = std::chrono::system_clock::now();
        int Quantity = std::min(NewOrder.GetQuantity(), OrderBook_.GetBestAsk()->GetQuantity());
        Trade NewTrade = Trade(OrderBook_.GetBestAsk()->GetTraderID(),
                               NewOrder.GetTraderID(),
                               OrderBook_.GetBestAsk()->GetPrice(),
                               Quantity,
                               now
        );
        Trades.push_back(NewTrade);
        HistoryTrades_.push_back(NewTrade);
        if (HistoryTrades_.size() > 100) {
            HistoryTrades_.pop_front();
        }

        NewOrder.SetQuantity(NewOrder.GetQuantity() - Quantity);
        if (OrderBook_.GetBestAsk()->GetQuantity() - Quantity == 0) {
            OrderBook_.PopBestAsk();
        } else {
            OrderBook_.UpdateQuantity(OrderBook_.GetBestAsk()->GetOrderID(),
                                      OrderBook_.GetBestAsk()->GetQuantity() - Quantity);
        }
    }
}

void MatchingEngine::MatchOrderAsk(Order &NewOrder, std::vector<Trade> &Trades) {

    while (!OrderBook_.IsBidEmpty()
           and NewOrder.GetQuantity() > 0
           and (NewOrder.GetPrice() <= OrderBook_.GetBestBid()->GetPrice()
               or NewOrder.GetType() == OrderType::MARKET)) {
        auto now = std::chrono::system_clock::now();
        int Quantity = std::min(NewOrder.GetQuantity(), OrderBook_.GetBestBid()->GetQuantity());
        Trade NewTrade = Trade(OrderBook_.GetBestBid()->GetTraderID(),
                               NewOrder.GetTraderID(),
                               OrderBook_.GetBestBid()->GetPrice(),
                               Quantity,
                               now
        );
        Trades.push_back(NewTrade);
        HistoryTrades_.push_back(NewTrade);
        if (HistoryTrades_.size() > 100) {
            HistoryTrades_.pop_front();
        }

        NewOrder.SetQuantity(NewOrder.GetQuantity() - Quantity);
        if (OrderBook_.GetBestBid()->GetQuantity() - Quantity == 0) {
            OrderBook_.PopBestBid();
        } else {
            OrderBook_.UpdateQuantity(OrderBook_.GetBestBid()->GetOrderID(),
                                      OrderBook_.GetBestBid()->GetQuantity() - Quantity);
        }
    }
}

std::vector<Trade> MatchingEngine::ProcessBuyLimit(Order &NewOrder) {
    std::vector<Trade>Trades;

    if (NewOrder.GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityAsks(NewOrder.GetQuantity(),NewOrder.GetPrice())) {
            MatchOrderBid(NewOrder, Trades);
        }
        return Trades;
    }

    MatchOrderBid(NewOrder, Trades);
    if (NewOrder.GetTIF() == TimeInForce::GTC and NewOrder.GetQuantity() > 0) {
        OrderBook_.AddOrder(NewOrder);
    }
    return Trades;
}

std::vector<Trade> MatchingEngine::ProcessBuyMarket(Order &NewOrder) {
    std::vector<Trade>Trades;

    if (NewOrder.GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityAsks(NewOrder.GetQuantity(),NewOrder.GetPrice())) {
            MatchOrderBid(NewOrder, Trades);
        }
        return Trades;
    }

    MatchOrderBid(NewOrder, Trades);

    return Trades;
}

std::vector<Trade> MatchingEngine::ProcessSellLimit(Order &NewOrder) {
    std::vector<Trade>Trades;

    if (NewOrder.GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityBids(NewOrder.GetQuantity(),NewOrder.GetPrice())) {
            MatchOrderAsk(NewOrder, Trades);
        }
        return Trades;
    }

    MatchOrderAsk(NewOrder, Trades);
    if (NewOrder.GetTIF() == TimeInForce::GTC and NewOrder.GetQuantity() > 0) {
        OrderBook_.AddOrder(NewOrder);
    }
    return Trades;

}

std::vector<Trade> MatchingEngine::ProcessSellMarket(Order &NewOrder) {
    std::vector<Trade>Trades;

    if (NewOrder.GetTIF()==TimeInForce::FOK) {
        if (OrderBook_.CanFillQuantityBids(NewOrder.GetQuantity(),NewOrder.GetPrice())) {
            MatchOrderAsk(NewOrder, Trades);
        }
        return Trades;
    }

    MatchOrderAsk(NewOrder, Trades);

    return Trades;
}


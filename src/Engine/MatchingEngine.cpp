//
// Created by denni on 3/11/2026.
//
#include "Engine/MatchingEngine.h"


IOrderBook& MatchingEngine::GetOrderBook(const std::string& Symbol) {
    auto it = OrderBook_->find(Symbol);

    // Dacă simbolul nu există în map
    if (it == OrderBook_->end()) {
        (*OrderBook_)[Symbol] = std::make_unique<MultisetOrderBook>();
        return *(*OrderBook_)[Symbol];
    }

    return *(it->second);
}

std::vector<Trade> MatchingEngine::ProcessOrder(Order &NewOrder) {

    OrderType Type=NewOrder.GetType();
    OrderSide Side=NewOrder.GetSide();

    if (Side == OrderSide::BUY) {

        if (Type == OrderType::LIMIT) {
            return ProcessBuyLimit(NewOrder);
        }

        if (Type == OrderType::MARKET) {
            return ProcessBuyMarket(NewOrder);
        }
    }
    else if (Side == OrderSide::SELL) {
        if (Type == OrderType::LIMIT) {
            return ProcessSellLimit(NewOrder);
        }
        if (Type == OrderType::MARKET) {
            return ProcessSellMarket(NewOrder);
        }
    }
}

void MatchingEngine::MatchOrderBid(Order &NewOrder, std::vector<Trade> &Trades) {
    auto &Book_=(*OrderBook_)[NewOrder.GetSymbol()];
    while (!Book_->IsAskEmpty()
          and NewOrder.GetQuantity() > 0
          and (NewOrder.GetPrice() >= Book_->GetBestAsk()->GetPrice()
              or NewOrder.GetType() == OrderType::MARKET)) {
        auto now = std::chrono::system_clock::now();
        int Quantity = std::min(NewOrder.GetQuantity(), Book_->GetBestAsk()->GetQuantity());
        Trade NewTrade = Trade(Book_->GetBestAsk()->GetTraderID(),
                               NewOrder.GetTraderID(),
                               Book_->GetBestAsk()->GetPrice(),
                               Quantity,
                               now
        );
        Trades.push_back(NewTrade);
        HistoryTrades_.push_back(NewTrade);
        if (HistoryTrades_.size() > 100) {
            HistoryTrades_.pop_front();
        }

        NewOrder.SetQuantity(NewOrder.GetQuantity() - Quantity);
        if (Book_->GetBestAsk()->GetQuantity() - Quantity == 0) {
            Book_->PopBestAsk();
        } else {
            Book_->UpdateQuantity(Book_->GetBestAsk()->GetOrderID(),
                                      Book_->GetBestAsk()->GetQuantity() - Quantity);
        }
    }
}

void MatchingEngine::MatchOrderAsk(Order &NewOrder, std::vector<Trade> &Trades) {
    auto &Book_=(*OrderBook_)[NewOrder.GetSymbol()];
    while (!Book_->IsBidEmpty()
           and NewOrder.GetQuantity() > 0
           and (NewOrder.GetPrice() <= Book_->GetBestBid()->GetPrice()
               or NewOrder.GetType() == OrderType::MARKET)) {
        auto now = std::chrono::system_clock::now();
        int Quantity = std::min(NewOrder.GetQuantity(), Book_->GetBestBid()->GetQuantity());
        Trade NewTrade = Trade(Book_->GetBestBid()->GetTraderID(),
                               NewOrder.GetTraderID(),
                               Book_->GetBestBid()->GetPrice(),
                               Quantity,
                               now
        );
        Trades.push_back(NewTrade);
        HistoryTrades_.push_back(NewTrade);
        if (HistoryTrades_.size() > 100) {
            HistoryTrades_.pop_front();
        }

        NewOrder.SetQuantity(NewOrder.GetQuantity() - Quantity);
        if (Book_->GetBestBid()->GetQuantity() - Quantity == 0) {
            Book_->PopBestBid();
        } else {
            Book_->UpdateQuantity(Book_->GetBestBid()->GetOrderID(),
                                      Book_->GetBestBid()->GetQuantity() - Quantity);
        }
    }
}

std::vector<Trade> MatchingEngine::ProcessBuyLimit(Order &NewOrder) {
    std::vector<Trade>Trades;
    auto &Book_=(*OrderBook_)[NewOrder.GetSymbol()];
    if (NewOrder.GetTIF()==TimeInForce::FOK) {
        if (Book_->CanFillQuantityAsks(NewOrder.GetQuantity(),NewOrder.GetPrice())) {
            MatchOrderBid(NewOrder, Trades);
        }
        return Trades;
    }

    MatchOrderBid(NewOrder, Trades);
    if (NewOrder.GetTIF() == TimeInForce::GTC and NewOrder.GetQuantity() > 0) {
        Book_->AddOrder(NewOrder);
    }
    return Trades;
}

std::vector<Trade> MatchingEngine::ProcessBuyMarket(Order &NewOrder) {
    std::vector<Trade>Trades;
    auto &Book_=(*OrderBook_)[NewOrder.GetSymbol()];
    if (NewOrder.GetTIF()==TimeInForce::FOK) {
        if (Book_->CanFillQuantityAsks(NewOrder.GetQuantity(),NewOrder.GetPrice())) {
            MatchOrderBid(NewOrder, Trades);
        }
        return Trades;
    }

    MatchOrderBid(NewOrder, Trades);

    return Trades;
}

std::vector<Trade> MatchingEngine::ProcessSellLimit(Order &NewOrder) {
    std::vector<Trade>Trades;
    auto &Book_=(*OrderBook_)[NewOrder.GetSymbol()];
    if (NewOrder.GetTIF()==TimeInForce::FOK) {
        if (Book_->CanFillQuantityBids(NewOrder.GetQuantity(),NewOrder.GetPrice())) {
            MatchOrderAsk(NewOrder, Trades);
        }
        return Trades;
    }

    MatchOrderAsk(NewOrder, Trades);
    if (NewOrder.GetTIF() == TimeInForce::GTC and NewOrder.GetQuantity() > 0) {
        Book_->AddOrder(NewOrder);
    }
    return Trades;

}

std::vector<Trade> MatchingEngine::ProcessSellMarket(Order &NewOrder) {
    std::vector<Trade>Trades;
    auto &Book_=(*OrderBook_)[NewOrder.GetSymbol()];
    if (NewOrder.GetTIF()==TimeInForce::FOK) {
        if (Book_->CanFillQuantityBids(NewOrder.GetQuantity(),NewOrder.GetPrice())) {
            MatchOrderAsk(NewOrder, Trades);
        }
        return Trades;
    }

    MatchOrderAsk(NewOrder, Trades);

    return Trades;
}


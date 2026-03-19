//
// Created by denni on 3/19/2026.
//
//
// AppEngineTest.cpp
// Tests for AppEngine — consistent with MatchingEngineTest conventions
//

#include <gtest/gtest.h>
#include "../../include/Engine/AppEngine.h"
#include "../../include/Domain/order.h"
#include "../../include/Domain/trade.h"
#include <chrono>

// ─────────────────────────────────────────────
// Fixture
// ─────────────────────────────────────────────

class AppEngineTest : public ::testing::Test {
protected:
    AppEngine engine;
    std::chrono::system_clock::time_point Now = std::chrono::system_clock::now();

    std::shared_ptr<Order> MakeBuy(int id, uint64_t price, int qty,
                                   const std::string& symbol = "BTCUSD",
                                   OrderType type = OrderType::LIMIT,
                                   TimeInForce tif = TimeInForce::GTC) {
        return std::make_shared<Order>(id, id * 10, OrderSide::BUY, type, symbol,
                                       price, qty, Now, tif, OrderStatus::NEW);
    }

    std::shared_ptr<Order> MakeSell(int id, uint64_t price, int qty,
                                    const std::string& symbol = "BTCUSD",
                                    OrderType type = OrderType::LIMIT,
                                    TimeInForce tif = TimeInForce::GTC) {
        return std::make_shared<Order>(id, id * 10, OrderSide::SELL, type, symbol,
                                       price, qty, Now, tif, OrderStatus::NEW);
    }
};

// ═════════════════════════════════════════════
// GetTradesHistory — simbol necunoscut
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, GetTradesHistory_UnknownSymbol_ReturnsEmpty) {
    auto trades = engine.GetTradesHistory("NOPE");
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// AddOrder — creare engine la simbol nou
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_NewSymbol_NoMatch_NoTrades) {
    engine.AddOrder(MakeBuy(1, 100, 10));
    auto trades = engine.GetTradesHistory("BTCUSD");
    EXPECT_TRUE(trades.empty());
}

TEST_F(AppEngineTest, AddOrder_NewSymbol_DoesNotCrash) {
    EXPECT_NO_THROW(engine.AddOrder(MakeBuy(1, 100, 10)));
}

// ═════════════════════════════════════════════
// AddOrder — match complet
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_FullMatch_TradeRecordedInHistory) {
    engine.AddOrder(MakeSell(1, 100, 10));
    engine.AddOrder(MakeBuy(2, 100, 10));

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
    EXPECT_EQ(trades[0]->GetPrice(), 100u);
}

TEST_F(AppEngineTest, AddOrder_FullMatch_CorrectMakerTakerID) {
    engine.AddOrder(MakeSell(1, 100, 10)); // MakerID = 1*10 = 10
    engine.AddOrder(MakeBuy(2, 100, 10));  // TakerID = 2*10 = 20

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetMakerID(), 10);
    EXPECT_EQ(trades[0]->GetTakerID(), 20);
}

// ═════════════════════════════════════════════
// AddOrder — match parțial
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_PartialMatch_CorrectTradeQuantity) {
    engine.AddOrder(MakeSell(1, 100, 5));
    engine.AddOrder(MakeBuy(2, 100, 10));

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 5);
}

// ═════════════════════════════════════════════
// AddOrder — fără match (prețuri incompatibile)
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_IncompatiblePrices_NoTradeRecorded) {
    engine.AddOrder(MakeBuy(1, 90, 10));
    engine.AddOrder(MakeSell(2, 100, 10));

    auto trades = engine.GetTradesHistory("BTCUSD");
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// AddOrder — multiple trade-uri
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_MultipleMatches_AllQuantitiesRecorded) {
    engine.AddOrder(MakeBuy(1, 100, 5));
    engine.AddOrder(MakeBuy(2, 100, 5));
    engine.AddOrder(MakeSell(3, 100, 10));

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 2u);

    int totalQty = 0;
    for (const auto& t : trades) totalQty += t->GetQuantity();
    EXPECT_EQ(totalQty, 10);
}

// ═════════════════════════════════════════════
// AddOrder — simboluri diferite sunt izolate
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_DifferentSymbols_HistoriesAreIsolated) {
    engine.AddOrder(MakeSell(1, 100, 10, "BTCUSD"));
    engine.AddOrder(MakeBuy(2, 100, 10, "BTCUSD"));
    engine.AddOrder(MakeBuy(3, 200, 5, "ETHUSD"));

    auto btcTrades = engine.GetTradesHistory("BTCUSD");
    auto ethTrades = engine.GetTradesHistory("ETHUSD");

    EXPECT_EQ(btcTrades.size(), 1u);
    EXPECT_TRUE(ethTrades.empty());
}

TEST_F(AppEngineTest, AddOrder_DifferentSymbols_BothMatch_CorrectHistory) {
    engine.AddOrder(MakeSell(1, 100, 10, "BTCUSD"));
    engine.AddOrder(MakeBuy(2, 100, 10, "BTCUSD"));
    engine.AddOrder(MakeSell(3, 200, 5, "ETHUSD"));
    engine.AddOrder(MakeBuy(4, 200, 5, "ETHUSD"));

    auto btcTrades = engine.GetTradesHistory("BTCUSD");
    auto ethTrades = engine.GetTradesHistory("ETHUSD");

    ASSERT_EQ(btcTrades.size(), 1u);
    ASSERT_EQ(ethTrades.size(), 1u);
    EXPECT_EQ(btcTrades[0]->GetQuantity(), 10);
    EXPECT_EQ(ethTrades[0]->GetQuantity(), 5);
}

// ═════════════════════════════════════════════
// AddOrder — prețuri crossing (buy > sell)
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_CrossingPrices_TradeAtMakerPrice) {
    engine.AddOrder(MakeBuy(1, 110, 10));  // resting bid la 110
    engine.AddOrder(MakeSell(2, 100, 10)); // incoming sell la 100 → match la 110

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetPrice(), 110u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

// ═════════════════════════════════════════════
// AddOrder — Market orders prin AppEngine
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_MarketBuy_MatchesRestingAsk) {
    engine.AddOrder(MakeSell(1, 100, 10));
    engine.AddOrder(MakeBuy(2, 0, 10, "BTCUSD", OrderType::MARKET));

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(AppEngineTest, AddOrder_MarketSell_MatchesRestingBid) {
    engine.AddOrder(MakeBuy(1, 100, 10));
    engine.AddOrder(MakeSell(2, 0, 10, "BTCUSD", OrderType::MARKET));

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(AppEngineTest, AddOrder_MarketBuy_NoAsks_NoTrades) {
    engine.AddOrder(MakeBuy(1, 0, 10, "BTCUSD", OrderType::MARKET));
    auto trades = engine.GetTradesHistory("BTCUSD");
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// AddOrder — IOC prin AppEngine
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_IOC_PartialMatch_RemainderDiscarded) {
    engine.AddOrder(MakeSell(1, 100, 5));
    engine.AddOrder(MakeBuy(2, 100, 10, "BTCUSD", OrderType::LIMIT, TimeInForce::IOC));

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 5);
}

TEST_F(AppEngineTest, AddOrder_IOC_NoMatch_NoTradeNoRemainder) {
    engine.AddOrder(MakeBuy(1, 100, 10, "BTCUSD", OrderType::LIMIT, TimeInForce::IOC));
    auto trades = engine.GetTradesHistory("BTCUSD");
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// AddOrder — FOK prin AppEngine
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, AddOrder_FOK_CanFill_TradeExecuted) {
    engine.AddOrder(MakeSell(1, 100, 10));
    engine.AddOrder(MakeBuy(2, 100, 10, "BTCUSD", OrderType::LIMIT, TimeInForce::FOK));

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(AppEngineTest, AddOrder_FOK_CannotFill_NoTrades) {
    engine.AddOrder(MakeSell(1, 100, 5));
    engine.AddOrder(MakeBuy(2, 100, 10, "BTCUSD", OrderType::LIMIT, TimeInForce::FOK));

    auto trades = engine.GetTradesHistory("BTCUSD");
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// CancelOrder
// ═════════════════════════════════════════════

TEST_F(AppEngineTest, CancelOrder_CancelledBid_DoesNotMatch) {
    engine.AddOrder(MakeBuy(1, 100, 10));
    engine.CancelOrder(1, "BTCUSD");

    engine.AddOrder(MakeSell(2, 100, 10));
    auto trades = engine.GetTradesHistory("BTCUSD");
    EXPECT_TRUE(trades.empty());
}

TEST_F(AppEngineTest, CancelOrder_CancelledAsk_DoesNotMatch) {
    engine.AddOrder(MakeSell(1, 100, 10));
    engine.CancelOrder(1, "BTCUSD");

    engine.AddOrder(MakeBuy(2, 100, 10));
    auto trades = engine.GetTradesHistory("BTCUSD");
    EXPECT_TRUE(trades.empty());
}

TEST_F(AppEngineTest, CancelOrder_UnknownSymbol_NoCrash) {
    EXPECT_NO_THROW(engine.CancelOrder(999, "NOPE"));
}

TEST_F(AppEngineTest, CancelOrder_UnknownOrderID_NoCrash) {
    engine.AddOrder(MakeBuy(1, 100, 10));
    EXPECT_NO_THROW(engine.CancelOrder(9999, "BTCUSD"));
}
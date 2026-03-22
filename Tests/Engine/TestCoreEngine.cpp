//
// Created by denni on 3/19/2026.
//
//
// CoreEngineTest.cpp
// Tests for CoreEngine using Google Test
//

#include <gtest/gtest.h>
#include "../../include/Engine/CoreEngine.h"
#include "../../include/Domain/order.h"
#include "../../include/Domain/trade.h"

// ─────────────────────────────────────────────
// Fixture
// ─────────────────────────────────────────────

class CoreEngineTest : public ::testing::Test {
protected:
    CoreEngine engine;

    // Helpers — parametrii ca string, exact cum îi trimite UI-ul
    void Buy(uint64_t price, int qty, const std::string& symbol = "BTCUSD",
             const std::string& type = "LIMIT", const std::string& tif = "GTC",
             int traderID = 1) {
        engine.CreateOrder(price, qty, type, symbol, tif, traderID, "BUY");
    }

    void Sell(uint64_t price, int qty, const std::string& symbol = "BTCUSD",
              const std::string& type = "LIMIT", const std::string& tif = "GTC",
              int traderID = 1) {
        engine.CreateOrder(price, qty, type, symbol, tif, traderID, "SELL");
    }
};

// ═════════════════════════════════════════════
// GetTradesHistory — simbol necunoscut
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, GetTradesHistory_UnknownSymbol_ReturnsEmpty) {
    auto trades = engine.GetTradesHistory("NOPE");
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// GetOrders
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, GetOrders_InitiallyEmpty) {
    EXPECT_TRUE(engine.GetOrders("BTCUSD").empty());
}

TEST_F(CoreEngineTest, GetOrders_AfterCreateOrder_ContainsOrder) {
    Buy(100, 10);
    EXPECT_EQ(engine.GetOrders("BTCUSD").size(), 1u);
}

TEST_F(CoreEngineTest, GetOrders_MultipleOrders_AllStored) {
    Buy(100, 10);
    Sell(100, 10);
    Buy(90, 5);
    EXPECT_EQ(engine.GetOrders("BTCUSD").size(), 3u);
}

// ═════════════════════════════════════════════
// CreateOrder — GenerateID unic
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_GeneratesUniqueOrderIDs) {
    Buy(100, 10);
    Buy(100, 5);
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 2u);
    EXPECT_NE(orders[0]->GetOrderID(), orders[1]->GetOrderID());
}

// ═════════════════════════════════════════════
// CreateOrder — Price * Tick
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_PriceMultipliedByTick) {
    Buy(100, 10); // Price intern = 100 * 100 = 10000
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetPrice(), 100u * 100u);
}

// ═════════════════════════════════════════════
// CreateOrder — câmpuri corecte pe Order
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_CorrectSide_Buy) {
    Buy(100, 10);
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetSide(), OrderSide::BUY);
}

TEST_F(CoreEngineTest, CreateOrder_CorrectSide_Sell) {
    Sell(100, 10);
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetSide(), OrderSide::SELL);
}

TEST_F(CoreEngineTest, CreateOrder_CorrectSymbol) {
    Buy(100, 10, "ETHUSD");
    auto orders = engine.GetOrders("ETHUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetSymbol(), "ETHUSD");
}

TEST_F(CoreEngineTest, CreateOrder_CorrectQuantity) {
    Buy(100, 7);
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetQuantity(), 7);
}

TEST_F(CoreEngineTest, CreateOrder_CorrectTraderID) {
    engine.CreateOrder(100, 10, "LIMIT", "BTCUSD", "GTC", 42, "BUY");
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetTraderID(), 42);
}

TEST_F(CoreEngineTest, CreateOrder_CorrectType_Market) {
    Buy(0, 10, "BTCUSD", "MARKET");
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetType(), OrderType::MARKET);
}

TEST_F(CoreEngineTest, CreateOrder_CorrectTIF_IOC) {
    Buy(100, 10, "BTCUSD", "LIMIT", "IOC");
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetTIF(), TimeInForce::IOC);
}

TEST_F(CoreEngineTest, CreateOrder_CorrectTIF_FOK) {
    Buy(100, 10, "BTCUSD", "LIMIT", "FOK");
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetTIF(), TimeInForce::FOK);
}

TEST_F(CoreEngineTest, CreateOrder_StatusIsNew) {
    Buy(100, 10);
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0]->GetStatus(), OrderStatus::NEW);
}

// ═════════════════════════════════════════════
// Match complet
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_FullMatch_TradeRecorded) {
    Sell(100, 10);
    Buy(100, 10);

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
    EXPECT_EQ(trades[0]->GetPrice(), 100u * 100u);
}

// ═════════════════════════════════════════════
// Match parțial
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_PartialMatch_CorrectTradeQuantity) {
    Sell(100, 5);
    Buy(100, 10);

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 5);
}

// ═════════════════════════════════════════════
// Fără match
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_IncompatiblePrices_NoTrade) {
    Buy(90, 10);
    Sell(100, 10);

    auto trades = engine.GetTradesHistory("BTCUSD");
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// Simboluri diferite izolate
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_DifferentSymbols_HistoriesIsolated) {
    Sell(100, 10, "BTCUSD");
    Buy(100, 10, "BTCUSD");
    Buy(200, 5, "ETHUSD");

    EXPECT_EQ(engine.GetTradesHistory("BTCUSD").size(), 1u);
    EXPECT_TRUE(engine.GetTradesHistory("ETHUSD").empty());
}

// ═════════════════════════════════════════════
// Market order
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_MarketBuy_MatchesRestingAsk) {
    Sell(100, 10);
    Buy(0, 10, "BTCUSD", "MARKET");

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(CoreEngineTest, CreateOrder_MarketSell_MatchesRestingBid) {
    Buy(100, 10);
    Sell(0, 10, "BTCUSD", "MARKET");

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(CoreEngineTest, CreateOrder_MarketBuy_NoAsks_NoTrades) {
    Buy(0, 10, "BTCUSD", "MARKET");
    EXPECT_TRUE(engine.GetTradesHistory("BTCUSD").empty());
}

// ═════════════════════════════════════════════
// IOC
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_IOC_PartialMatch_RemainderDiscarded) {
    Sell(100, 5);
    Buy(100, 10, "BTCUSD", "LIMIT", "IOC");

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 5);
}

TEST_F(CoreEngineTest, CreateOrder_IOC_NoMatch_NoTrade) {
    Buy(100, 10, "BTCUSD", "LIMIT", "IOC");
    EXPECT_TRUE(engine.GetTradesHistory("BTCUSD").empty());
}

// ═════════════════════════════════════════════
// FOK
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_FOK_CanFill_TradeExecuted) {
    Sell(100, 10);
    Buy(100, 10, "BTCUSD", "LIMIT", "FOK");

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(CoreEngineTest, CreateOrder_FOK_CannotFill_NoTrades) {
    Sell(100, 5);
    Buy(100, 10, "BTCUSD", "LIMIT", "FOK");

    EXPECT_TRUE(engine.GetTradesHistory("BTCUSD").empty());
}

// ═════════════════════════════════════════════
// CancelOrder
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CancelOrder_CancelledBid_DoesNotMatch) {
    Buy(100, 10);
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    int orderId = orders[0]->GetOrderID();

    engine.CancelOrder(orderId, "BTCUSD");
    Sell(100, 10);

    EXPECT_TRUE(engine.GetTradesHistory("BTCUSD").empty());
}

TEST_F(CoreEngineTest, CancelOrder_CancelledAsk_DoesNotMatch) {
    Sell(100, 10);
    auto orders = engine.GetOrders("BTCUSD");
    ASSERT_EQ(orders.size(), 1u);
    int orderId = orders[0]->GetOrderID();

    engine.CancelOrder(orderId, "BTCUSD");
    Buy(100, 10);

    EXPECT_TRUE(engine.GetTradesHistory("BTCUSD").empty());
}

TEST_F(CoreEngineTest, CancelOrder_UnknownSymbol_NoCrash) {
    EXPECT_NO_THROW(engine.CancelOrder(999, "NOPE"));
}

TEST_F(CoreEngineTest, CancelOrder_UnknownOrderID_NoCrash) {
    Buy(100, 10);
    EXPECT_NO_THROW(engine.CancelOrder(9999, "BTCUSD"));
}

// ═════════════════════════════════════════════
// Crossing prices — trade la prețul maker-ului
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, CreateOrder_CrossingPrices_TradeAtMakerPrice) {
    Buy(110, 10);           // resting bid la 110*100
    Sell(100, 10);          // incoming sell la 100*100 → match la 110*100

    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetPrice(), 110u * 100u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

// ═════════════════════════════════════════════
// GetBestBids
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, GetBestBids_UnknownSymbol_ReturnsEmpty) {
    std::string sym = "NOPE";
    EXPECT_TRUE(engine.GetBestBids(5, sym).empty());
}

TEST_F(CoreEngineTest, GetBestBids_XZero_ReturnsEmpty) {
    Buy(100, 10);
    std::string sym = "BTCUSD";
    EXPECT_TRUE(engine.GetBestBids(0, sym).empty());
}

TEST_F(CoreEngineTest, GetBestBids_NoOrdersForSymbol_ReturnsEmpty) {
    Buy(100, 10, "BTCUSD");
    std::string sym = "ETHUSD";
    EXPECT_TRUE(engine.GetBestBids(5, sym).empty());
}

TEST_F(CoreEngineTest, GetBestBids_SingleOrder_ReturnedCorrectly) {
    Buy(100, 10);
    std::string sym = "BTCUSD";
    auto result = engine.GetBestBids(5, sym);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->GetPrice(), 100u * 100u); // Price * Tick
    EXPECT_EQ(result[0]->GetQuantity(), 10);
}

TEST_F(CoreEngineTest, GetBestBids_SortedDescending_HighestPriceFirst) {
    Buy(100, 10);
    Buy(105, 10);
    Buy(103, 10);
    std::string sym = "BTCUSD";
    auto result = engine.GetBestBids(3, sym);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0]->GetPrice(), 105u * 100u);
    EXPECT_EQ(result[1]->GetPrice(), 103u * 100u);
    EXPECT_EQ(result[2]->GetPrice(), 100u * 100u);
}

TEST_F(CoreEngineTest, GetBestBids_XLargerThanBook_ReturnsAll) {
    Buy(100, 10);
    Buy(105, 10);
    std::string sym = "BTCUSD";
    auto result = engine.GetBestBids(10, sym);
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(CoreEngineTest, GetBestBids_ExactX_ReturnsX) {
    Buy(100, 10);
    Buy(105, 10);
    Buy(103, 10);
    std::string sym = "BTCUSD";
    auto result = engine.GetBestBids(2, sym);
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(CoreEngineTest, GetBestBids_DoesNotModifyBook) {
    Buy(100, 10);
    Buy(105, 10);
    std::string sym = "BTCUSD";
    engine.GetBestBids(2, sym);
    // book intact — un sell la 105 tot ar trebui să matcheze
    Sell(105, 10);
    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(CoreEngineTest, GetBestBids_DifferentSymbols_IsolatedResults) {
    Buy(100, 10, "BTCUSD");
    Buy(200, 5,  "ETHUSD");
    std::string btc = "BTCUSD";
    std::string eth = "ETHUSD";
    auto btcBids = engine.GetBestBids(5, btc);
    auto ethBids = engine.GetBestBids(5, eth);
    ASSERT_EQ(btcBids.size(), 1u);
    ASSERT_EQ(ethBids.size(), 1u);
    EXPECT_EQ(btcBids[0]->GetPrice(), 100u * 100u);
    EXPECT_EQ(ethBids[0]->GetPrice(), 200u * 100u);
}

TEST_F(CoreEngineTest, GetBestBids_AfterCancelBestBid_ReturnsNext) {
    Buy(105, 10);
    Buy(100, 10);
    auto orders = engine.GetOrders("BTCUSD");
    // primul ordin adăugat e cel cu 105 → id-ul lui
    int orderId = orders[0]->GetOrderID();
    engine.CancelOrder(orderId, "BTCUSD");
    std::string sym = "BTCUSD";
    auto result = engine.GetBestBids(5, sym);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->GetPrice(), 100u * 100u);
}

// ═════════════════════════════════════════════
// GetBestAsks
// ═════════════════════════════════════════════

TEST_F(CoreEngineTest, GetBestAsks_UnknownSymbol_ReturnsEmpty) {
    std::string sym = "NOPE";
    EXPECT_TRUE(engine.GetBestAsks(5, sym).empty());
}

TEST_F(CoreEngineTest, GetBestAsks_XZero_ReturnsEmpty) {
    Sell(100, 5);
    std::string sym = "BTCUSD";
    EXPECT_TRUE(engine.GetBestAsks(0, sym).empty());
}

TEST_F(CoreEngineTest, GetBestAsks_NoOrdersForSymbol_ReturnsEmpty) {
    Sell(100, 10, "BTCUSD");
    std::string sym = "ETHUSD";
    EXPECT_TRUE(engine.GetBestAsks(5, sym).empty());
}

TEST_F(CoreEngineTest, GetBestAsks_SingleOrder_ReturnedCorrectly) {
    Sell(100, 5);
    std::string sym = "BTCUSD";
    auto result = engine.GetBestAsks(5, sym);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->GetPrice(), 100u * 100u); // Price * Tick
    EXPECT_EQ(result[0]->GetQuantity(), 5);
}

TEST_F(CoreEngineTest, GetBestAsks_SortedAscending_LowestPriceFirst) {
    Sell(107, 5);
    Sell(100, 5);
    Sell(103, 5);
    std::string sym = "BTCUSD";
    auto result = engine.GetBestAsks(3, sym);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0]->GetPrice(), 100u * 100u);
    EXPECT_EQ(result[1]->GetPrice(), 103u * 100u);
    EXPECT_EQ(result[2]->GetPrice(), 107u * 100u);
}

TEST_F(CoreEngineTest, GetBestAsks_XLargerThanBook_ReturnsAll) {
    Sell(100, 5);
    Sell(105, 5);
    std::string sym = "BTCUSD";
    auto result = engine.GetBestAsks(10, sym);
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(CoreEngineTest, GetBestAsks_ExactX_ReturnsX) {
    Sell(100, 5);
    Sell(103, 5);
    Sell(107, 5);
    std::string sym = "BTCUSD";
    auto result = engine.GetBestAsks(2, sym);
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(CoreEngineTest, GetBestAsks_DoesNotModifyBook) {
    Sell(100, 10);
    Sell(105, 10);
    std::string sym = "BTCUSD";
    engine.GetBestAsks(2, sym);
    // book intact — un buy la 100 tot ar trebui să matcheze
    Buy(100, 10);
    auto trades = engine.GetTradesHistory("BTCUSD");
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(CoreEngineTest, GetBestAsks_DifferentSymbols_IsolatedResults) {
    Sell(100, 10, "BTCUSD");
    Sell(200, 5,  "ETHUSD");
    std::string btc = "BTCUSD";
    std::string eth = "ETHUSD";
    auto btcAsks = engine.GetBestAsks(5, btc);
    auto ethAsks = engine.GetBestAsks(5, eth);
    ASSERT_EQ(btcAsks.size(), 1u);
    ASSERT_EQ(ethAsks.size(), 1u);
    EXPECT_EQ(btcAsks[0]->GetPrice(), 100u * 100u);
    EXPECT_EQ(ethAsks[0]->GetPrice(), 200u * 100u);
}

TEST_F(CoreEngineTest, GetBestAsks_AfterCancelBestAsk_ReturnsNext) {
    Sell(100, 5);
    Sell(105, 5);
    auto orders = engine.GetOrders("BTCUSD");
    // primul ordin adăugat e cel cu 100 → id-ul lui
    int orderId = orders[0]->GetOrderID();
    engine.CancelOrder(orderId, "BTCUSD");
    std::string sym = "BTCUSD";
    auto result = engine.GetBestAsks(5, sym);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->GetPrice(), 105u * 100u);
}
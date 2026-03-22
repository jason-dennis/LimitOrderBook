#include <gtest/gtest.h>
#include "../../include/Engine/MatchingEngine.h"
#include "../../include/Storage/MultisetOrderBookStorage.h"
#include "../../include/Storage/BinaryOrderBookStorage.h"
#include "../../include/Domain/order.h"
#include "../../include/Domain/trade.h"
#include <chrono>
#include <atomic>

// ─────────────────────────────────────────────
// Fixture
// ─────────────────────────────────────────────

class MatchingEngineTest : public ::testing::Test {
protected:
    MultisetOrderBook book;
    std::atomic<int> TradeCounter_{1};
    MatchingEngine engine{book,TradeCounter_};

    std::chrono::system_clock::time_point Now = std::chrono::system_clock::now();

    std::shared_ptr<Order> MakeBuy(int id, uint64_t price, int qty,
                                   OrderType type = OrderType::LIMIT,
                                   TimeInForce tif = TimeInForce::GTC) {
        return std::make_shared<Order>(id, id * 10, OrderSide::BUY, type, "BTCUSD",
                                       price, qty, Now, tif, OrderStatus::NEW);
    }

    std::shared_ptr<Order> MakeSell(int id, uint64_t price, int qty,
                                    OrderType type = OrderType::LIMIT,
                                    TimeInForce tif = TimeInForce::GTC) {
        return std::make_shared<Order>(id, id * 10, OrderSide::SELL, type, "BTCUSD",
                                       price, qty, Now, tif, OrderStatus::NEW);
    }
};

// ═════════════════════════════════════════════
// ProcessBuyLimit — GTC
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, BuyLimit_GTC_NoMatch_AddsToBook) {
    auto buy = MakeBuy(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);
    EXPECT_TRUE(trades.empty());
    EXPECT_FALSE(book.IsBidEmpty());
}

TEST_F(MatchingEngineTest, BuyLimit_GTC_FullMatch_NoRemainder) {
    auto sell = MakeSell(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades); // resting ask

    trades.clear();
    auto buy = MakeBuy(2, 100, 10);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
    EXPECT_TRUE(book.IsBidEmpty());
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MatchingEngineTest, BuyLimit_GTC_PartialMatch_RemainderAddedToBook) {
    auto sell = MakeSell(1, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades); // resting ask of 5

    trades.clear();
    auto buy = MakeBuy(2, 100, 10);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 5);
    // remaining 5 should rest on bid side
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 5);
}

TEST_F(MatchingEngineTest, BuyLimit_GTC_PriceTooLow_NoMatch_AddsToBook) {
    auto sell = MakeSell(1, 110, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 10); // price below ask
    engine.ProcessOrder(buy, trades);
    EXPECT_TRUE(trades.empty());
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_FALSE(book.IsAskEmpty());
}

TEST_F(MatchingEngineTest, BuyLimit_GTC_MultipleAsks_MatchesMultiple) {
    auto sell1 = MakeSell(1, 100, 5);
    auto sell2 = MakeSell(2, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell1, trades);
    engine.ProcessOrder(sell2, trades);

    trades.clear();
    auto buy = MakeBuy(3, 100, 10);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 2u);
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// ProcessBuyLimit — IOC
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, BuyLimit_IOC_PartialMatch_RemainderDiscarded) {
    auto sell = MakeSell(1, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 10, OrderType::LIMIT, TimeInForce::IOC);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 5);
    // remainder NOT added to book for IOC
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MatchingEngineTest, BuyLimit_IOC_NoMatch_NothingAdded) {
    auto buy = MakeBuy(1, 100, 10, OrderType::LIMIT, TimeInForce::IOC);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);
    EXPECT_TRUE(trades.empty());
    EXPECT_TRUE(book.IsBidEmpty());
}

// ═════════════════════════════════════════════
// ProcessBuyLimit — FOK
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, BuyLimit_FOK_CanFill_ExecutesFully) {
    auto sell = MakeSell(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 10, OrderType::LIMIT, TimeInForce::FOK);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(MatchingEngineTest, BuyLimit_FOK_CannotFill_NoTrades) {
    auto sell = MakeSell(1, 100, 5); // only 5 available
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 10, OrderType::LIMIT, TimeInForce::FOK); // needs 10
    engine.ProcessOrder(buy, trades);
    EXPECT_TRUE(trades.empty());
    EXPECT_FALSE(book.IsAskEmpty()); // ask untouched
}

// ═════════════════════════════════════════════
// ProcessBuyMarket
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, BuyMarket_GTC_FullMatch) {
    auto sell = MakeSell(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 0, 10, OrderType::MARKET, TimeInForce::GTC);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MatchingEngineTest, BuyMarket_GTC_NoAsks_NoTrades) {
    auto buy = MakeBuy(1, 0, 10, OrderType::MARKET, TimeInForce::GTC);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);
    EXPECT_TRUE(trades.empty());
}

TEST_F(MatchingEngineTest, BuyMarket_FOK_CanFill_Executes) {
    auto sell = MakeSell(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 10, OrderType::MARKET, TimeInForce::FOK);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
}

TEST_F(MatchingEngineTest, BuyMarket_FOK_CannotFill_NoTrades) {
    auto sell = MakeSell(1, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 10, OrderType::MARKET, TimeInForce::FOK);
    engine.ProcessOrder(buy, trades);
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// ProcessSellLimit — GTC
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, SellLimit_GTC_NoMatch_AddsToBook) {
    auto sell = MakeSell(1, 110, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);
    EXPECT_TRUE(trades.empty());
    EXPECT_FALSE(book.IsAskEmpty());
}

TEST_F(MatchingEngineTest, SellLimit_GTC_FullMatch_NoRemainder) {
    auto buy = MakeBuy(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
    EXPECT_TRUE(book.IsBidEmpty());
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MatchingEngineTest, SellLimit_GTC_PartialMatch_RemainderAddedToBook) {
    auto buy = MakeBuy(1, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 5);
    EXPECT_FALSE(book.IsAskEmpty());
    EXPECT_EQ(book.GetBestAsk()->GetQuantity(), 5);
}

TEST_F(MatchingEngineTest, SellLimit_GTC_PriceTooHigh_NoMatch_AddsToBook) {
    auto buy = MakeBuy(1, 90, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10); // price above bid
    engine.ProcessOrder(sell, trades);
    EXPECT_TRUE(trades.empty());
    EXPECT_FALSE(book.IsAskEmpty());
    EXPECT_FALSE(book.IsBidEmpty());
}

TEST_F(MatchingEngineTest, SellLimit_GTC_MultipleBids_MatchesMultiple) {
    auto buy1 = MakeBuy(1, 100, 5);
    auto buy2 = MakeBuy(2, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy1, trades);
    engine.ProcessOrder(buy2, trades);

    trades.clear();
    auto sell = MakeSell(3, 100, 10);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 2u);
    EXPECT_TRUE(book.IsBidEmpty());
}

// ═════════════════════════════════════════════
// ProcessSellLimit — IOC
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, SellLimit_IOC_PartialMatch_RemainderDiscarded) {
    auto buy = MakeBuy(1, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10, OrderType::LIMIT, TimeInForce::IOC);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_TRUE(book.IsAskEmpty()); // remainder not added
}

TEST_F(MatchingEngineTest, SellLimit_IOC_NoMatch_NothingAdded) {
    auto sell = MakeSell(1, 100, 10, OrderType::LIMIT, TimeInForce::IOC);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);
    EXPECT_TRUE(trades.empty());
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// ProcessSellLimit — FOK
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, SellLimit_FOK_CanFill_ExecutesFully) {
    auto buy = MakeBuy(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10, OrderType::LIMIT, TimeInForce::FOK);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
}

TEST_F(MatchingEngineTest, SellLimit_FOK_CannotFill_NoTrades) {
    auto buy = MakeBuy(1, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10, OrderType::LIMIT, TimeInForce::FOK);
    engine.ProcessOrder(sell, trades);
    EXPECT_TRUE(trades.empty());
    EXPECT_FALSE(book.IsBidEmpty()); // bid untouched
}

// ═════════════════════════════════════════════
// ProcessSellMarket
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, SellMarket_GTC_FullMatch) {
    auto buy = MakeBuy(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 0, 10, OrderType::MARKET, TimeInForce::GTC);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 10);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MatchingEngineTest, SellMarket_GTC_NoBids_NoTrades) {
    auto sell = MakeSell(1, 0, 10, OrderType::MARKET, TimeInForce::GTC);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);
    EXPECT_TRUE(trades.empty());
}

TEST_F(MatchingEngineTest, SellMarket_FOK_CanFill_Executes) {
    auto buy = MakeBuy(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10, OrderType::MARKET, TimeInForce::FOK);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
}

TEST_F(MatchingEngineTest, SellMarket_FOK_CannotFill_NoTrades) {
    auto buy = MakeBuy(1, 100, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10, OrderType::MARKET, TimeInForce::FOK);
    engine.ProcessOrder(sell, trades);
    EXPECT_TRUE(trades.empty());
}

// ═════════════════════════════════════════════
// Trade fields correctness
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, Trade_HasCorrectPrice) {
    auto sell = MakeSell(1, 105, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 105, 10);
    engine.ProcessOrder(buy, trades);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetPrice(), 105u);
}

TEST_F(MatchingEngineTest, Trade_HasCorrectMakerAndTakerID) {
    auto sell = MakeSell(1, 100, 10); // MakerID = 1*10 = 10
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 10);  // TakerID = 2*10 = 20
    engine.ProcessOrder(buy, trades);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetMakerID(), 10);
    EXPECT_EQ(trades[0]->GetTakerID(), 20);
}

TEST_F(MatchingEngineTest, Trade_HasCorrectQuantity_PartialMatch) {
    auto sell = MakeSell(1, 100, 4);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 10);
    engine.ProcessOrder(buy, trades);
    ASSERT_GE(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 4);
}

// ═════════════════════════════════════════════
// HistoryTrades — capped at 100
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, HistoryTrades_CappedAt100) {
    for (int i = 1; i <= 101; i++) {
        auto sell = MakeSell(i, 100, 1);
        std::vector<std::shared_ptr<Trade>> trades;
        engine.ProcessOrder(sell, trades);

        auto buy = MakeBuy(i + 200, 100, 1);
        engine.ProcessOrder(buy, trades);
    }
    auto sell = MakeSell(999, 100, 1);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(1000, 100, 1);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
}

// ═════════════════════════════════════════════
// GetOrderBook
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, GetOrderBook_ReturnsCorrectState) {
    auto buy = MakeBuy(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);
    const auto& ob = engine.GetOrderBook();
    EXPECT_FALSE(ob.IsBidEmpty());
    EXPECT_TRUE(ob.IsAskEmpty());
}

// ═════════════════════════════════════════════
// Integration scenarios
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, Integration_BuyAndSell_CrossingPrices_Match) {
    auto buy = MakeBuy(1, 105, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 10);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_TRUE(book.IsBidEmpty());
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MatchingEngineTest, Integration_MultipleOrders_CorrectMatchOrder) {
    auto buy1 = MakeBuy(1, 100, 5);
    auto buy2 = MakeBuy(2, 105, 5);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy1, trades);
    engine.ProcessOrder(buy2, trades);

    trades.clear();
    auto sell = MakeSell(3, 100, 5);
    engine.ProcessOrder(sell, trades);
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetPrice(), 105u); // matched at best bid
}

TEST_F(MatchingEngineTest, Integration_MarketBuy_IgnoresPrice_MatchesAnyAsk) {
    auto sell = MakeSell(1, 999, 10); // very high ask price
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 0, 10, OrderType::MARKET);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
}

TEST_F(MatchingEngineTest, Integration_MarketSell_IgnoresPrice_MatchesAnyBid) {
    auto buy = MakeBuy(1, 1, 10); // very low bid price
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 0, 10, OrderType::MARKET);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
}

// ═════════════════════════════════════════════
// UpdateQuantity correctness
// ═════════════════════════════════════════════

TEST_F(MatchingEngineTest, SellLimit_GTC_PartialMatch_BidQuantityUpdatedCorrectly) {
    auto buy = MakeBuy(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(buy, trades);

    trades.clear();
    auto sell = MakeSell(2, 100, 4);
    engine.ProcessOrder(sell, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 4);
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 6);
}

TEST_F(MatchingEngineTest, BuyLimit_GTC_PartialMatch_AskQuantityUpdatedCorrectly) {
    auto sell = MakeSell(1, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell, trades);

    trades.clear();
    auto buy = MakeBuy(2, 100, 4);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0]->GetQuantity(), 4);
    EXPECT_FALSE(book.IsAskEmpty());
    EXPECT_EQ(book.GetBestAsk()->GetQuantity(), 6);
}

TEST_F(MatchingEngineTest, BuyLimit_GTC_MultipleAsks_PopAndUpdateInSameOrder) {
    auto sell1 = MakeSell(1, 100, 3);
    auto sell2 = MakeSell(2, 100, 10);
    std::vector<std::shared_ptr<Trade>> trades;
    engine.ProcessOrder(sell1, trades);
    engine.ProcessOrder(sell2, trades);

    trades.clear();
    auto buy = MakeBuy(3, 100, 8);
    engine.ProcessOrder(buy, trades);
    EXPECT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0]->GetQuantity(), 3);
    EXPECT_EQ(trades[1]->GetQuantity(), 5);
    EXPECT_FALSE(book.IsAskEmpty());
    EXPECT_EQ(book.GetBestAsk()->GetQuantity(), 5);
}
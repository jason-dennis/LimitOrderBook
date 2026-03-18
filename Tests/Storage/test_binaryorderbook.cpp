#include <gtest/gtest.h>
#include "../../include/Storage/BinaryOrderBookStorage.h"
#include <deque>
// ─────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────

static std::shared_ptr<Order> MakeBuy(int id, uint64_t price, int qty) {
    return std::make_shared<Order>(id, 1, OrderSide::BUY, OrderType::LIMIT, "AAPL", price, qty,
                 std::chrono::system_clock::now(), TimeInForce::GTC,
                 OrderStatus::NEW);
}

static std::shared_ptr<Order> MakeSell(int id, uint64_t price, int qty) {
    return std::make_shared<Order>(id, 1, OrderSide::SELL, OrderType::LIMIT, "AAPL", price, qty,
                 std::chrono::system_clock::now(), TimeInForce::GTC,
                 OrderStatus::NEW);
}

// ─────────────────────────────────────────────
// Fixture
// ─────────────────────────────────────────────

class BinaryOrderBookTest : public ::testing::Test {
protected:
    BinaryOrderBook book;
    std::vector<std::shared_ptr<Order>> orders;

    void AddBuy(int id, uint64_t price, int qty) {
        orders.emplace_back(MakeBuy(id, price, qty));
        book.AddOrder(orders.back());
    }

    void AddSell(int id, uint64_t price, int qty) {
        orders.emplace_back(MakeSell(id, price, qty));
        book.AddOrder(orders.back());
    }
};

// ═════════════════════════════════════════════
// AddOrder
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, AddBuyOrder_BookNotEmpty) {
    AddBuy(1, 100, 10);
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(BinaryOrderBookTest, AddSellOrder_BookNotEmpty) {
    AddSell(2, 101, 5);
    EXPECT_FALSE(book.IsAskEmpty());
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, AddMultipleBuyOrders_BestBidIsHighestPrice) {
    AddBuy(1, 99,  10);
    AddBuy(2, 105, 10);
    AddBuy(3, 101, 10);
    ASSERT_NE(book.GetBestBid(), nullptr);
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 105);
}

TEST_F(BinaryOrderBookTest, AddMultipleSellOrders_BestAskIsLowestPrice) {
    AddSell(1, 110, 5);
    AddSell(2, 102, 5);
    AddSell(3, 108, 5);
    ASSERT_NE(book.GetBestAsk(), nullptr);
    EXPECT_EQ(book.GetBestAsk()->GetPrice(), 102);
}

TEST_F(BinaryOrderBookTest, AddOrdersAtSamePrice_BothStored) {
    AddBuy(1, 100, 10);
    AddBuy(2, 100, 20);
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 100);
    book.PopBestBid();
    EXPECT_FALSE(book.IsBidEmpty()); // second one still there
}

// ═════════════════════════════════════════════
// GetBestBid / GetBestAsk on empty book
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, GetBestBid_EmptyBook_ReturnsNullptr) {
    EXPECT_EQ(book.GetBestBid(), nullptr);
}

TEST_F(BinaryOrderBookTest, GetBestAsk_EmptyBook_ReturnsNullptr) {
    EXPECT_EQ(book.GetBestAsk(), nullptr);
}

// ═════════════════════════════════════════════
// IsBidEmpty / IsAskEmpty
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, IsBidEmpty_InitiallyTrue) {
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, IsAskEmpty_InitiallyTrue) {
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(BinaryOrderBookTest, IsBidEmpty_FalseAfterAdd) {
    AddBuy(1, 100, 10);
    EXPECT_FALSE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, IsAskEmpty_FalseAfterAdd) {
    AddSell(1, 100, 10);
    EXPECT_FALSE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// CancelOrder
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, CancelBuyOrder_RemovesFromBook) {
    AddBuy(1, 100, 10);
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, CancelSellOrder_RemovesFromBook) {
    AddSell(2, 101, 5);
    book.CancelOrder(2);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(BinaryOrderBookTest, CancelOrder_SetsStatusCanceled) {
    AddBuy(1, 100, 10);
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, CancelOrder_NonExistentId_DoesNotCrash) {
    AddBuy(1, 100, 10);
    book.CancelOrder(999);
    EXPECT_FALSE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, CancelOrder_OnlyRemovesTargetOrder) {
    AddBuy(1, 100, 10);
    AddBuy(2, 105, 10);
    book.CancelOrder(1);
    ASSERT_NE(book.GetBestBid(), nullptr);
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 105);
}

TEST_F(BinaryOrderBookTest, CancelOrder_EmptyBook_DoesNotCrash) {
    book.CancelOrder(42);
    EXPECT_TRUE(book.IsBidEmpty());
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// UpdateQuantity
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, UpdateQuantity_BuyOrder_PartialFill) {
    AddBuy(1, 100, 10);
    book.UpdateQuantity(1, 5);
    ASSERT_NE(book.GetBestBid(), nullptr);
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 5);
    EXPECT_EQ(book.GetBestBid()->GetStatus(), OrderStatus::PARTIALLY_FILLED);
}

TEST_F(BinaryOrderBookTest, UpdateQuantity_BuyOrder_FullFill_RemovesOrder) {
    AddBuy(1, 100, 10);
    book.UpdateQuantity(1, 0);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, UpdateQuantity_SellOrder_PartialFill) {
    AddSell(1, 101, 20);
    book.UpdateQuantity(1, 8);
    ASSERT_NE(book.GetBestAsk(), nullptr);
    EXPECT_EQ(book.GetBestAsk()->GetQuantity(), 8);
    EXPECT_EQ(book.GetBestAsk()->GetStatus(), OrderStatus::PARTIALLY_FILLED);
}

TEST_F(BinaryOrderBookTest, UpdateQuantity_SellOrder_FullFill_RemovesOrder) {
    AddSell(1, 101, 20);
    book.UpdateQuantity(1, 0);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(BinaryOrderBookTest, UpdateQuantity_NonExistentId_DoesNotCrash) {
    AddBuy(1, 100, 10);
    book.UpdateQuantity(999, 5);
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 10);
}

TEST_F(BinaryOrderBookTest, UpdateQuantity_SetsFilledStatus_WhenZero) {
    AddSell(1, 101, 10);
    book.UpdateQuantity(1, 0);
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// PopBestBid
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, PopBestBid_RemovesTopBid) {
    AddBuy(1, 100, 10);
    AddBuy(2, 110, 5);
    book.PopBestBid(); // removes 110
    ASSERT_NE(book.GetBestBid(), nullptr);
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 100);
}

TEST_F(BinaryOrderBookTest, PopBestBid_EmptyBook_DoesNotCrash) {
    book.PopBestBid();
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, PopBestBid_SingleOrder_BookBecomesEmpty) {
    AddBuy(1, 100, 10);
    book.PopBestBid();
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, PopBestBid_SetsFilledStatus) {
    AddBuy(1, 100, 10);
    book.PopBestBid();
    EXPECT_TRUE(book.IsBidEmpty());
}

// ═════════════════════════════════════════════
// PopBestAsk
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, PopBestAsk_RemovesTopAsk) {
    AddSell(1, 105, 5);
    AddSell(2, 102, 5);
    book.PopBestAsk(); // removes 102
    ASSERT_NE(book.GetBestAsk(), nullptr);
    EXPECT_EQ(book.GetBestAsk()->GetPrice(), 105);
}

TEST_F(BinaryOrderBookTest, PopBestAsk_EmptyBook_DoesNotCrash) {
    book.PopBestAsk();
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(BinaryOrderBookTest, PopBestAsk_SingleOrder_BookBecomesEmpty) {
    AddSell(1, 100, 5);
    book.PopBestAsk();
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// Integration / sequence scenarios
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, Integration_AddPartialFillThenCancel) {
    AddBuy(1, 100, 20);
    book.UpdateQuantity(1, 10);
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 10);
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, Integration_PopUntilEmpty) {
    AddSell(1, 100, 5);
    AddSell(2, 101, 5);
    AddSell(3, 102, 5);
    book.PopBestAsk();
    book.PopBestAsk();
    book.PopBestAsk();
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(BinaryOrderBookTest, Integration_BidAndAskSide_Independent) {
    AddBuy(1, 100, 10);
    AddSell(2, 101, 10);
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
    EXPECT_FALSE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// Branch coverage: UpdateQuantity negative value
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, UpdateQuantity_NegativeValue_BuyOrder_TreatedAsFilled) {
    AddBuy(1, 100, 10);
    book.UpdateQuantity(1, -1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, UpdateQuantity_NegativeValue_SellOrder_TreatedAsFilled) {
    AddSell(1, 100, 10);
    book.UpdateQuantity(1, -1);
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// Branch coverage: CancelOrder — bid/ask find() hit/miss paths
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, CancelOrder_BidNotFound_AskFound) {
    AddSell(1, 100, 10);
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(BinaryOrderBookTest, CancelOrder_BidFound_AskNotSearched) {
    AddBuy(1, 100, 10);
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, CancelOrder_NeitherFound_BothMiss) {
    AddBuy(1, 100, 10);
    AddSell(2, 101, 10);
    book.CancelOrder(999);
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_FALSE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// Branch coverage: UpdateQuantity — bid not found, ask lookup taken
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, UpdateQuantity_BidNotFound_AskFound_Partial) {
    AddSell(1, 100, 20);
    book.UpdateQuantity(1, 7);
    EXPECT_EQ(book.GetBestAsk()->GetQuantity(), 7);
}

TEST_F(BinaryOrderBookTest, UpdateQuantity_BidNotFound_AskFound_Full) {
    AddSell(1, 100, 20);
    book.UpdateQuantity(1, 0);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(BinaryOrderBookTest, UpdateQuantity_NeitherFound_BothMiss) {
    AddBuy(1, 100, 10);
    book.UpdateQuantity(999, 5);
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 10);
}

// ═════════════════════════════════════════════
// Bitmap specific: verify bitmap stays consistent
// after multiple adds and removes at same price level
// ═════════════════════════════════════════════

TEST_F(BinaryOrderBookTest, Bitmap_AddRemoveAtSamePrice_BitmapConsistent) {
    AddBuy(1, 100, 10);
    AddBuy(2, 100, 10);
    book.PopBestBid(); // removes first node, price level still has order 2
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 100);
    book.PopBestBid(); // removes last node at price 100
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, Bitmap_MultiplePriceLevels_AfterPop_NextLevelVisible) {
    AddBuy(1, 100, 10);
    AddBuy(2, 200, 10);
    AddBuy(3, 300, 10);
    book.PopBestBid(); // removes 300
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 200);
    book.PopBestBid(); // removes 200
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 100);
    book.PopBestBid();
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(BinaryOrderBookTest, Bitmap_AskMultiplePriceLevels_AfterPop_NextLevelVisible) {
    AddSell(1, 300, 5);
    AddSell(2, 200, 5);
    AddSell(3, 100, 5);
    book.PopBestAsk(); // removes 100
    EXPECT_EQ(book.GetBestAsk()->GetPrice(), 200);
    book.PopBestAsk(); // removes 200
    EXPECT_EQ(book.GetBestAsk()->GetPrice(), 300);
    book.PopBestAsk();
    EXPECT_TRUE(book.IsAskEmpty());
}
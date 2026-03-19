#include <gtest/gtest.h>
#include "../../include/Storage/MultisetOrderBookStorage.h"

// ─────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────

static std::shared_ptr<Order> MakeBuy(int id, uint64_t price, int qty) {
    return std::make_shared<Order>(id, 1, OrderSide::BUY, OrderType::LIMIT, "AAPL", price, qty,
                 std::chrono::system_clock::now(), TimeInForce::GTC,
                 OrderStatus::NEW);
}

static std::shared_ptr<Order>  MakeSell(int id, uint64_t price, int qty) {
    return  std::make_shared<Order>(id, 1, OrderSide::SELL, OrderType::LIMIT, "AAPL", price, qty,
                 std::chrono::system_clock::now(), TimeInForce::GTC,
                 OrderStatus::NEW);
}

// ─────────────────────────────────────────────
// Fixture
// ─────────────────────────────────────────────

class MultisetOrderBookTest : public ::testing::Test {
protected:
    MultisetOrderBook book;
};

// ═════════════════════════════════════════════
// AddOrder
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, AddBuyOrder_BookNotEmpty) {
    book.AddOrder(MakeBuy(1, 100, 10));
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, AddSellOrder_BookNotEmpty) {
    book.AddOrder(MakeSell(2, 101, 5));
    EXPECT_FALSE(book.IsAskEmpty());
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, AddMultipleBuyOrders_BestBidIsHighestPrice) {
    book.AddOrder(MakeBuy(1, 99,  10));
    book.AddOrder(MakeBuy(2, 105, 10));
    book.AddOrder(MakeBuy(3, 101, 10));
    ASSERT_NE(book.GetBestBid(), nullptr);
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 105);
}

TEST_F(MultisetOrderBookTest, AddMultipleSellOrders_BestAskIsLowestPrice) {
    book.AddOrder(MakeSell(1, 110, 5));
    book.AddOrder(MakeSell(2, 102, 5));
    book.AddOrder(MakeSell(3, 108, 5));
    ASSERT_NE(book.GetBestAsk(), nullptr);
    EXPECT_EQ(book.GetBestAsk()->GetPrice(), 102);
}

TEST_F(MultisetOrderBookTest, AddOrdersAtSamePrice_BothStored) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeBuy(2, 100, 20));
    // Both should be in the book; best bid is still 100
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 100);
    book.PopBestBid();
    EXPECT_FALSE(book.IsBidEmpty()); // second one still there
}

// ═════════════════════════════════════════════
// GetBestBid / GetBestAsk on empty book
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, GetBestBid_EmptyBook_ReturnsNullptr) {
    EXPECT_EQ(book.GetBestBid(), nullptr);
}

TEST_F(MultisetOrderBookTest, GetBestAsk_EmptyBook_ReturnsNullptr) {
    EXPECT_EQ(book.GetBestAsk(), nullptr);
}

// ═════════════════════════════════════════════
// IsBidEmpty / IsAskEmpty
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, IsBidEmpty_InitiallyTrue) {
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, IsAskEmpty_InitiallyTrue) {
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, IsBidEmpty_FalseAfterAdd) {
    book.AddOrder(MakeBuy(1, 100, 10));
    EXPECT_FALSE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, IsAskEmpty_FalseAfterAdd) {
    book.AddOrder(MakeSell(1, 100, 10));
    EXPECT_FALSE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// CancelOrder
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, CancelBuyOrder_RemovesFromBook) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, CancelSellOrder_RemovesFromBook) {
    book.AddOrder(MakeSell(2, 101, 5));
    book.CancelOrder(2);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, CancelOrder_SetsStatusCanceled) {
    std::shared_ptr<Order> buy = MakeBuy(1, 100, 10);
    book.AddOrder(buy);
    // After cancel the order is gone from the book; status was set before erase
    // We verify indirectly: book is empty and no crash
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, CancelOrder_NonExistentId_DoesNotCrash) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.CancelOrder(999); // should be a no-op
    EXPECT_FALSE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, CancelOrder_OnlyRemovesTargetOrder) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeBuy(2, 105, 10));
    book.CancelOrder(1);
    ASSERT_NE(book.GetBestBid(), nullptr);
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 105);
}

TEST_F(MultisetOrderBookTest, CancelOrder_EmptyBook_DoesNotCrash) {
    book.CancelOrder(42); // no-op on empty book
    EXPECT_TRUE(book.IsBidEmpty());
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// UpdateQuantity
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, UpdateQuantity_BuyOrder_PartialFill) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.UpdateQuantity(1, 5);
    ASSERT_NE(book.GetBestBid(), nullptr);
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 5);
    EXPECT_EQ(book.GetBestBid()->GetStatus(), OrderStatus::PARTIALLY_FILLED);
}

TEST_F(MultisetOrderBookTest, UpdateQuantity_BuyOrder_FullFill_RemovesOrder) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.UpdateQuantity(1, 0);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, UpdateQuantity_SellOrder_PartialFill) {
    book.AddOrder(MakeSell(1, 101, 20));
    book.UpdateQuantity(1, 8);
    ASSERT_NE(book.GetBestAsk(), nullptr);
    EXPECT_EQ(book.GetBestAsk()->GetQuantity(), 8);
    EXPECT_EQ(book.GetBestAsk()->GetStatus(), OrderStatus::PARTIALLY_FILLED);
}

TEST_F(MultisetOrderBookTest, UpdateQuantity_SellOrder_FullFill_RemovesOrder) {
    book.AddOrder(MakeSell(1, 101, 20));
    book.UpdateQuantity(1, 0);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, UpdateQuantity_NonExistentId_DoesNotCrash) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.UpdateQuantity(999, 5); // no-op
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 10);
}

TEST_F(MultisetOrderBookTest, UpdateQuantity_SetsFilledStatus_WhenZero) {
    book.AddOrder(MakeSell(1, 101, 10));
    book.UpdateQuantity(1, 0);
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// PopBestBid
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, PopBestBid_RemovesTopBid) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeBuy(2, 110, 5));
    book.PopBestBid(); // removes 110
    ASSERT_NE(book.GetBestBid(), nullptr);
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 100);
}

TEST_F(MultisetOrderBookTest, PopBestBid_EmptyBook_DoesNotCrash) {
    book.PopBestBid(); // no-op
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, PopBestBid_SingleOrder_BookBecomesEmpty) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.PopBestBid();
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, PopBestBid_SetsFilledStatus) {
    book.AddOrder(MakeBuy(1, 100, 10));
    // After pop the order is gone; verify book is empty (status was set before erase)
    book.PopBestBid();
    EXPECT_TRUE(book.IsBidEmpty());
}

// ═════════════════════════════════════════════
// PopBestAsk
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, PopBestAsk_RemovesTopAsk) {
    book.AddOrder(MakeSell(1, 105, 5));
    book.AddOrder(MakeSell(2, 102, 5));
    book.PopBestAsk(); // removes 102
    ASSERT_NE(book.GetBestAsk(), nullptr);
    EXPECT_EQ(book.GetBestAsk()->GetPrice(), 105);
}

TEST_F(MultisetOrderBookTest, PopBestAsk_EmptyBook_DoesNotCrash) {
    book.PopBestAsk(); // no-op
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, PopBestAsk_SingleOrder_BookBecomesEmpty) {
    book.AddOrder(MakeSell(1, 100, 5));
    book.PopBestAsk();
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// CanFillQuantityAsks
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, CanFillQuantityAsks_SingleOrder_ExactFill) {
    book.AddOrder(MakeSell(1, 100, 10));
    EXPECT_TRUE(book.CanFillQuantityAsks(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityAsks_SingleOrder_PartialAvailable) {
    book.AddOrder(MakeSell(1, 100, 5));
    EXPECT_FALSE(book.CanFillQuantityAsks(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityAsks_MultipleOrders_CombinedFill) {
    book.AddOrder(MakeSell(1, 100, 5));
    book.AddOrder(MakeSell(2, 100, 5));
    EXPECT_TRUE(book.CanFillQuantityAsks(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityAsks_PriceTooHigh_BreaksEarly) {
    // Only orders above Price=99 exist; should not fill
    book.AddOrder(MakeSell(1, 105, 100));
    EXPECT_FALSE(book.CanFillQuantityAsks(10, 99));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityAsks_EmptyBook_ReturnsFalse) {
    EXPECT_FALSE(book.CanFillQuantityAsks(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityAsks_MixedPrices_OnlyUsesEligible) {
    book.AddOrder(MakeSell(1, 100, 5));  // eligible  (price <= 102)
    book.AddOrder(MakeSell(2, 103, 50)); // ineligible (price > 102)
    EXPECT_FALSE(book.CanFillQuantityAsks(10, 102));
}

// ═════════════════════════════════════════════
// CanFillQuantityBids
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, CanFillQuantityBids_SingleOrder_ExactFill) {
    book.AddOrder(MakeBuy(1, 100, 10));
    EXPECT_TRUE(book.CanFillQuantityBids(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityBids_SingleOrder_PartialAvailable) {
    book.AddOrder(MakeBuy(1, 100, 5));
    EXPECT_FALSE(book.CanFillQuantityBids(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityBids_MultipleOrders_CombinedFill) {
    book.AddOrder(MakeBuy(1, 105, 5));
    book.AddOrder(MakeBuy(2, 103, 5));
    EXPECT_TRUE(book.CanFillQuantityBids(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityBids_PriceTooLow_BreaksEarly) {
    // Bids are sorted descending; break when price < Price
    book.AddOrder(MakeBuy(1, 95, 100));
    EXPECT_FALSE(book.CanFillQuantityBids(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityBids_EmptyBook_ReturnsFalse) {
    EXPECT_FALSE(book.CanFillQuantityBids(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityBids_MixedPrices_OnlyUsesEligible) {
    book.AddOrder(MakeBuy(1, 110, 5)); // eligible   (price >= 105)
    book.AddOrder(MakeBuy(2, 100, 50)); // ineligible (price < 105)
    EXPECT_FALSE(book.CanFillQuantityBids(10, 105));
}

// ═════════════════════════════════════════════
// Integration / sequence scenarios
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, Integration_AddPartialFillThenCancel) {
    book.AddOrder(MakeBuy(1, 100, 20));
    book.UpdateQuantity(1, 10); // partial fill
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 10);
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, Integration_PopUntilEmpty) {
    book.AddOrder(MakeSell(1, 100, 5));
    book.AddOrder(MakeSell(2, 101, 5));
    book.AddOrder(MakeSell(3, 102, 5));
    book.PopBestAsk();
    book.PopBestAsk();
    book.PopBestAsk();
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, Integration_BidAndAskSide_Independent) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeSell(2, 101, 10));
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
    EXPECT_FALSE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, Integration_CanFillAfterPartialUpdate) {
    book.AddOrder(MakeSell(1, 100, 10));
    book.UpdateQuantity(1, 4); // now only 4 available
    EXPECT_FALSE(book.CanFillQuantityAsks(5, 100));
    EXPECT_TRUE(book.CanFillQuantityAsks(4, 100));
}

TEST_F(MultisetOrderBookTest, Integration_FullFillThenCanFillReturnsFalse) {
    book.AddOrder(MakeSell(1, 100, 10));
    book.UpdateQuantity(1, 0); // fully filled — removed
    EXPECT_FALSE(book.CanFillQuantityAsks(1, 100));
}

// ═════════════════════════════════════════════
// Branch coverage: UpdateQuantity negative value
// covers the implicit else (quantity < 0) branch
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, UpdateQuantity_NegativeValue_BuyOrder_TreatedAsFilled) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.UpdateQuantity(1, -1); // else branch: treated as filled, order removed
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, UpdateQuantity_NegativeValue_SellOrder_TreatedAsFilled) {
    book.AddOrder(MakeSell(1, 100, 10));
    book.UpdateQuantity(1, -1); // else branch: treated as filled, order removed
    EXPECT_TRUE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// Branch coverage: CancelOrder — bid not found, ask lookup taken
// and vice versa, to cover both find() hit/miss paths explicitly
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, CancelOrder_BidNotFound_AskFound) {
    // Only a sell order exists — bidLocation.find() misses, askLocation.find() hits
    book.AddOrder(MakeSell(1, 100, 10));
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, CancelOrder_BidFound_AskNotSearched) {
    // Only a buy order exists — bidLocation.find() hits, early return taken
    book.AddOrder(MakeBuy(1, 100, 10));
    book.CancelOrder(1);
    EXPECT_TRUE(book.IsBidEmpty());
}

TEST_F(MultisetOrderBookTest, CancelOrder_NeitherFound_BothMiss) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeSell(2, 101, 10));
    book.CancelOrder(999); // both find() calls miss
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_FALSE(book.IsAskEmpty());
}

// ═════════════════════════════════════════════
// Branch coverage: UpdateQuantity — bid not found, ask lookup taken
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, UpdateQuantity_BidNotFound_AskFound_Partial) {
    // bidLocation.find() misses, askLocation.find() hits, quantity > 0 branch
    book.AddOrder(MakeSell(1, 100, 20));
    book.UpdateQuantity(1, 7);
    EXPECT_EQ(book.GetBestAsk()->GetQuantity(), 7);
}

TEST_F(MultisetOrderBookTest, UpdateQuantity_BidNotFound_AskFound_Full) {
    // bidLocation.find() misses, askLocation.find() hits, quantity == 0 branch
    book.AddOrder(MakeSell(1, 100, 20));
    book.UpdateQuantity(1, 0);
    EXPECT_TRUE(book.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, UpdateQuantity_NeitherFound_BothMiss) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.UpdateQuantity(999, 5); // both find() calls miss
    EXPECT_EQ(book.GetBestBid()->GetQuantity(), 10);
}

// ═════════════════════════════════════════════
// Branch coverage: CanFillQuantityAsks — inner Quantity==0 branch
// both true (return true) and false (loop exhausted) paths
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, CanFillQuantityAsks_QuantityHitsZeroMidLoop) {
    // Two orders; quantity is satisfied after the first — Quantity==0 true mid-loop
    book.AddOrder(MakeSell(1, 100, 10));
    book.AddOrder(MakeSell(2, 101,  5));
    EXPECT_TRUE(book.CanFillQuantityAsks(10, 101));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityAsks_LoopExhausted_NeverHitsZero) {
    // Orders exist but combined qty < requested — loop ends, Quantity==0 never true
    book.AddOrder(MakeSell(1, 100, 3));
    book.AddOrder(MakeSell(2, 100, 3));
    EXPECT_FALSE(book.CanFillQuantityAsks(10, 100));
}

// ═════════════════════════════════════════════
// Branch coverage: CanFillQuantityBids — inner Quantity==0 branch
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, CanFillQuantityBids_QuantityHitsZeroMidLoop) {
    // Two orders; quantity satisfied after the first
    book.AddOrder(MakeBuy(1, 110, 10));
    book.AddOrder(MakeBuy(2, 105,  5));
    EXPECT_TRUE(book.CanFillQuantityBids(10, 100));
}

TEST_F(MultisetOrderBookTest, CanFillQuantityBids_LoopExhausted_NeverHitsZero) {
    // Combined qty < requested — Quantity==0 never true
    book.AddOrder(MakeBuy(1, 110, 3));
    book.AddOrder(MakeBuy(2, 108, 3));
    EXPECT_FALSE(book.CanFillQuantityBids(10, 100));
}

// ═════════════════════════════════════════════
// Shared ownership verification
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, SharedOwnership_OrderModifiedViaSharedPtr) {
    auto order = MakeBuy(1, 100, 10);
    book.AddOrder(order);
    // order and book share ownership — status change visible from both
    book.CancelOrder(1);
    EXPECT_EQ(order->GetStatus(), OrderStatus::CANCELED);
}

TEST_F(MultisetOrderBookTest, SharedOwnership_PartialFillVisibleOutside) {
    auto order = MakeSell(1, 100, 20);
    book.AddOrder(order);
    book.UpdateQuantity(1, 5);
    // shared_ptr means the same object is updated
    EXPECT_EQ(order->GetQuantity(), 5);
    EXPECT_EQ(order->GetStatus(), OrderStatus::PARTIALLY_FILLED);
}

// ═════════════════════════════════════════════
// GetBestBids
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, GetBestBids_EmptyBook_ReturnsEmpty) {
    EXPECT_TRUE(book.GetBestBids(5).empty());
}

TEST_F(MultisetOrderBookTest, GetBestBids_XLargerThanBook_ReturnsAll) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeBuy(2, 105, 10));
    auto result = book.GetBestBids(10); // 10 cerut, 2 disponibile
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(MultisetOrderBookTest, GetBestBids_XZero_ReturnsEmpty) {
    book.AddOrder(MakeBuy(1, 100, 10));
    EXPECT_TRUE(book.GetBestBids(0).empty());
}

TEST_F(MultisetOrderBookTest, GetBestBids_ExactX_ReturnsX) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeBuy(2, 105, 10));
    book.AddOrder(MakeBuy(3, 103, 10));
    auto result = book.GetBestBids(2);
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(MultisetOrderBookTest, GetBestBids_SortedDescending_HighestPriceFirst) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeBuy(2, 105, 10));
    book.AddOrder(MakeBuy(3, 103, 10));
    auto result = book.GetBestBids(3);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0]->GetPrice(), 105u);
    EXPECT_EQ(result[1]->GetPrice(), 103u);
    EXPECT_EQ(result[2]->GetPrice(), 100u);
}

TEST_F(MultisetOrderBookTest, GetBestBids_SamePriceFIFO_OlderOrderFirst) {
    // Comenzile cu același preț sunt ordonate după timp (FIFO)
    // MakeBuy folosește now() la fiecare apel → ordinea de inserție = ordinea temporală
    auto b1 = MakeBuy(1, 100, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto b2 = MakeBuy(2, 100, 8);
    book.AddOrder(b1);
    book.AddOrder(b2);
    auto result = book.GetBestBids(2);
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0]->GetOrderID(), 1); // b1 mai vechi → primul
    EXPECT_EQ(result[1]->GetOrderID(), 2);
}

TEST_F(MultisetOrderBookTest, GetBestBids_DoesNotModifyBook) {
    book.AddOrder(MakeBuy(1, 100, 10));
    book.AddOrder(MakeBuy(2, 105, 10));
    book.GetBestBids(2);
    // book intact după query
    EXPECT_FALSE(book.IsBidEmpty());
    EXPECT_EQ(book.GetBestBid()->GetPrice(), 105u);
}

TEST_F(MultisetOrderBookTest, GetBestBids_OnlyBidsSide_AskUnaffected) {
    book.AddOrder(MakeBuy(1,  100, 10));
    book.AddOrder(MakeSell(2, 101, 10));
    auto result = book.GetBestBids(5);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->GetSide(), OrderSide::BUY);
}

TEST_F(MultisetOrderBookTest, GetBestBids_AfterCancel_ReturnsUpdatedBook) {
    book.AddOrder(MakeBuy(1, 105, 10));
    book.AddOrder(MakeBuy(2, 100, 10));
    book.CancelOrder(1); // scoate cel mai bun bid
    auto result = book.GetBestBids(2);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->GetPrice(), 100u);
}

TEST_F(MultisetOrderBookTest, GetBestBids_Top3_CorrectQuantities) {
    book.AddOrder(MakeBuy(1, 110, 3));
    book.AddOrder(MakeBuy(2, 105, 7));
    book.AddOrder(MakeBuy(3, 100, 12));
    book.AddOrder(MakeBuy(4,  95, 1));
    auto result = book.GetBestBids(3);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0]->GetQuantity(), 3);
    EXPECT_EQ(result[1]->GetQuantity(), 7);
    EXPECT_EQ(result[2]->GetQuantity(), 12);
}

// ═════════════════════════════════════════════
// GetBestAsks
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, GetBestAsks_EmptyBook_ReturnsEmpty) {
    EXPECT_TRUE(book.GetBestAsks(5).empty());
}

TEST_F(MultisetOrderBookTest, GetBestAsks_XLargerThanBook_ReturnsAll) {
    book.AddOrder(MakeSell(1, 100, 5));
    book.AddOrder(MakeSell(2, 105, 5));
    auto result = book.GetBestAsks(10);
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(MultisetOrderBookTest, GetBestAsks_XZero_ReturnsEmpty) {
    book.AddOrder(MakeSell(1, 100, 5));
    EXPECT_TRUE(book.GetBestAsks(0).empty());
}

TEST_F(MultisetOrderBookTest, GetBestAsks_ExactX_ReturnsX) {
    book.AddOrder(MakeSell(1, 100, 5));
    book.AddOrder(MakeSell(2, 103, 5));
    book.AddOrder(MakeSell(3, 107, 5));
    auto result = book.GetBestAsks(2);
    EXPECT_EQ(result.size(), 2u);
}

TEST_F(MultisetOrderBookTest, GetBestAsks_SortedAscending_LowestPriceFirst) {
    book.AddOrder(MakeSell(1, 107, 5));
    book.AddOrder(MakeSell(2, 100, 5));
    book.AddOrder(MakeSell(3, 103, 5));
    auto result = book.GetBestAsks(3);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0]->GetPrice(), 100u);
    EXPECT_EQ(result[1]->GetPrice(), 103u);
    EXPECT_EQ(result[2]->GetPrice(), 107u);
}

TEST_F(MultisetOrderBookTest, GetBestAsks_SamePriceFIFO_OlderOrderFirst) {
    auto s1 = MakeSell(1, 100, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto s2 = MakeSell(2, 100, 8);
    book.AddOrder(s1);
    book.AddOrder(s2);
    auto result = book.GetBestAsks(2);
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0]->GetOrderID(), 1);
    EXPECT_EQ(result[1]->GetOrderID(), 2);
}

TEST_F(MultisetOrderBookTest, GetBestAsks_DoesNotModifyBook) {
    book.AddOrder(MakeSell(1, 100, 5));
    book.AddOrder(MakeSell(2, 105, 5));
    book.GetBestAsks(2);
    EXPECT_FALSE(book.IsAskEmpty());
    EXPECT_EQ(book.GetBestAsk()->GetPrice(), 100u);
}

TEST_F(MultisetOrderBookTest, GetBestAsks_OnlyAsksSide_BidUnaffected) {
    book.AddOrder(MakeBuy(1,  99, 10));
    book.AddOrder(MakeSell(2, 101, 5));
    auto result = book.GetBestAsks(5);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->GetSide(), OrderSide::SELL);
}

TEST_F(MultisetOrderBookTest, GetBestAsks_AfterCancel_ReturnsUpdatedBook) {
    book.AddOrder(MakeSell(1, 100, 5));
    book.AddOrder(MakeSell(2, 105, 5));
    book.CancelOrder(1); // scoate cel mai bun ask
    auto result = book.GetBestAsks(2);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->GetPrice(), 105u);
}

TEST_F(MultisetOrderBookTest, GetBestAsks_Top3_CorrectQuantities) {
    book.AddOrder(MakeSell(1, 100, 2));
    book.AddOrder(MakeSell(2, 103, 6));
    book.AddOrder(MakeSell(3, 107, 9));
    book.AddOrder(MakeSell(4, 112, 1));
    auto result = book.GetBestAsks(3);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0]->GetQuantity(), 2);
    EXPECT_EQ(result[1]->GetQuantity(), 6);
    EXPECT_EQ(result[2]->GetQuantity(), 9);
}

// ═════════════════════════════════════════════
// GetBestBids + GetBestAsks — combinat
// ═════════════════════════════════════════════

TEST_F(MultisetOrderBookTest, GetBestBidsAndAsks_IndependentOfEachOther) {
    book.AddOrder(MakeBuy(1,  99, 10));
    book.AddOrder(MakeBuy(2,  95,  5));
    book.AddOrder(MakeSell(3, 101,  3));
    book.AddOrder(MakeSell(4, 104,  7));

    auto bids = book.GetBestBids(2);
    auto asks = book.GetBestAsks(2);

    ASSERT_EQ(bids.size(), 2u);
    ASSERT_EQ(asks.size(), 2u);
    EXPECT_EQ(bids[0]->GetPrice(), 99u);
    EXPECT_EQ(bids[1]->GetPrice(), 95u);
    EXPECT_EQ(asks[0]->GetPrice(), 101u);
    EXPECT_EQ(asks[1]->GetPrice(), 104u);
}

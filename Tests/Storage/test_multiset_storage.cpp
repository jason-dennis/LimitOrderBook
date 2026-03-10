#include <gtest/gtest.h>
#include "../../include/Storage/MultisetOrderBookStorage.h"
#include "../../include/Domain/order.h"
#include <chrono>

class MultisetOrderBookTest : public ::testing::Test {
protected:
    MultisetOrderBook ob;

    Order CreateTestOrder(int id, uint64_t price, int qty, OrderSide side, int sec_offset = 0) {
        auto timestamp = std::chrono::system_clock::now() + std::chrono::seconds(sec_offset);
        return Order(id, 100, side, OrderType::LIMIT, "AAPL", price, qty, timestamp, TimeInForce::GTC, OrderStatus::NEW);
    }
};

// ============================================================================
// 1. TESTE PRIORITATE (Price-Time Priority)
// ============================================================================

TEST_F(MultisetOrderBookTest, BidsPricePriority) {
    ob.AddOrder(CreateTestOrder(1, 100, 10, OrderSide::BUY));
    ob.AddOrder(CreateTestOrder(2, 120, 10, OrderSide::BUY)); // Cel mai bun pret
    ob.AddOrder(CreateTestOrder(3, 110, 10, OrderSide::BUY));

    ASSERT_NE(ob.GetBestBid(), nullptr);
    EXPECT_EQ(ob.GetBestBid()->GetPrice(), 120);
    EXPECT_EQ(ob.GetBestBid()->GetOrderID(), 2);
}

TEST_F(MultisetOrderBookTest, BidsTimePriorityAtSamePrice) {
    ob.AddOrder(CreateTestOrder(1, 100, 10, OrderSide::BUY, 0)); // Primul venit
    ob.AddOrder(CreateTestOrder(2, 100, 10, OrderSide::BUY, 5)); // Venit ulterior

    ASSERT_NE(ob.GetBestBid(), nullptr);
    EXPECT_EQ(ob.GetBestBid()->GetOrderID(), 1);
}

TEST_F(MultisetOrderBookTest, AsksPricePriority) {
    ob.AddOrder(CreateTestOrder(1, 100, 10, OrderSide::SELL));
    ob.AddOrder(CreateTestOrder(2, 80, 10, OrderSide::SELL)); // Cel mai ieftin
    ob.AddOrder(CreateTestOrder(3, 90, 10, OrderSide::SELL));

    ASSERT_NE(ob.GetBestAsk(), nullptr);
    EXPECT_EQ(ob.GetBestAsk()->GetPrice(), 80);
    EXPECT_EQ(ob.GetBestAsk()->GetOrderID(), 2);
}

TEST_F(MultisetOrderBookTest, AsksTimePriorityAtSamePrice) {
    ob.AddOrder(CreateTestOrder(100, 100, 10, OrderSide::SELL, 0));
    ob.AddOrder(CreateTestOrder(101, 100, 10, OrderSide::SELL, 5));

    ASSERT_NE(ob.GetBestAsk(), nullptr);
    EXPECT_EQ(ob.GetBestAsk()->GetOrderID(), 100);
}

// ============================================================================
// 2. TESTE OPERAȚIUNI (Update & Cancel)
// ============================================================================

TEST_F(MultisetOrderBookTest, UpdateQuantityPartialAndFullFill) {
    // Test pe Bid
    ob.AddOrder(CreateTestOrder(1, 100, 10, OrderSide::BUY));
    ob.UpdateQuantity(1, 7); // Partial
    EXPECT_EQ(ob.GetBestBid()->GetStatus(), OrderStatus::PARTIALLY_FILLED);
    ob.UpdateQuantity(1, 0); // Full
    EXPECT_TRUE(ob.IsBidEmpty());

    // Test pe Ask (Coverage pentru ramura Ask din UpdateQuantity)
    ob.AddOrder(CreateTestOrder(2, 100, 10, OrderSide::SELL));
    ob.UpdateQuantity(2, 5);
    EXPECT_EQ(ob.GetBestAsk()->GetStatus(), OrderStatus::PARTIALLY_FILLED);
    ob.UpdateQuantity(2, 0);
    EXPECT_TRUE(ob.IsAskEmpty());
}

TEST_F(MultisetOrderBookTest, CancelOrderRemovesSuccessfully) {
    // Ramura Bid
    ob.AddOrder(CreateTestOrder(10, 100, 10, OrderSide::BUY));
    ob.CancelOrder(10);
    EXPECT_TRUE(ob.IsBidEmpty());

    // Ramura Ask
    ob.AddOrder(CreateTestOrder(20, 100, 10, OrderSide::SELL));
    ob.CancelOrder(20);
    EXPECT_TRUE(ob.IsAskEmpty());
}

// ============================================================================
// 3. TESTE ELIMINARE (Pop Functions)
// ============================================================================

TEST_F(MultisetOrderBookTest, PopBestBidRemovesTopCorrectly) {
    ob.AddOrder(CreateTestOrder(1, 110, 10, OrderSide::BUY));
    ob.AddOrder(CreateTestOrder(2, 100, 10, OrderSide::BUY));

    ob.PopBestBid(); // Sterge 110
    ASSERT_NE(ob.GetBestBid(), nullptr);
    EXPECT_EQ(ob.GetBestBid()->GetOrderID(), 2);
}

TEST_F(MultisetOrderBookTest, PopBestAskRemovesTopCorrectly) {
    ob.AddOrder(CreateTestOrder(1, 90, 10, OrderSide::SELL));
    ob.AddOrder(CreateTestOrder(2, 100, 10, OrderSide::SELL));

    ob.PopBestAsk(); // Sterge 90
    ASSERT_NE(ob.GetBestAsk(), nullptr);
    EXPECT_EQ(ob.GetBestAsk()->GetOrderID(), 2);
}

// ============================================================================
// 4. CAZURI LIMITĂ ȘI COVERAGE AUXILIAR
// ============================================================================

TEST_F(MultisetOrderBookTest, EmptyBookSafetyAndInvalidIDs) {
    // Citiri pe gol
    EXPECT_EQ(ob.GetBestBid(), nullptr);
    EXPECT_EQ(ob.GetBestAsk(), nullptr);
    EXPECT_TRUE(ob.IsBidEmpty());

    // Actiuni pe ID-uri inexistente (Coverage pentru ramurile 'it == end()')
    EXPECT_NO_THROW(ob.CancelOrder(999));
    EXPECT_NO_THROW(ob.UpdateQuantity(999, 5));
    EXPECT_NO_THROW(ob.PopBestBid());
    EXPECT_NO_THROW(ob.PopBestAsk());
}

TEST_F(MultisetOrderBookTest, GlobalToStringCoverage) {
    EXPECT_STREQ(ToString(OrderStatus::NEW), "NEW");
    EXPECT_STREQ(ToString(OrderSide::BUY), "BUY");
    EXPECT_STREQ(ToString(OrderType::LIMIT), "LIMIT");
    EXPECT_STREQ(ToString(TimeInForce::GTC), "GTC");
}
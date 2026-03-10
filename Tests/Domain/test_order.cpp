//
// Created by denni on 3/8/2026.
//
#include <gtest/gtest.h>
#include <chrono>
#include "../../include/Domain/order.h"

//Test 1: Validate a correct order creation
TEST(OrderTest, ValidOrderCreation) {
    auto now=std::chrono::system_clock::now();
    Order order(1,
    12,
    OrderSide::BUY,
    OrderType::LIMIT,
    "BTC",
    1502500,
    100,
    now,
    TimeInForce::GTC,
    OrderStatus::NEW
    );

    EXPECT_EQ(order.Get_OrderID(),1);
    EXPECT_EQ(order.Get_TraderID(),12);
    EXPECT_EQ(order.Get_Side(),OrderSide::BUY);
    EXPECT_EQ(order.Get_Type(),OrderType::LIMIT);
    EXPECT_EQ(order.Get_Symbol(),"BTC");
    EXPECT_EQ(order.Get_Price(),1502500);
    EXPECT_EQ(order.Get_Quantity(),100);
    EXPECT_EQ(order.Get_Time(),now);
    EXPECT_EQ(order.Get_TIF(),TimeInForce::GTC);
    EXPECT_EQ(order.Get_Status(),OrderStatus::NEW);

}

//Test 2: Catch excpetions for invalid order creation
TEST(OrderTest, ThrowsExceptions) {
    auto now=std::chrono::system_clock::now();

    //Throw excetion for price = 0
    EXPECT_THROW({
        Order order(1,
   12,
   OrderSide::BUY,
   OrderType::LIMIT,
   "BTC",
   0,
   100,
   now,
   TimeInForce::GTC,
   OrderStatus::NEW
   );

    },std::invalid_argument);

    //Throw excetion for negative quantity
    EXPECT_THROW({
        Order order(1,
   12,
   OrderSide::BUY,
   OrderType::LIMIT,
   "BTC",
   1502500,
   -100,
   now,
   TimeInForce::GTC,
   OrderStatus::NEW
   );
    },std::invalid_argument);

    //Throw excetion for an empty symbol
    EXPECT_THROW({
        Order order(1,
      12,
      OrderSide::BUY,
      OrderType::LIMIT,
      "",
      1502500,
      100,
      now,
      TimeInForce::GTC,
      OrderStatus::NEW
      );
    },std::invalid_argument);
}

//Test 3: Check Status update
TEST(OrderTest, UpdateStatus) {
    auto now=std::chrono::system_clock::now();
    Order order(1,
   12,
   OrderSide::BUY,
   OrderType::LIMIT,
   "BTC",
   1502500,
   100,
   now,
   TimeInForce::GTC,
   OrderStatus::NEW
   );
    EXPECT_EQ(order.Get_Status(),OrderStatus::NEW);
    order.Set_Status(OrderStatus::FILLED);
    EXPECT_EQ(order.Get_Status(),OrderStatus::FILLED);

}

TEST(OrderTest,OperatorStreamFormat) {

    auto now=std::chrono::system_clock::now();
    Order order(1,
   12,
   OrderSide::BUY,
   OrderType::LIMIT,
   "BTC",
   1502500,
   100,
   now,
   TimeInForce::GTC,
   OrderStatus::NEW
   );

    std::stringstream ss;
    ss<<order;
    std::string result=ss.str();

    EXPECT_TRUE(result.find("ID: 1")!=std::string::npos);
    EXPECT_TRUE(result.find("Trader: 12")!=std::string::npos);
    EXPECT_TRUE(result.find("Side: BUY")!=std::string::npos);
    EXPECT_TRUE(result.find("Type: LIMIT")!=std::string::npos);
    EXPECT_TRUE(result.find("Symbol: BTC")!=std::string::npos);
    EXPECT_TRUE(result.find("Price: 1502500")!=std::string::npos);
    EXPECT_TRUE(result.find("Quantity: 100")!=std::string::npos);
    EXPECT_TRUE(result.find("TIF: GTC")!=std::string::npos);
    EXPECT_TRUE(result.find("Status: NEW")!=std::string::npos);
    EXPECT_TRUE(result.find("Time: ")!=std::string::npos);

}
class OrderStatusTest : public ::testing::TestWithParam<OrderStatus> {};

TEST_P(OrderStatusTest,CheckToString) {
    OrderStatus status=GetParam();
    const char* result=ToString(status);
    EXPECT_NE(result,nullptr);
}

INSTANTIATE_TEST_SUITE_P(
    AllStatuses,
    OrderStatusTest,
    ::testing::Values(
        OrderStatus::NEW,
        OrderStatus::PARTIALLY_FILLED,
        OrderStatus::FILLED,
        OrderStatus::CANCELED,
        OrderStatus::REJECTED
    )
);
#include <benchmark/benchmark.h>
#include "../../include/Storage/MultisetOrderBookStorage.h"
#include "../../include/Storage/BinaryOrderBookStorage.h"
#include <random>
#include <vector>
#include <chrono>

// ============================================================================
// PRICE GENERATORS — realistic tick sizes
// Pretul e stocat ca intreg: price * 100 (tick = 0.01)
// Ex: $150.25 -> 15025
// Range realist: $10.00 - $1999.99 -> 1000 - 199999
// ============================================================================

static std::mt19937 rng(42); // seed fix pentru reproductibilitate

// Preturi random dispersate pe tot spatiul
static uint64_t RandPrice() {
    std::uniform_int_distribution<uint64_t> dist(1'000, 199'999);
    return dist(rng);
}

// Preturi clustered in jurul unui mid-price (mai realist pentru o actiune)
// Mid ~$100.00 = 10000, spread de +-$50.00 = +-5000
static uint64_t ClusteredPrice(uint64_t mid = 10'000, uint64_t spread = 5'000) {
    std::uniform_int_distribution<uint64_t> dist(mid - spread, mid + spread);
    return dist(rng);
}

static Order MakeOrder(int id, uint64_t price, OrderSide side) {
    return Order(id, 1, side, OrderType::LIMIT, "AAPL", price, 100,
                 std::chrono::system_clock::now(), TimeInForce::GTC, OrderStatus::NEW);
}

// Pre-genereaza N ordere cu preturi realiste pentru a nu contamina benchmark-ul
static std::vector<Order> GenerateOrders(int n, bool clustered = false) {
    std::vector<Order> orders;
    orders.reserve(n);
    for (int i = 0; i < n; ++i) {
        uint64_t price = clustered ? ClusteredPrice() : RandPrice();
        OrderSide side = (i % 2 == 0) ? OrderSide::BUY : OrderSide::SELL;
        orders.emplace_back(MakeOrder(i, price, side));
    }
    return orders;
}

// ============================================================================
// 1. ADD ORDER — preturi dispersate random
// ============================================================================

static void BM_Multiset_AddOrder_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n * 10);
    MultisetOrderBook ob;
    int idx = 0;
    for (auto _ : state) {
        if (idx >= (int)orders.size()) idx = 0;
        ob.AddOrder(orders[idx++]);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_AddOrder_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_AddOrder_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n * 10);
    BinaryOrderBook ob;
    int idx = 0;
    for (auto _ : state) {
        if (idx >= (int)orders.size()) idx = 0;
        ob.AddOrder(orders[idx++]);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_AddOrder_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 2. ADD ORDER — preturi clustered (mai realist pentru o singura actiune)
// ============================================================================

static void BM_Multiset_AddOrder_Clustered(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n * 10, true);
    MultisetOrderBook ob;
    int idx = 0;
    for (auto _ : state) {
        if (idx >= (int)orders.size()) idx = 0;
        ob.AddOrder(orders[idx++]);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_AddOrder_Clustered)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_AddOrder_Clustered(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n * 10, true);
    BinaryOrderBook ob;
    int idx = 0;
    for (auto _ : state) {
        if (idx >= (int)orders.size()) idx = 0;
        ob.AddOrder(orders[idx++]);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_AddOrder_Clustered)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 3. GET BEST BID/ASK — carte plina cu preturi dispersate
// ============================================================================

static void BM_Multiset_GetBestBid_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    MultisetOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);

    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestBid());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_GetBestBid_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_GetBestBid_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    BinaryOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);

    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestBid());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_GetBestBid_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Multiset_GetBestAsk_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    MultisetOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);

    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestAsk());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_GetBestAsk_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_GetBestAsk_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    BinaryOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);

    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestAsk());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_GetBestAsk_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 4. CANCEL ORDER — carte plina, cancel random
// ============================================================================

static void BM_Multiset_CancelOrder_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);

    MultisetOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);

    std::uniform_int_distribution<int> dist(0, n - 1);
    int current_id = 0;

    for (auto _ : state) {
        if (ob.IsBidEmpty() && ob.IsAskEmpty()) {
            state.PauseTiming();
            for (auto& o : orders) ob.AddOrder(o);
            current_id = 0;
            state.ResumeTiming();
        }
        ob.CancelOrder(current_id++);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_CancelOrder_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_CancelOrder_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);

    BinaryOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);

    int current_id = 0;

    for (auto _ : state) {
        if (ob.IsBidEmpty() && ob.IsAskEmpty()) {
            state.PauseTiming();
            for (auto& o : orders) ob.AddOrder(o);
            current_id = 0;
            state.ResumeTiming();
        }
        ob.CancelOrder(current_id++);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_CancelOrder_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 5. POP BEST BID/ASK — carte plina cu preturi dispersate
// ============================================================================

static void BM_Multiset_PopBestBid_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);

    MultisetOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);

    for (auto _ : state) {
        if (ob.IsBidEmpty()) {
            state.PauseTiming();
            for (auto& o : orders) ob.AddOrder(o);
            state.ResumeTiming();
        }
        ob.PopBestBid();
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_PopBestBid_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_PopBestBid_Realistic(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);

    BinaryOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);

    for (auto _ : state) {
        if (ob.IsBidEmpty()) {
            state.PauseTiming();
            for (auto& o : orders) ob.AddOrder(o);
            state.ResumeTiming();
        }
        ob.PopBestBid();
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_PopBestBid_Realistic)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 6. THROUGHPUT MIXT — simuleaza un flux real de ordere:
//    70% add, 20% cancel, 10% update quantity
// ============================================================================

static void BM_Multiset_MixedWorkload(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n * 2);
    MultisetOrderBook ob;

    for (int i = 0; i < n; ++i) ob.AddOrder(orders[i]);

    std::uniform_int_distribution<int> op_dist(0, 9);
    std::uniform_int_distribution<int> id_dist(0, n - 1);
    int add_idx = n;

    for (auto _ : state) {
        int op = op_dist(rng);
        if (op < 7) {
            // 70% add
            if (add_idx < (int)orders.size()) {
                ob.AddOrder(orders[add_idx++]);
            }
        } else if (op < 9) {
            // 20% cancel
            ob.CancelOrder(id_dist(rng));
        } else {
            // 10% update
            ob.UpdateQuantity(id_dist(rng), 50);
        }
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_MixedWorkload)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_MixedWorkload(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n * 2);
    BinaryOrderBook ob;

    for (int i = 0; i < n; ++i) ob.AddOrder(orders[i]);

    std::uniform_int_distribution<int> op_dist(0, 9);
    std::uniform_int_distribution<int> id_dist(0, n - 1);
    int add_idx = n;

    for (auto _ : state) {
        int op = op_dist(rng);
        if (op < 7) {
            // 70% add
            if (add_idx < (int)orders.size()) {
                ob.AddOrder(orders[add_idx++]);
            }
        } else if (op < 9) {
            // 20% cancel
            ob.CancelOrder(id_dist(rng));
        } else {
            // 10% update
            ob.UpdateQuantity(id_dist(rng), 50);
        }
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_MixedWorkload)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

BENCHMARK_MAIN();
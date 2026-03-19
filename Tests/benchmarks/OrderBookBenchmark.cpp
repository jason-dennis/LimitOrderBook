#include <benchmark/benchmark.h>
#include "../../include/Storage/MultisetOrderBookStorage.h"
#include "../../include/Storage/BinaryOrderBookStorage.h"
#include <random>
#include <vector>
#include <chrono>

// ============================================================================
// PRICE GENERATORS
// Tick = 100 (0.01), range realist: $10.00-$1999.99 → 1000-199999
// ============================================================================

static std::mt19937 rng(42);

static uint64_t RandPrice() {
    std::uniform_int_distribution<uint64_t> dist(1'000, 199'999);
    return dist(rng);
}

static uint64_t ClusteredPrice(uint64_t mid = 10'000, uint64_t spread = 5'000) {
    std::uniform_int_distribution<uint64_t> dist(mid - spread, mid + spread);
    return dist(rng);
}

static std::shared_ptr<Order> MakeOrder(int id, uint64_t price, OrderSide side) {
    return std::make_shared<Order>(id, 1, side, OrderType::LIMIT, "AAPL", price, 100,
                 std::chrono::system_clock::now(), TimeInForce::GTC, OrderStatus::NEW);
}

static std::vector<std::shared_ptr<Order>> GenerateOrders(int n, bool clustered = false) {
    std::vector<std::shared_ptr<Order>> orders;
    orders.reserve(n);
    for (int i = 0; i < n; ++i) {
        uint64_t price = clustered ? ClusteredPrice() : RandPrice();
        OrderSide side = (i % 2 == 0) ? OrderSide::BUY : OrderSide::SELL;
        orders.emplace_back(MakeOrder(i, price, side));
    }
    return orders;
}

// ============================================================================
// 1. ADD ORDER — random dispersat
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
// 2. ADD ORDER — clustered (mai realist)
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
// 3. GET BEST BID / ASK
// ============================================================================

static void BM_Multiset_GetBestBid(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    MultisetOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);
    for (auto _ : state) {
        auto result = ob.GetBestBid();
        benchmark::DoNotOptimize(result);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_GetBestBid)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_GetBestBid(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    BinaryOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestBid());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_GetBestBid)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Multiset_GetBestAsk(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    MultisetOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestAsk());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_GetBestAsk)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_GetBestAsk(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    BinaryOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestAsk());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_GetBestAsk)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 4. GET BEST N BIDS/ASKS (x=5 și x=15)
// ============================================================================

static void BM_Multiset_GetBestBids_5(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    MultisetOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestBids(5));
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_GetBestBids_5)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_GetBestBids_5(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    BinaryOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestBids(5));
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_GetBestBids_5)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Multiset_GetBestBids_15(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    MultisetOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestBids(15));
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Multiset_GetBestBids_15)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_GetBestBids_15(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    BinaryOrderBook ob;
    for (auto& o : orders) ob.AddOrder(o);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestBids(15));
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_GetBestBids_15)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 5. CANCEL ORDER
// ============================================================================

static void BM_Multiset_CancelOrder(benchmark::State& state) {
    int n = state.range(0);
    auto orders = GenerateOrders(n);
    MultisetOrderBook ob;
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
BENCHMARK(BM_Multiset_CancelOrder)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_CancelOrder(benchmark::State& state) {
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
BENCHMARK(BM_Binary_CancelOrder)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 6. POP BEST BID/ASK
// ============================================================================

static void BM_Multiset_PopBestBid(benchmark::State& state) {
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
BENCHMARK(BM_Multiset_PopBestBid)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_Binary_PopBestBid(benchmark::State& state) {
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
BENCHMARK(BM_Binary_PopBestBid)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 7. MIXED WORKLOAD — 70% add, 20% cancel, 10% update
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
            if (add_idx < (int)orders.size()) ob.AddOrder(orders[add_idx++]);
        } else if (op < 9) {
            ob.CancelOrder(id_dist(rng));
        } else {
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
            if (add_idx < (int)orders.size()) ob.AddOrder(orders[add_idx++]);
        } else if (op < 9) {
            ob.CancelOrder(id_dist(rng));
        } else {
            ob.UpdateQuantity(id_dist(rng), 50);
        }
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Binary_MixedWorkload)->Range(1000, 100000)->Repetitions(5)->ReportAggregatesOnly(true);

BENCHMARK_MAIN();
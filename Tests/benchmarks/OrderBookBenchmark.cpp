#include <benchmark/benchmark.h>
#include "Storage/MultisetOrderBookStorage.h"
#include <chrono>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static Order CreateDummyOrder(int id, uint64_t price, OrderSide side) {
    return Order(id, 1, side, OrderType::LIMIT, "AAPL", price, 100,
                 std::chrono::system_clock::now(), TimeInForce::GTC, OrderStatus::NEW);
}

// ============================================================================
// 1. BENCHMARK INSERȚIE (O(log N))
// ============================================================================

static void BM_AddOrder(benchmark::State& state) {
    MultisetOrderBook ob;
    int id = 0;
    for (auto _ : state) {
        ob.AddOrder(CreateDummyOrder(++id, 100 + (id % 10), OrderSide::BUY));
    }
    // Adaugă coloana "Ops/sec"
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_AddOrder)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 2. BENCHMARK CITIRE / ACCES (O(1))
// ============================================================================

static void BM_GetBestBid(benchmark::State& state) {
    MultisetOrderBook ob;
    int n = state.range(0);
    for(int i = 0; i < n; ++i) ob.AddOrder(CreateDummyOrder(i, 100 + (i % 50), OrderSide::BUY));

    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.GetBestBid());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_GetBestBid)->Range(100, 10000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_IsEmpty(benchmark::State& state) {
    MultisetOrderBook ob;
    ob.AddOrder(CreateDummyOrder(1, 100, OrderSide::BUY));

    for (auto _ : state) {
        benchmark::DoNotOptimize(ob.IsBidEmpty());
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_IsEmpty)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 3. BENCHMARK UPDATE (O(1) pentru Partial Fill)
// ============================================================================

static void BM_UpdateQuantity_PartialFill(benchmark::State& state) {
    MultisetOrderBook ob;
    int n = state.range(0);
    for(int i = 0; i < n; ++i) ob.AddOrder(CreateDummyOrder(i, 100, OrderSide::BUY));

    int id_to_update = n / 2;
    for (auto _ : state) {
        ob.UpdateQuantity(id_to_update, 50);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_UpdateQuantity_PartialFill)->Range(100, 10000)->Repetitions(5)->ReportAggregatesOnly(true);

// ============================================================================
// 4. BENCHMARK ȘTERGERE / POP (O(log N))
// ============================================================================

auto RefillBook = [](MultisetOrderBook& ob, int size) {
    for(int i = 0; i < size; ++i) {
        ob.AddOrder(CreateDummyOrder(i, 100 + (i % 10), OrderSide::BUY));
    }
};

static void BM_CancelOrder(benchmark::State& state) {
    MultisetOrderBook ob;
    int initial_size = state.range(0);
    int current_id = 0;

    RefillBook(ob, initial_size);

    for (auto _ : state) {
        if (ob.IsBidEmpty()) {
            state.PauseTiming();
            RefillBook(ob, initial_size);
            current_id = 0;
            state.ResumeTiming();
        }
        ob.CancelOrder(current_id++);
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_CancelOrder)->Range(100, 10000)->Repetitions(5)->ReportAggregatesOnly(true);

static void BM_PopBestBid(benchmark::State& state) {
    MultisetOrderBook ob;
    int initial_size = state.range(0);

    RefillBook(ob, initial_size);

    for (auto _ : state) {
        if (ob.IsBidEmpty()) {
            state.PauseTiming();
            RefillBook(ob, initial_size);
            state.ResumeTiming();
        }
        ob.PopBestBid();
    }
    state.counters["Ops/sec"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_PopBestBid)->Range(100, 10000)->Repetitions(5)->ReportAggregatesOnly(true);

BENCHMARK_MAIN();
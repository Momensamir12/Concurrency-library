#include <algorithm>
#include <vector>
#include <thread>
#include <iterator>
#include <numeric>
#include <iostream>
#include <chrono>
#include <cstdio>

// ─── parallel_accumulate ────────────────────────────────────────────────────

template <typename Iterator, typename T>
T parallel_accumulate(Iterator beg, Iterator end, T init)
{
    unsigned long const length = std::distance(beg, end);
    if (!length) return init;

    unsigned long const minimum_per_thread = 100;
    unsigned const max_threads = (length + minimum_per_thread - 1) / minimum_per_thread;
    unsigned const hardware_threads = std::thread::hardware_concurrency();
    unsigned const num_threads = std::min(
        hardware_threads != 0 ? hardware_threads : 2,
        max_threads
    );

    unsigned long const block_size = length / num_threads;
    std::vector<T> results(num_threads, T{});
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = beg;
    for (unsigned i = 0; i < num_threads - 1; i++)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread([block_start, block_end, &results, i]() {
            results[i] = std::accumulate(block_start, block_end, T{});
        });
        block_start = block_end;
    }

    results[num_threads - 1] = std::accumulate(block_start, end, T{});
    for (auto& t : threads) t.join();
    return std::accumulate(results.begin(), results.end(), init);
}

// ─── benchmark ──────────────────────────────────────────────────────────────

struct Result { size_t n; double serial_us; double parallel_us; };

Result run_benchmark(size_t N, int runs = 5)
{
    std::vector<long long> data(N, 1LL);
    long long serial_us = 0, parallel_us = 0;

    for (int r = 0; r < runs; r++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        volatile auto s = std::accumulate(data.begin(), data.end(), 0LL);
        auto t2 = std::chrono::high_resolution_clock::now();
        serial_us += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        (void)s;
    }

    for (int r = 0; r < runs; r++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        volatile auto p = parallel_accumulate(data.begin(), data.end(), 0LL);
        auto t2 = std::chrono::high_resolution_clock::now();
        parallel_us += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        (void)p;
    }

    return { N, (double)serial_us / runs, (double)parallel_us / runs };
}

// ─── main ────────────────────────────────────────────────────────────────────

int main()
{
    std::vector<size_t> sizes = {
        100, 500, 1000, 5000, 10000, 50000,
        100000, 500000, 1000000, 2000000,
        5000000, 10000000, 20000000, 50000000
    };

    std::vector<Result> results;
    for (size_t n : sizes) {
        auto r = run_benchmark(n);
        results.push_back(r);
        std::cout << "N=" << n
                  << "  serial=" << r.serial_us << "us"
                  << "  parallel=" << r.parallel_us << "us"
                  << "  speedup=" << r.serial_us / std::max(r.parallel_us, 1.0) << "x\n";
    }

    // ── write data file ──
    FILE* dat = fopen("bench.dat", "w");
    fprintf(dat, "# elements serial_us parallel_us speedup\n");
    for (auto& r : results)
        fprintf(dat, "%zu %.1f %.1f %.3f\n",
                r.n, r.serial_us, r.parallel_us,
                r.serial_us / std::max(r.parallel_us, 1.0));
    fclose(dat);

    // ── write gnuplot script ──
    FILE* gp = fopen("plot.gp", "w");
    fprintf(gp,
        "set terminal pngcairo size 1200,500 enhanced font 'Sans,11'\n"
        "set output 'benchmark.png'\n"
        "set multiplot layout 1,2\n"

        // left: execution time
        "set title 'Execution Time vs Elements'\n"
        "set xlabel 'Number of Elements'\n"
        "set ylabel 'Time (us)'\n"
        "set logscale x\n"
        "set logscale y\n"
        "set grid\n"
        "set key top left\n"
        "plot 'bench.dat' using 1:2 with linespoints lw 2 pt 7 title 'Serial', \\\n"
        "     'bench.dat' using 1:3 with linespoints lw 2 pt 5 title 'Parallel'\n"

        // right: speedup
        "unset logscale y\n"
        "set title 'Speedup vs Elements'\n"
        "set xlabel 'Number of Elements'\n"
        "set ylabel 'Speedup (serial / parallel)'\n"
        "set yrange [0:*]\n"
        "set key top left\n"
        "plot 'bench.dat' using 1:4 with linespoints lw 2 pt 7 lc rgb '#e74c3c' title 'Speedup', \\\n"
        "     1 with lines lw 1 lc rgb '#888888' dashtype 2 title 'Baseline (1x)'\n"

        "unset multiplot\n"
    );
    fclose(gp);

    // ── run gnuplot ──
    int rc = system("gnuplot plot.gp");
    if (rc == 0)
        std::cout << "\nPlot saved to benchmark.png\n";
    else
        std::cerr << "\ngnuplot failed. Run manually: gnuplot plot.gp\n";

    return 0;
}

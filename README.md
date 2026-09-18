# even-flop

Given two Texas Hold'em starting hands, find the flops that split equity closest
to 50/50.

```
$ even-flop AhKs QdQc --top 5
AhKs vs QdQc
17296 flops evaluated in 34 ms

  #  Flop            AhKs      QdQc      Off
---  ----------  --------  --------  -------
  1  8h 9h Jh      50.20%    49.80%    0.20%
  2  8s 9s Js      50.25%    49.75%    0.25%
  3  8h 9h Th      50.61%    49.39%    0.61%
  4  8s 9s Ts      50.66%    49.34%    0.66%
  5  7s 9s Js      50.86%    49.14%    0.86%
```

## How it works

The search is exhaustive, not sampled — there is no Monte Carlo and no variance
in the answer:

- Two known hands leave 48 unknown cards, so there are `C(48,3)` = **17,296** possible flops.
- Each flop leaves 45 unknown cards, so there are `C(45,2)` = **990** turn/river runouts.
- That is ~17.1M runouts and ~34M seven-card evaluations per matchup.

Every runout is scored for both players; a chop counts as half a win each, so the
two equities always sum to 100%. Flops are ranked by `|hero equity - 50%|`, with
ties broken by card order so runs are byte-for-byte reproducible.

Hand evaluation uses [PokerHandEvaluator](https://github.com/HenryRLee/PokerHandEvaluator)
(Apache-2.0), a perfect-hash evaluator. The flop space is split across worker
threads, each owning a disjoint index range, which keeps the search lock-free and
order-independent. A full matchup takes roughly 35ms on an M4 Pro.

## Build

Requires CMake 3.20+ and a C++23 compiler. Dependencies are fetched automatically.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Test

```sh
./build/tests/even_flop_tests
# or
ctest --test-dir build
```

## Usage

```
even-flop <hand> <hand> [options]

  -n, --top <count>      How many flops to list (default 10)
  -j, --threads <count>  Worker threads (default: one per core)
      --json             Emit JSON instead of a table
  -h, --help             Show usage
```

Hands are two cards each, written as `AhKs` or `"Ah Ks"`. Ranks are
`23456789TJQKA` and suits are `cdhs`, both case-insensitive.

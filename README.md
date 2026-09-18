# even-flop

Given two Texas Hold'em starting hands, find the flops that split equity closest
to 50/50.

There are two front ends over one engine: a web UI and a command-line tool.

## Run the web UI

No build step and no toolchain. The WebAssembly engine is committed, so a fresh
clone runs as-is:

```sh
git clone https://github.com/ayl-duke-dev-28/even-flop.git
cd even-flop
./serve.sh
```

Then open <http://localhost:8000>. Pass a port if 8000 is taken: `./serve.sh 9000`.

The script uses whichever of `python3`, `npx` or `ruby` it finds. If you have
none of them, you don't need a server at all — open `web/index.html` in a
browser directly. The page fetches nothing at runtime, so `file://` works and so
does being offline.

Pick two hands from the card grid and it searches automatically. A full search
takes about 350ms in the browser.

## Run the command-line tool

Needs CMake 3.20+ and a C++23 compiler. Dependencies are fetched automatically.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/even-flop AhKs QdQc --top 5
```

```
AhKs vs QdQc
17296 flops evaluated in 48 ms

Preflop            42.84%    57.16%   (7.16% off even)
Most even flop     50.20%    49.80%   (0.20% off even, on 8h 9h Jh)

  #  Flop            AhKs      QdQc      Off
---  ----------  --------  --------  -------
  1  8h 9h Jh      50.20%    49.80%    0.20%
  2  8s 9s Js      50.25%    49.75%    0.25%
  3  8h 9h Th      50.61%    49.39%    0.61%
  4  8s 9s Ts      50.66%    49.34%    0.66%
  5  7s 9s Js      50.86%    49.14%    0.86%
```

```
even-flop <hand> <hand> [options]

  -n, --top <count>      How many flops to list (default 10)
  -j, --threads <count>  Worker threads (default: one per core)
      --json             Emit JSON instead of a table
  -h, --help             Show usage
```

Hands are two cards each, written as `AhKs` or `"Ah Ks"`. Ranks are
`23456789TJQKA` and suits are `cdhs`, both case-insensitive.

## Reading the output

The `Preflop` line is where the matchup starts and `Most even flop` is the
closest a flop can bring it, so you can see both the size of the gap and how
much of it a flop can close.

Some matchups cannot get close at all. `2c2d` vs `3c3d` starts at 18.89% and the
best any flop manages is 65.25%, overshooting past even, because the deuces have
to flop a set to compete.

## How it works

The search is exhaustive, not sampled — there is no Monte Carlo and no variance
in the answer:

- Two known hands leave 48 unknown cards, so there are `C(48,3)` = **17,296** possible flops.
- Each flop leaves 45 unknown cards, so there are `C(45,2)` = **990** turn/river runouts.
- That is ~17.1M runouts and ~34M seven-card evaluations per matchup.

Every runout is scored for both players; a chop counts as half a win each, so the
two equities always sum to 100%. Flops are ranked by `|hero equity - 50%|`, with
ties broken by card order so runs are byte-for-byte reproducible.

Preflop equity comes out of the same enumeration for free. Each of the `C(48,5)`
= 1,712,304 five-card boards is reachable through exactly `C(5,3)` = 10 distinct
flop/runout splits, and equally so, so summing the per-flop win/loss/tie counts
is an exact preflop equity rather than an estimate. A test cross-checks it
against a direct enumeration of all five-card boards.

Hand evaluation uses [PokerHandEvaluator](https://github.com/HenryRLee/PokerHandEvaluator)
(Apache-2.0), a perfect-hash evaluator. Natively, the flop space is split across
worker threads, each owning a disjoint index range, which keeps the search
lock-free and order-independent: a full matchup takes roughly 35ms on an M4 Pro.
The browser build runs single-threaded in about 350ms.

## Test

```sh
ctest --test-dir build
# or
./build/tests/even_flop_tests
```

67 tests covering card and hand parsing, known-value equity cases, thread-count
determinism, and agreement between the threaded and single-shot paths.

## Rebuilding the WebAssembly engine

Only needed if you change the C++ and want the web UI to pick it up. Requires
[emscripten](https://emscripten.org/):

```sh
./web/build.sh
```

This writes `web/even-flop-engine.js` and `web/even-flop-wasm.js`, both of which
are committed so that running the UI needs no toolchain.

## Layout

```
include/even_flop/   public headers
src/                 engine: card, hand, deck, equity, search
src/cli/             argument parsing and output formatting
src/wasm/            emscripten entry point
tests/               GoogleTest suites
web/                 static UI + the committed WebAssembly build
serve.sh             serves web/ on localhost
```

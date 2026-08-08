# wiki-racer

just a project i made, I wanted to know how the wikipedia race sites work and how there must be an ideal path for all of them.

this was just a way to find out whats happening under the hood.

## what it does

you give it a start article and a target article, and it finds a path between them by hopping from link to link, like the wikipedia race game but automated.

## how it works

- implemented bidirectional bfs for searching from both ends, so one search starts from the start page and another starts from the target page and they meet in the middle. that way the search space stays way smaller than a normal bfs.
- implemented multithreading so we can fetch the links of a bunch of articles at the same time instead of one by one, since every article is basically just an http request.
- built a small graph class for testing the search on made-up graphs before pointing it at real wikipedia.
- caching everywhere so we never fetch the same article twice.

## the wikipedia api

- uses libcurl to hit the wikipedia api and nlohmann/json to parse the responses.
- gets outgoing links with `prop=links` and incoming links with `list=backlinks` (so the backward search has something to walk on).
- handles redirects by resolving the title to its real article first, otherwise you get articles with no links in or out.
- wikipedia rate-limits anonymous requests pretty hard, like 5 requests per ~11 seconds before it starts returning 429 with a `Retry-After` header. so we fetch in small batches of 2, stop the moment both searches meet, and sleep however long wikipedia tells us to when we get blocked.
- paginates with `plcontinue` / `blcontinue` so an article's full link and backlink list is fetched, not just the first 500 the api returns.

## example

searching from `Lebron James` to `Spider-man`:

```text
Lebron James
2019–2020 Hong Kong protests
Hong Kong Disneyland
Spider-man
```

four hops. its interesting that `Lebron James` is a redirect to `LeBron James` and `Spider-man` is to `Spider-Man`, so the redirect resolution is doing real work here to get from the title you typed to the actual articles. also took about 36 seconds because wikipedia API limits.

## benchmarks

there's a benchmark suite in `bench/bench.cpp` (complete vibe slop):

```bash
./build/wiki-racer-bench
```

it runs the search algorithms on made-up graphs (no network) so you can see how they scale, plus a live run against the real wikipedia api. the offline `fetches` column counts node expansions (provider calls) on the synthetic graph, not http requests.

### offline (algorithms on synthetic graphs)

| benchmark                             | fetches | path length | time   |
| ------------------------------------- | ------- | ----------- | ------ |
| bfs, random graph n=20k               | 10939   | 4           | 0.060s |
| bidirectional bfs, random graph n=20k | 27      | 4           | 0.001s |
| bfs, random graph n=60k               | 27893   | 5           | 0.159s |
| bidirectional bfs, random graph n=60k | 29      | 5           | 0.001s |
| bfs, 250x250 grid (corner to corner)  | 62499   | 499         | 0.095s |
| bidirectional bfs, grid               | 61972   | 499         | 0.893s |
| spoke graph, no scorer                | 416     | 4           | 0.012s |
| spoke graph, topic-aware scorer       | 18      | 4           | 0.002s |
| 100x `reverseNeighbours` (10k nodes)  | 100     | -           | 0.395s |

random graphs are directed with 25 outgoing links per node, meant to be roughly wikipedia-shaped. grids are the worst case for bidirectional search, because the two balls expanding from opposite corners each end up covering most of the graph before they meet.

### live (real wikipedia api)

searching `Andrew Garfield` to `LeBron James`:

- path found: `Andrew Garfield -> Sandra Bullock -> LeBron James`
- 9 http requests
- ~25s wall clock
- ~0.37 requests/second

### what the numbers say

- on the random graphs, bidirectional bfs expands far fewer nodes than plain bfs (27 vs 10939 at n=20k), since it searches from both ends.
- the live run is rate-limit bound, not search bound: 9 requests took ~25s and the process was basically idle the whole time. the multithreaded fetching doesn't get you anything here because the throttle serializes requests anyway.
- the per-batch thread spawning does cost something though: on the grid, bidirectional bfs took ~9x longer in wall clock than plain bfs, with about the same number of node expansions.
- the heuristic scoring only mattered on the spoke graph, where it went from 416 fetches to 18. on the grid it changed nothing, since all the final frontier nodes had the same manhattan distance to the goal.

## building

needs cmake, libcurl, nlohmann/json, and googletest on homebrew.

```bash
cmake -B build
cmake --build build
./build/wiki-racer
```

## what's next

- sort each frontier by a heuristic before expanding it, so promising articles get fetched first. right now that's a simple title-word-overlap scorer, and the benchmarks show it cuts fetches a lot when it has a signal.
- maybe something smarter for scoring later, like pulling categories to rank how related an article is to the target.

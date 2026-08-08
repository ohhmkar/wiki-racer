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
- wikipedia rate-limits anonymous requests pretty hard, like 5 requests per ~11 seconds before it starts returning 429 with a `Retry-After` header. so we fetch in small batches of 4, stop the moment both searches meet, and sleep however long wikipedia tells us to when we get blocked.

## example

searching from `Lebron James` to `Spider-man`:

```text
Lebron James
2019–2020 Hong Kong protests
Hong Kong Disneyland
Spider-man
```

four hops. its interesting that `Lebron James` is a redirect and `Spider-man` is too, so the redirect resolution is doing real work here to get from the title you typed to the actual articles. also took about 36 seconds because wikipedia kept telling us to slow down, but it got there.

## building

needs cmake, libcurl, nlohmann/json, and googletest on homebrew.

```bash
cmake -B build
cmake --build build
./build/wiki-racer
```

## what's next

probably some ml/heuristic stuff to pick better next hops than just whatever order the api returns links in.

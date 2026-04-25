# BlindBoxAuctionEstimator

C++17 + Qt + nlohmann/json project for blind-box auction valuation.

The current implementation is phase 1:

- JSON files are loaded once at startup.
- Items, roles, auction rules, and vision config live under `data/`.
- Runtime valuation uses in-memory indexes and dynamic-programming samples.
- The Qt UI currently implements the Victor role input flow.
- OpenCV-related modules are present as extension points for phase 2.

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The build copies `data/` next to the executable.

## Data Notes

`data/items.json` is the authoritative item list. `width`, `height`, `category`, and `shape` can be filled manually later.


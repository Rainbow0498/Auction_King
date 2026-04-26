# BlindBoxAuctionEstimator

C++17 + Qt + nlohmann/json project for blind-box auction valuation.

The current implementation is phase 1:

- JSON files are loaded once at startup.
- Items, roles, auction rules, and vision config live under `data/`.
- `roles.json` drives role selection and color input cards.
- Runtime valuation uses in-memory item indexes.
- The Qt UI implements role selector, dynamic input cards, combination output, and estimate output.
- OpenCV-related modules are present as extension points for phase 2.

## Current Role Flow

`data/roles.json` defines:

- Victor: purple / gold / red high-value analysis.
- Ahmed: white+green / blue / purple / gold / red grouped analysis.
- Raven: direct estimate mode.

The main phase-1 pipeline is:

```text
RoleConfigLoader
  -> RoleSelectorWidget
  -> DynamicInputPanel + ColorInputCard
  -> CombinationSolver
  -> PriceCalculator
  -> CombinationPanel + EstimateResultPanel
```

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The build copies `data/` next to the executable.

## Data Notes

`data/items.json` is the authoritative item list. `width`, `height`, `category`, and `shape` can be filled manually later.

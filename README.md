# LinSpire

Incremental and dynamic linear feasibility solver for large-scale systems.

LinSpire maintains bounds for variables subject to linear (in)equalities and supports fast, incremental updates. It can retract constraints in any order and produces concise conflict explanations when the system becomes infeasible.

## Highlights

- Incremental constraints: add equalities (==), non-strict (<=, >=) and strict (<, >) inequalities on linear expressions.
- Arbitrary retraction: remove any previously added constraint, in any order, and continue solving.
- Conflict explanations: when `check()` finds infeasibility, retrieve a set of constraints that together cause the conflict.

## Build and test

This is a standard CMake project. From the repository root:

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
ctest --output-on-failure
```

This will build the static library and run the unit tests in `tests/`.

## Example

```cpp
#include "linspire.hpp"

int main() {
    linspire::solver s;

    // Create variables
    auto x = s.new_var();
    auto y = s.new_var();

    // Add constraints
    auto c0 = std::make_shared<linspire::constraint>();
    bool res0 = s.new_gt({{x, 1}, {y, 1}}, 1, false, c0); // x + y > 1
    auto c1 = std::make_shared<linspire::constraint>();
    bool res1 = s.new_gt({{x, 1}}, 0, false, c1);         // x > 0
    auto c2 = std::make_shared<linspire::constraint>();
    bool res2 = s.new_gt({{y, 1}}, 0, false, c2);         // y > 0

    // Check satisfiability
    bool cons = s.check(); // cons == true
    if (!cons) {
        auto expl = s.get_conflict(); // expl contains the conflicting constraints

        // Retract a constraint in any order
        s.retract(c0);
        cons = s.check(); // cons == true
    }
    return 0;
}
```
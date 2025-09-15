#include "LinSpire.h"
#include <iostream>

int main() {
    std::cout << "LinSpire - Incremental and dynamic linear feasibility solver" << std::endl;
    std::cout << "Version: " << LinSpire::Solver::getVersion() << std::endl;
    
    LinSpire::Solver solver;
    bool result = solver.solve();
    
    if (result) {
        std::cout << "Solver completed successfully!" << std::endl;
        return 0;
    } else {
        std::cout << "Solver failed!" << std::endl;
        return 1;
    }
}
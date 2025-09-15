#pragma once

namespace LinSpire {
    /**
     * @brief Simple linear feasibility solver interface
     */
    class Solver {
    public:
        /**
         * @brief Initialize the solver
         */
        Solver();
        
        /**
         * @brief Run the solver
         * @return true if successful, false otherwise
         */
        bool solve();
        
        /**
         * @brief Get solver version
         * @return version string
         */
        static const char* getVersion();
    };
}
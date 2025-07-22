#ifndef SIMULATED_ANNEALING_OPTIMIZER_HPP
#define SIMULATED_ANNEALING_OPTIMIZER_HPP
#include <functional>
#include <random>
#include <cmath>
#include <limits>

template<typename State>
class SimulatedAnnealingOptimizer {
public:
    using CostFunction = std::function<double(const State&)>;
    using NeighborFunction = std::function<State(const State&)>;
    using TemperatureSchedule = std::function<double(double, int)>;

    SimulatedAnnealingOptimizer(
        CostFunction costFn,
        NeighborFunction neighborFn,
        double initialTemp,
        double finalTemp,
        int maxIters,
        TemperatureSchedule tempSchedule = defaultSchedule
    )
    : costFn(costFn),
      neighborFn(neighborFn),
      initialTemp(initialTemp),
      finalTemp(finalTemp),
      maxIters(maxIters),
      tempSchedule(tempSchedule)
    {}

    State optimize(const State& initialState) {
        State currState = initialState;
        State bestState = currState;

        double currCost = costFn(currState);
        double bestCost = currCost;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        for (int iter = 0; iter < maxIters; ++iter) {
            double temp = tempSchedule(initialTemp, iter);

            if (temp < finalTemp)
                break;

            State nextState = neighborFn(currState);
            double nextCost = costFn(nextState);
            double delta = nextCost - currCost;

            if (delta < 0 || dis(gen) < std::exp(-delta / temp)) {
                currState = nextState;
                currCost = nextCost;

                if (currCost < bestCost) {
                    bestCost = currCost;
                    bestState = currState;
                }
            }
        }

        return bestState;
    }

private:
    CostFunction costFn;
    NeighborFunction neighborFn;
    double initialTemp;
    double finalTemp;
    int maxIters;
    TemperatureSchedule tempSchedule;

    static double defaultSchedule(double T0, int iter) {
        return T0 * std::pow(0.95, iter); // geometric cooling
    }
};

#endif
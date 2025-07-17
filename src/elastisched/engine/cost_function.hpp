#ifndef COST_FUNCTION_H
#define COST_FUNCTION_H

#include "schedule.hpp"

double busy_saturday_afternoon_cost(Schedule S);
double busy_friday_afternoon_cost(Schedule S);
double work_block_duration_cost(Schedule S);
double daily_work_load_balance(Schedule S);
double bad_sleep_cost(Schedule S);
double context_switch_cost(Schedule S);
double finish_later_cost(Schedule S);
double priority_inversion_cost(Schedule S);

double scheduleCost(Schedule S) {
    return busy_saturday_afternoon_cost(S) 
        + busy_friday_afternoon_cost(S) 
        + work_block_duration_cost(S) 
        + daily_work_load_balance(S)
        + bad_sleep_cost(S)
        + context_switch_cost(S)
        + finish_later_cost(S)
        + priority_inversion_cost(S);
};
#endif
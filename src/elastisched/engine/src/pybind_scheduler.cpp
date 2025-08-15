#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <utility>

namespace py = pybind11;

#include "tag.hpp"
#include "policy.hpp"
#include "job.hpp"
#include "schedule.hpp"
#include "scheduler.hpp"
#include "constants.hpp"
#include "utils/Interval.hpp"

PYBIND11_MODULE(engine, m) {
    // Tag
    py::class_<Tag>(m, "Tag")
        .def(py::init<const std::string&>())
        .def_property("name", &Tag::getName, &Tag::setName)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def("getName", &Tag::getName)
        .def("setName", &Tag::setName)
        .def("__hash__", [](const Tag& tag) { 
            return std::hash<std::string>()(tag.getName()); });

    // Policy
    py::class_<Policy>(m, "Policy")
        .def(py::init<uint8_t, time_t, uint8_t>())
        .def("getMaxSplits", &Policy::getMaxSplits)
        .def("getMinSplitDuration", &Policy::getMinSplitDuration)
        .def("getSchedulingPolicies", &Policy::getSchedulingPolicies)
        .def("isSplittable", &Policy::isSplittable)
        .def("isOverlappable", &Policy::isOverlappable)
        .def("isInvisible", &Policy::isInvisible);

    // TimeRange (Interval<time_t>)
    py::class_<Interval<time_t>>(m, "TimeRange")
        .def(py::init<time_t>())
        .def(py::init<time_t, time_t>())
        .def("getLow", &Interval<time_t>::getLow)
        .def("getHigh", &Interval<time_t>::getHigh)
        .def("overlaps", &Interval<time_t>::overlaps)
        .def("contains", &Interval<time_t>::contains)
        .def("length", &Interval<time_t>::length)
        .def(py::self == py::self)
        .def(py::self != py::self);

    // Job
    py::class_<Job>(m, "Job")
        .def(py::init<time_t, Interval<time_t>, Interval<time_t>, std::string, Policy, std::set<std::string>, std::set<Tag>>())
        .def_readwrite("duration", &Job::duration)
        .def_readwrite("schedulableTimeRange", &Job::schedulableTimeRange)
        .def_readwrite("scheduledTimeRange", &Job::scheduledTimeRange)
        .def_readwrite("id", &Job::id)
        .def_readwrite("policy", &Job::policy)
        .def_readwrite("dependencies", &Job::dependencies)
        .def_readwrite("tags", &Job::tags)
        .def("isRigid", &Job::isRigid)
        .def("__str__", &Job::toString);

    // Schedule
    py::class_<Schedule>(m, "Schedule")
        .def(py::init<std::vector<Job>>(),
             py::arg("scheduledJobs") = std::vector<Job>{})
        .def_readwrite("scheduledJobs", &Schedule::scheduledJobs)
        .def("addJob", &Schedule::addJob)
        .def("clear", &Schedule::clear)
        .def("__len__", [](const Schedule& schedule) { return schedule.scheduledJobs.size(); })
        .def("__iter__", [](const Schedule& schedule) {
            return py::make_iterator(schedule.scheduledJobs.begin(), schedule.scheduledJobs.end());
        }, py::keep_alive<0, 1>());

    // Cost Function
    py::class_<ScheduleCostFunction>(m, "ScheduleCostFunction")
        .def(py::init<const Schedule&, time_t>())
        .def("schedule_cost", &ScheduleCostFunction::scheduleCost)
        .def("busy_saturday_cost", &ScheduleCostFunction::busy_saturday_cost)
        .def("busy_friday_afternoon_cost", &ScheduleCostFunction::busy_friday_afternoon_cost)
        .def("busy_afternoon_exponential_cost", &ScheduleCostFunction::busy_afternoon_exponential_cost,
             py::arg("DAYS_SINCE_MONDAY"))
        .def("busy_day_constant_cost", &ScheduleCostFunction::busy_day_constant_cost,
             py::arg("DAYS_SINCE_MONDAY"));

    m.def("schedule", &schedule, "Run the scheduler with default configurations",
          py::arg("jobs"), py::arg("granularity"));

    m.def("schedule_jobs", &scheduleJobs, "Run the scheduler",
          py::arg("jobs"), py::arg("granularity"), py::arg("initialTemp"), py::arg("finalTemp"), py::arg("numIters"));
} 
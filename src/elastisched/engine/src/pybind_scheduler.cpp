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

PYBIND11_MODULE(scheduler, m) {
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
        .def(py::init<uint8_t, double, uint8_t>())
        .def("getMaxSplits", &Policy::getMaxSplits)
        .def("getMinSplitDuration", &Policy::getMinSplitDuration)
        .def("getSchedulingPolicies", &Policy::getSchedulingPolicies)
        .def("isSplittable", &Policy::isSplittable)
        .def("isOverlappable", &Policy::isOverlappable);

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
        .def("isRigid", &Job::isRigid);

    // Schedule
    py::class_<Schedule>(m, "Schedule")
        .def(py::init<>())
        .def_readwrite("scheduledJobs", &Schedule::scheduledJobs)
        .def("addJob", &Schedule::addJob)
        .def("clear", &Schedule::clear);

    // Expose the schedule function
    m.def("schedule", &schedule, "Run the scheduler",
          py::arg("jobs"), py::arg("num_jobs"), py::arg("granularity"), py::arg("start_epoch"));
} 
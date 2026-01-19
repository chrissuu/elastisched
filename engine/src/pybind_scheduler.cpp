#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <utility>

namespace py = pybind11;

#include "TAG.hpp"
#include "POLICY.hpp"
#include "JOB.hpp"
#include "engine.hpp"
#include "constants.hpp"
#include "INTERVAL.hpp"

PYBIND11_MODULE(engine, m) {
    // Tag
    py::class_<Tag>(m, "Tag")
        .def(py::init<const std::string&, const std::string&>(),
             py::arg("name"),
             py::arg("description") = "")
        .def_property("name", &Tag::get_name, &Tag::set_name)
        .def_property("description", &Tag::get_description, &Tag::set_description)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def("get_name", &Tag::get_name)
        .def("set_name", &Tag::set_name)
        .def("get_description", &Tag::get_description)
        .def("set_description", &Tag::set_description)
        .def("__hash__", [](const Tag& tag) { 
            return std::hash<std::string>()(tag.get_name()); });

    // Policy
    py::class_<Policy>(m, "Policy")
        .def(py::init<>())
        .def(py::init<uint8_t, time_t, uint8_t>(),
             py::arg("max_splits"),
             py::arg("min_split_duration"),
             py::arg("scheduling_policies"))
        .def(py::init<uint8_t, time_t, uint8_t, bool>(),
             py::arg("max_splits"),
             py::arg("min_split_duration"),
             py::arg("scheduling_policies"),
             py::arg("round_to_granularity"))
        .def("get_max_splits", &Policy::get_max_splits)
        .def("get_min_split_duration", &Policy::get_min_split_duration)
        .def("get_round_to_granularity", &Policy::get_round_to_granularity)
        .def("get_scheduling_policies", &Policy::get_scheduling_policies)
        .def("is_splittable", &Policy::is_splittable)
        .def("is_overlappable", &Policy::is_overlappable)
        .def("is_invisible", &Policy::is_invisible);

    // TimeRange (Interval<time_t>)
    py::class_<Interval<time_t>>(m, "TimeRange")
        .def(py::init<time_t>())
        .def(py::init<time_t, time_t>())
        .def("get_low", &Interval<time_t>::get_low)
        .def("get_high", &Interval<time_t>::get_high)
        .def("overlaps", &Interval<time_t>::overlaps)
        .def("contains", &Interval<time_t>::contains)
        .def("length", &Interval<time_t>::length)
        .def(py::self == py::self)
        .def(py::self != py::self);

    // Job
    py::class_<Job>(m, "Job")
        .def(py::init<time_t, Interval<time_t>, Interval<time_t>, std::string, Policy, std::set<std::string>, std::set<Tag>>())
        .def_readwrite("duration", &Job::duration)
        .def_readwrite("schedulable_time_range", &Job::schedulable_time_range)
        .def_readwrite("scheduled_time_range", &Job::scheduled_time_range)
        .def_readwrite("scheduled_time_ranges", &Job::scheduled_time_ranges)
        .def_readwrite("id", &Job::id)
        .def_readwrite("policy", &Job::policy)
        .def_readwrite("dependencies", &Job::dependencies)
        .def_readwrite("tags", &Job::tags)
        .def("is_rigid", &Job::is_rigid)
        .def("__str__", &Job::to_string);

    // Schedule
    py::class_<Schedule>(m, "Schedule")
        .def(py::init<std::vector<Job>>(),
             py::arg("scheduled_jobs") = std::vector<Job>{})
        .def_readwrite("scheduled_jobs", &Schedule::scheduled_jobs)
        .def("add_job", &Schedule::add_job)
        .def("clear", &Schedule::clear)
        .def("__len__", [](const Schedule& schedule) { return schedule.scheduled_jobs.size(); })
        .def("__iter__", [](const Schedule& schedule) {
            return py::make_iterator(schedule.scheduled_jobs.begin(), schedule.scheduled_jobs.end());
        }, py::keep_alive<0, 1>());

    // Cost Function
    py::class_<ScheduleCostFunction>(m, "ScheduleCostFunction")
        .def(py::init<const Schedule&, time_t>())
        .def("schedule_cost", &ScheduleCostFunction::schedule_cost);

    m.def("schedule", &schedule, "Run the scheduler with default configurations",
          py::arg("jobs"), py::arg("granularity"));

    m.def("schedule_jobs", &schedule_jobs, "Run the scheduler",
          py::arg("jobs"), py::arg("granularity"), py::arg("initial_temp"), py::arg("final_temp"), py::arg("num_iters"));
} 

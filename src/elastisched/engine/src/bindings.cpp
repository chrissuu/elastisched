#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include "job.hpp"
#include "schedule.hpp"
#include "scheduler.hpp"
#include "constants.hpp"
#include "policy.hpp"


/**
 * The distinction between blob and job is nuanced, but exists.
 * The Blob class is a wrapper around the CPP Job class. However,
 * it was necessary since non-flexible infinite recurrences cannot be 
 * efficiently defined. 
 */
PYBIND11_MODULE(myscheduler, m) {
    py::class_<Tag>(m, "Tag")
        .def(py::init<>())
        .def_readwrite("name", &Tag::name);

    py::class_<TimeRange>(m, "TimeRange")
        .def(py::init<>())
        .def_readwrite("start", &TimeRange::start)
        .def_readwrite("end", &TimeRange::end);

    py::class_<ScheduledJob>(m, "ScheduledJob")
        .def(py::init<>())
        .def_readwrite("tags", &ScheduledJob::tags)
        .def_readwrite("scheduledTimeRange", &ScheduledJob::scheduledTimeRange)
        .def_readwrite("id", &ScheduledJob::id);

    py::class_<Schedule>(m, "Schedule")
        .def(py::init<>())
        .def_readwrite("scheduledJobs", &Schedule::scheduledJobs);

    m.def("schedule", &schedule, "Run the scheduler",
          py::arg("jobs"), py::arg("num_jobs"), py::arg("granularity"), py::arg("start_epoch"));
}

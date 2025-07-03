# Elastisched
Do YOU struggle with time management? Wish someone were there telling you when to do something? Elastisched might be for you!

## Main Idea
The tenets of Elastisched are the following:

1. Easy to use
2. Has a terminal interface
3. Modular, extensible

The main idea behind elastisched is the "blob" type and the "optimization criterion". 

### Blob
Blob is a generic event/task type which represents a block of time on your calendar.

Three classes extend the base Blob class:

1. Rigid
3. Flexible
4. Invisible

### RigidBlob

RigidBlob represents an event which is fixed and can only be scheduled in the time that it begins and for the duration it lasts for. 
Events such as meetings, birthday parties, classes, etc, could be considered a RigidBlob. 

### FlexibleBlob
A FlexibleBlob represents an event which is not fixed and will be scheduled by the scheduling algorithm within the valid time 
range that the event can be scheduled in. FlexibleBlob has a "splittable" boolean, which represents whether the blob 
can be split apart.

### InvisibleBlob 
An InvisibleBlob represents an event which does not affect the scheduler. This is mostly for interfacing with external APIs, 
or having less intrusive reminders. For instance, birthday reminders, the premier of a movie that is interesting but 
you have no plans to go to, etc.

### Optimization Criterion
The OptimizationCriterion base class forms the main scheduling algorithm. It is left purposely abstract such that one may use
various OptimizationCriterions to create more meaningful scheduling algorithms. 

The OptimziationCriterion class is a function wrapper which can compare two Schedules together. The scheduler uses this comparator
to find the schedule which best fits your taste.

The OptimizationCriterion has a priority level. Two Schedules will be compared using a list of OptimizationCriterions, moving
down this list in terms of priority for tie-breaking. If no further OptimizationCriterions exist for tie-breaking, it is broken
randomly.

### Scheduling
Since Scheduling is overall an NP-complete / NP-hard problem, making this efficient requires interfacing with a C++ backend.
A C++ backend was implemented, compiled to be a .so library and then a frontend was implemented to interface with the C++ backend
using ctypes (To be implemented, as of July 3rd 2025). 

The scheduling does not guarantee the best schedule amongst OptimizationCriterions, but hopes to be a greedy approximation.

The main idea is as follows:

1. All RigidBlobs are automatically placed and blocked in the calendar
2. InvisibleBlobs are filtered and added after the scheduling algorithm runs
3. Problem is now reduced to scheduling FlexibleBlobs with some constraints (from RigidBlobs)
4. Sample a subset of realizations of schedules
5. Split into realizable schedules and un-realizable schedules. A schedule is realizable iff. if Blobs that overlap are Flexible and Splittable
6. Sort by OptimizationCriterion (tie-breaking scheme above)
7. Return best Schedule

## Future Endeavors
1. Dockerization of this application is expected.
2. Integration with common calendars such as GoogleCalendar, Microsoft Outlook, etc.
3. Perhaps more efficient scheduling, although I haven't thought of any optimizations yet.

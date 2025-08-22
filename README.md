# Elastisched
Do YOU struggle with time management? Wish someone were there telling you when to do something? Elastisched might be for you!

## Main Idea
The tenets of Elastisched are the following:

1. Easy to use
2. Has a terminal interface
3. Modular, extensible

The main idea behind elastisched is the "blob" type and the "optimization criterion". 

### Blob
Blob is a generic event/task type which represents a schedulable block of time on your calendar, meaning, what period of time is the scheduler allowed to schedule your task?

A blob may be attached with a policy which allows the blob to have the following properties:

1. Overlappable (can the schedule overlap this blob with another blob?)
2. Invisible (should the schedule ignore scheduling this blob? useful for pure datetimes such as birthdays)
3. Splittable (can the scheduler split the blob into some number of partitions to find a schedule that works around constraints?)

### Optimization Criterion
The Blob's flexibility as well as the Cost Function's expressiveness together perhaps creates a very robust and powerful scheduler. 

The backend optimizer is a simulated annealer, which means that (unless given enough time), it will not converge to the global minima. However, it will have explored enough potential schedules to converge to some good local minima. 

### Scheduling
Since Scheduling is overall an NP-complete / NP-hard problem, making this efficient requires interfacing with a C++ backend. PYBIND with scikit-core-build was used to bind python calls to the C++ scheduler engine API. 

The main idea is as follows:

1. All RigidBlobs are automatically placed and blocked in the calendar
2. InvisibleBlobs are automatically added to the schedule
3. Problem is now reduced to scheduling non-rigid, non-invisible blobs with some constraints
4. Sample a random schedule
5. Do simulated annealing with cost function as defined in cost function class
6. Return best Schedule approximation

### Troubleshooting
1. I tried running unit tests (pytest) or interfacing with the engine API while using conda but got a "GLIB_X.X.XX" not found error. **Try**: forcing conda to use the same gcc and gxx version as when it compiles the library.
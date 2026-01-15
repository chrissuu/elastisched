from functools import lru_cache

"""
utils.py

Some utility functions for helping with integrations to-and-fro
other calendar services.

One of the main utilities here are the edit distance function,
which helps to semantically merge different events together.

The merging events algorithm works as such:

    For all imported calendar events, e, from some calendar A,
    mark them as "Event" types (default_scheduled_tr == e.timerange == 
    schedulable_tr)

    Find overlaps between the elastisched calendar and this event 
    set. Compute the edit distance between the name of the event
    against both the name of the recurrence and the name of
    the blob.

    Take the minimum edit distance and if this is less
    than some threshold, consider them to be the same
    event.
"""

def edit_distance(u, v) -> int:
    """
    Assuming you can add characters, delete characters or edit characters,
    returns the minimum number of operations required to transform u to v.
    """
    @lru_cache(maxsize=len(u) * len(v))
    def EditDistance(i, j):
        if (i == len(u)): return len(v) - j
        if (j == len(v)): return len(u) - i

        if (u[i] == v[j]):
            return EditDistance(i + 1, j + 1)

        cands = [
            EditDistance(i + 1, j), # delete at ind. i from u
            EditDistance(i, j + 1), # delete at ind. j from v
            EditDistance(i + 1, j + 1), # edit to the matching char. at ind. i to u
        ]

        return 1 + min(cands)

    return EditDistance(0, 0)
from enum import Enum

WORK_TAG = "ELASTISCHED_WORK_TYPE"

MINUTE = 60
HOUR = 60 * MINUTE
DAY = 24 * HOUR
WEEK = 7 * DAY

GRANULARITY = 15 * MINUTE

class Day(Enum):
    MONDAY = 0
    TUESDAY = 1
    WEDNESDAY = 2
    THURSDAY = 3
    FRIDAY = 4
    SATURDAY = 5
    SUNDAY = 6

    def __mul__(self, other):
        return self.value * other
    
    def __eq__(self, other):
        return self.value == other
    
    def __lt__(self, other):
        return self.value <= other
    
    def __le__(self, other):
        return self == other or self < other

    def __hash__(self):
        return hash(self.value)


class Hour(Enum):
    TWELVE_AM = 0
    ONE_AM = 1
    TWO_AM = 2
    THREE_AM = 3
    FOUR_AM = 4
    FIVE_AM = 5
    SIX_AM = 6
    SEVEN_AM = 7
    EIGHT_AM = 8
    NINE_AM = 9
    TEN_AM = 10
    ELEVEN_AM = 11
    TWELVE_PM = 12
    ONE_PM = 13
    TWO_PM = 14
    THREE_PM = 15
    FOUR_PM = 16
    FIVE_PM = 17
    SIX_PM = 18
    SEVEN_PM = 19
    EIGHT_PM = 20
    NINE_PM = 21
    TEN_PM = 22
    ELEVEN_PM = 23

    def __mul__(self, other):
        return self.value * other
    
    def __eq__(self, other):
        return self.value == other
    
    def __lt__(self, other):
        return self.value < other
    
    def __le__(self, other):
        return self == other or self < other

    def __hash__(self):
        return hash(self.value)
    
AFTERNOON_START = Hour.FIVE_PM

RANDOM_TEST_ITERATIONS = 30
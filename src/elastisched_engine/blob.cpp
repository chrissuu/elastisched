#pragma once
#include <string>
#include <optional>
#include <chrono>
#include <memory>

// Forward declarations for dependencies
class Policy;
class TimeRange;
enum class PolicyType;

// TimeRange class definition
class TimeRange {
public:
    std::chrono::utc_clock::time_point start;
    std::chrono::utc_clock::time_point end;
    
    // Default constructor with default dates
    TimeRange() : start(getDefaultStartDate()), end(getDefaultEndDate()) {}
    
    // Constructor with specific start and end times
    TimeRange(const std::chrono::utc_clock::time_point& start_time, 
              const std::chrono::utc_clock::time_point& end_time)
        : start(start_time), end(end_time) {}
    
private:
    // Helper methods for default dates (you'll need to implement these)
    std::chrono::utc_clock::time_point getDefaultStartDate() const;
    std::chrono::utc_clock::time_point getDefaultEndDate() const;
};

// Policy class (simplified - you'll need to implement based on your needs)
class Policy {
public:
    PolicyType type;
    
    // Default constructor with FLEXIBLE policy
    Policy() : type(PolicyType::FLEXIBLE) {}
    
    explicit Policy(PolicyType policy_type) : type(policy_type) {}
};

// Blob class - core scheduling unit representing a task/event
class Blob {
public:
    std::optional<TimeRange> default_scheduled_timerange;
    std::optional<TimeRange> schedulable_timerange;
    std::string name;
    std::optional<std::string> description;
    std::string tz;  // Using string for timezone (could use std::chrono::time_zone* in C++20)
    Policy policy;
    
    // Default constructor
    Blob() 
        : default_scheduled_timerange(std::nullopt),
          schedulable_timerange(std::nullopt),
          name("Unnamed Blob"),
          description(std::nullopt),
          tz(getDefaultTimezone()),
          policy(Policy(PolicyType::FLEXIBLE)) {}
    
    // Constructor with all parameters
    Blob(const std::optional<TimeRange>& default_scheduled,
         const std::optional<TimeRange>& schedulable,
         const std::string& blob_name = "Unnamed Blob",
         const std::optional<std::string>& desc = std::nullopt,
         const std::string& timezone = "",
         const Policy& blob_policy = Policy(PolicyType::FLEXIBLE))
        : default_scheduled_timerange(default_scheduled),
          schedulable_timerange(schedulable),
          name(blob_name),
          description(desc),
          tz(timezone.empty() ? getDefaultTimezone() : timezone),
          policy(blob_policy) {}
    
    // Copy constructor
    Blob(const Blob& other) = default;
    
    // Assignment operator
    Blob& operator=(const Blob& other) = default;
    
    // Move constructor
    Blob(Blob&& other) noexcept = default;
    
    // Move assignment operator
    Blob& operator=(Blob&& other) noexcept = default;
    
    // Destructor
    ~Blob() = default;
    
private:
    // Helper method to get default timezone
    std::string getDefaultTimezone() const {
        // You'll need to implement this based on your DEFAULT_TZ
        return "UTC";  // placeholder
    }
};

// You'll need to define PolicyType enum somewhere
enum class PolicyType {
    FLEXIBLE,
    STRICT,
    // Add other policy types as needed
};
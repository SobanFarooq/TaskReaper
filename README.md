# TaskReaper
Task Reaper is a Console Based CPP application that features baseline To-Do App incorporating basic to intermediate features. This is intended to be a semester project for Air University Islamabad.

# Team
**Soban Farooq** - 241843 (Lead)
**Shahrukh Kaleem** - 241828
**Haider** - 241849

## Overview

This is a sophisticated console-based To-Do List application written in C++ that demonstrates advanced programming concepts including object-oriented design, file I/O, data validation, and user management. The application supports multiple users, recurring tasks, task prioritization, search functionality, statistics, and undo operations.

## System Architecture

### Class Hierarchy and Design

The application follows a modular design with five main classes:

1. **DateUtils** - Utility class for date operations
2. **Task** - Core task entity with all task-related properties
3. **User** - User management and authentication
4. **Action** - Command pattern implementation for undo functionality
5. **ToDoList** - Main application controller managing all operations

## Detailed Class Documentation

### DateUtils Class

```cpp
class DateUtils {
public:
    static bool isValidDate(const string& date);
    static string getCurrentDate();
    static bool isOverdue(const string& dueDate);
    static bool isDueSoon(const string& dueDate, int days = 3);
};
```

**Purpose**: Provides date validation and comparison utilities.

**Key Methods**:
- `isValidDate()`: Validates date format (YYYY-MM-DD) and checks logical constraints
- `getCurrentDate()`: Returns current system date in standardized format
- `isOverdue()`: Checks if a task's due date has passed
- `isDueSoon()`: Checks if a task is due within specified days (default: 3)

**Features**:
- Uses regex for date format validation
- Handles month-specific day validation (30/31 days, February)
- Thread-safe static methods
- Consistent date format across the application

### Task Class

```cpp
class Task {
private:
    static int nextId;
    int id;
    string description;
    bool isCompleted;
    int priority;
    string dueDate;
    string category;
    bool isRecurring;
    string recurringType;
public:
    // Constructor, getters, setters, and utility methods
};
```

**Purpose**: Represents a single task with all its properties and behaviors.

**Key Features**:
- **Auto-incrementing ID**: Each task gets a unique identifier
- **Priority System**: 1 (High), 2 (Medium), 3 (Low)
- **Category Support**: Allows task organization
- **Recurring Tasks**: Supports daily, weekly, and monthly recurrence
- **Status Tracking**: Completed/pending status with overdue detection

**Core Methods**:
- `createNextOccurrence()`: Creates new task instance for recurring tasks
- `isOverdue()` / `isDueSoon()`: Status checking using DateUtils
- `getPriorityString()`: Human-readable priority representation

### User Class

```cpp
class User {
private:
    string username;
    string password;
public:
    User();
    User(const string& user, const string& pass);
    string getUsername() const;
    bool authenticate(const string& pass) const;
};
```

**Purpose**: Manages user credentials and authentication.

**Security Note**: The current implementation stores passwords in plain text. In a production environment, passwords should be hashed using secure algorithms like bcrypt or SHA-256.

### Action Class

```cpp
class Action {
public:
    enum Type { ADD, DELETE, COMPLETE, EDIT };
    Type type;
    Task task;
    int index;
};
```

**Purpose**: Implements the Command Pattern for undo functionality.

**Design Pattern**: Command Pattern - encapsulates operations as objects, enabling undo functionality.

### ToDoList Class (Main Controller)

The central controller managing all application operations:

**Key Responsibilities**:
- User session management
- Task CRUD operations
- File I/O operations
- Search and filtering
- Statistics generation
- Undo functionality

## Core Features Documentation

### User Management System

**Registration Process**:
1. Username uniqueness validation
2. User object creation
3. Storage in users map
4. Persistent storage in `users.txt`

**Login Process**:
1. Credential verification
2. Session establishment
3. User-specific task loading
4. Welcome reminders display

**Session Management**:
- Single active user session
- Automatic data saving on logout
- Clean session termination

### Task Management Operations

#### Adding Tasks
```
Flow: Input validation → Task creation → History logging → File persistence
```

**Validation Rules**:
- Description: Non-empty string
- Priority: Integer 1-3
- Due Date: YYYY-MM-DD format with logical validation
- Category: Optional (defaults to "General")

#### Task Editing
**Editable Properties**:
- Description
- Priority level
- Due date
- Category

**Process**:
1. Display current tasks
2. User selects task by number
3. User chooses property to edit
4. Input validation
5. History logging for undo
6. Update and save

#### Task Completion
**Regular Tasks**: Simple status update
**Recurring Tasks**: 
1. Mark current instance complete
2. Generate next occurrence
3. Add new occurrence to task list

### Search and Filtering System

**Search Categories**:
1. **Description Search**: Case-insensitive keyword matching
2. **Category Filter**: Exact category matching
3. **Priority Filter**: Specific priority level
4. **Status Filter**: Completed/Pending/Overdue

**Implementation**: Linear search through task vector with custom predicates.

### Statistics and Analytics

**Metrics Provided**:
- Task completion percentage
- Priority distribution
- Category-wise task count
- Status breakdown (completed/pending/overdue)

**Calculation Method**: Single-pass iteration through tasks with counters.

### Reminder System

**Alert Categories**:
- **Overdue Tasks**: Past due date, marked with ⚠️
- **Due Soon**: Within 3 days, marked with ⏰

**Display Timing**: Automatically shown on login and available on-demand.

### Undo Functionality

**Supported Operations**:
- Task addition/deletion
- Task completion
- Task editing

**Implementation**:
- Maintains action history stack (max 10 operations)
- Stores complete task state for restoration
- LIFO (Last In, First Out) operation order

## Data Persistence

### File Storage Format

**User Data** (`users.txt`):
```
username|encrypted_password
```

**Task Data** (`{username}_tasks.txt`):
```
id|description|isCompleted|priority|dueDate|category|isRecurring|recurringType
```

**Benefits**:
- Human-readable format
- Easy debugging and manual editing
- Pipe-separated values for reliable parsing

### File I/O Operations

**Save Process**:
1. Open file in write mode
2. Iterate through data structure
3. Format and write each record
4. Close file handle

**Load Process**:
1. Check file existence
2. Parse each line with string tokenization
3. Reconstruct objects
4. Populate data structures

## User Interface Design

### Menu System

**Two-tier Menu Structure**:
1. **Login Menu**: Authentication and registration
2. **Main Menu**: Application functionality

**Menu Flow**:
```
Login/Register → Main Menu → Feature Selection → User Input → Result Display → Return to Menu
```

### Input Validation

**Validation Strategies**:
- **Numeric Input**: While loops with error clearing
- **Date Input**: Regex validation with format checking
- **Range Validation**: Bounds checking for priorities and menu choices

**Error Handling**:
- Clear error messages
- Input stream clearing on invalid input
- Retry prompts for user correction

## Technical Implementation Details

### Memory Management

**Container Usage**:
- `vector`: Dynamic task storage
- `map`: User lookup by username
- `vector`: Undo history stack

**Memory Efficiency**:
- Object copying for undo functionality
- Automatic cleanup with RAII
- No manual memory allocation

### String Processing

**Key Techniques**:
- `getline()` for multi-word input
- `istringstream` for parsing
- `transform()` for case-insensitive search
- Regular expressions for date validation

### Date Handling

**System Integration**:
- Uses `` library for system date
- Manual date arithmetic for recurrence
- String-based date comparison
- Timezone consideration (local time)

## Compilation and Execution

### Required Headers
```cpp
#include <iostream>     // I/O operations
#include <vector>       // Dynamic arrays
#include <string>       // String handling
#include <fstream>      // File operations
#include <sstream>      // String streams
#include <iomanip>      // I/O formatting
#include <algorithm>    // STL algorithms
#include <map>          // Key-value storage
#include <ctime>        // Date/time functions
#include <regex>        // Regular expressions
```

### Compilation Command
```bash
g++ -std=c++11 main.cpp -o todolist
```

### Execution
```bash
./todolist
```

## Code Quality Analysis

### Strengths

1. **Object-Oriented Design**: Well-structured classes with clear responsibilities
2. **Input Validation**: Comprehensive validation for all user inputs
3. **Error Handling**: Graceful handling of invalid inputs and edge cases
4. **Modularity**: Separate utility classes for specific functionalities
5. **Persistence**: Reliable file-based data storage
6. **User Experience**: Intuitive menu system with clear feedback

### Areas for Improvement

1. **Security**: Plain text password storage
2. **Error Handling**: Limited exception handling for file operations
3. **Date Library**: Basic date arithmetic could benefit from modern date libraries
4. **Code Documentation**: Limited inline comments
5. **Testing**: No unit tests included
6. **Configuration**: Hard-coded file names and limits

## Potential Enhancements

### Security Improvements
- Password hashing (bcrypt, SHA-256)
- Session timeout implementation
- Input sanitization for file operations

### Feature Additions
- Task attachments and notes
- Collaborative tasks (task sharing)
- Data export (CSV, JSON)
- Advanced recurring patterns
- Task dependencies
- Mobile/web interface

### Technical Improvements
- Exception handling with try-catch blocks
- Modern C++ features (smart pointers, range-based loops)
- Database integration (SQLite)
- Configuration file support
- Logging system implementation

## Learning Outcomes

This project demonstrates mastery of:

1. **Object-Oriented Programming**: Classes, encapsulation, inheritance concepts
2. **STL Usage**: Vectors, maps, algorithms, iterators
3. **File I/O**: Reading/writing structured data
4. **String Manipulation**: Parsing, formatting, validation
5. **Design Patterns**: Command pattern for undo functionality
6. **User Interface Design**: Console-based menu systems
7. **Data Validation**: Input sanitization and error handling
8. **Project Structure**: Multi-class application organization

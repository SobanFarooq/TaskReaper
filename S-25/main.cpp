#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <iomanip>
#include <regex>
using namespace std;

// Utility class for date operations
class DateUtils {
public:
    static bool isValidDate(const string& date) {
        regex datePattern(R"(\d{4}-\d{2}-\d{2})");
        if (!regex_match(date, datePattern)) return false;

        istringstream ss(date);
        string year, month, day;
        getline(ss, year, '-');
        getline(ss, month, '-');
        getline(ss, day, '-');

        int y = stoi(year), m = stoi(month), d = stoi(day);
        if (m < 1 || m > 12 || d < 1 || d > 31) return false;

        // Simple validation for days in month
        if ((m == 4 || m == 6 || m == 9 || m == 11) && d > 30) return false;
        if (m == 2 && d > 29) return false;

        return true;
    }

    static string getCurrentDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        ostringstream oss;
        oss << (1900 + ltm->tm_year) << "-"
            << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
            << setfill('0') << setw(2) << ltm->tm_mday;
        return oss.str();
    }

    static bool isOverdue(const string& dueDate) {
        return dueDate < getCurrentDate();
    }

    static bool isDueSoon(const string& dueDate, int days = 3) {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        ltm->tm_mday += days;
        mktime(ltm); // Normalize the date

        ostringstream oss;
        oss << (1900 + ltm->tm_year) << "-"
            << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
            << setfill('0') << setw(2) << ltm->tm_mday;

        return dueDate <= oss.str() && dueDate >= getCurrentDate();
    }
};

// Enhanced Task class
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
    string recurringType; // "daily", "weekly", "monthly"

public:
    Task(const string& desc, int prio, const string& due, const string& cat = "General")
        : id(nextId++), description(desc), isCompleted(false), priority(prio),
          dueDate(due), category(cat), isRecurring(false), recurringType("") {}

    // Getters
    int getId() const { return id; }
    string getDescription() const { return description; }
    bool getIsCompleted() const { return isCompleted; }
    int getPriority() const { return priority; }
    string getDueDate() const { return dueDate; }
    string getCategory() const { return category; }
    bool getIsRecurring() const { return isRecurring; }
    string getRecurringType() const { return recurringType; }

    // Setters
    void setDescription(const string& desc) { description = desc; }
    void setPriority(int prio) { priority = prio; }
    void setDueDate(const string& due) { dueDate = due; }
    void setCategory(const string& cat) { category = cat; }
    void markCompleted() { isCompleted = true; }
    void markPending() { isCompleted = false; }
    void setRecurring(const string& type) {
        isRecurring = true;
        recurringType = type;
    }

    string getPriorityString() const {
        switch (priority) {
            case 1: return "High";
            case 2: return "Medium";
            case 3: return "Low";
            default: return to_string(priority);
        }
    }

    bool isOverdue() const {
        return !isCompleted && DateUtils::isOverdue(dueDate);
    }

    bool isDueSoon() const {
        return !isCompleted && DateUtils::isDueSoon(dueDate);
    }

    // Create next occurrence for recurring tasks
    Task createNextOccurrence() const {
        if (!isRecurring) return *this;

        Task nextTask = *this;
        nextTask.id = nextId++;
        nextTask.isCompleted = false;

        // Simple date advancement (basic implementation)
        time_t now = time(0);
        tm* ltm = localtime(&now);

        if (recurringType == "daily") {
            ltm->tm_mday += 1;
        } else if (recurringType == "weekly") {
            ltm->tm_mday += 7;
        } else if (recurringType == "monthly") {
            ltm->tm_mon += 1;
        }

        mktime(ltm);
        ostringstream oss;
        oss << (1900 + ltm->tm_year) << "-"
            << setfill('0') << setw(2) << (1 + ltm->tm_mon) << "-"
            << setfill('0') << setw(2) << ltm->tm_mday;
        nextTask.dueDate = oss.str();

        return nextTask;
    }
};

int Task::nextId = 1;

// User class for multi-user support
class User {
private:
    string username;
    string password;

public:
    User() : username(""), password("") {} // Default constructor
    User(const string& user, const string& pass) : username(user), password(pass) {}

    string getUsername() const { return username; }
    bool authenticate(const string& pass) const { return password == pass; }
};

// Action class for undo functionality
class Action {
public:
    enum Type { ADD, DELETE, COMPLETE, EDIT };
    Type type;
    Task task;
    int index;

    Action(Type t, const Task& tsk, int idx = -1) : type(t), task(tsk), index(idx) {}
};

// Enhanced ToDoList class
class ToDoList {
private:
    vector<Task> tasks;
    vector<Action> actionHistory;
    map<string, User> users;
    string currentUser;
    const int MAX_HISTORY = 10;

public:
    ToDoList() {
        loadUsers();
    }

    ~ToDoList() {
        saveUsers();
    }

    // User management
    bool registerUser(const string& username, const string& password) {
        if (users.find(username) != users.end()) {
            return false; // User already exists
        }
        users.insert(make_pair(username, User(username, password)));
        return true;
    }

    bool loginUser(const string& username, const string& password) {
        auto it = users.find(username);
        if (it != users.end() && it->second.authenticate(password)) {
            currentUser = username;
            loadTasks();
            return true;
        }
        return false;
    }

    void logoutUser() {
        if (!currentUser.empty()) {
            saveTasks();
            currentUser = "";
            tasks.clear();
            actionHistory.clear();
        }
    }

    bool isLoggedIn() const { return !currentUser.empty(); }
    string getCurrentUser() const { return currentUser; }

    // Enhanced task management
    void addTask() {
        string desc, due, category;
        int prio;
        char recurring;

        cout << "Enter task description: ";
        getline(cin >> ws, desc);

        // Priority validation
        do {
            cout << "Enter priority (1 = High, 2 = Medium, 3 = Low): ";
            while (!(cin >> prio) || prio < 1 || prio > 3) {
                cout << "Invalid input. Please enter 1, 2, or 3: ";
                cin.clear();
                cin.ignore(10000, '\n');
            }
        } while (prio < 1 || prio > 3);

        cin.ignore();

        // Date validation
        do {
            cout << "Enter due date (YYYY-MM-DD): ";
            getline(cin, due);
            if (!DateUtils::isValidDate(due)) {
                cout << "Invalid date format. Please use YYYY-MM-DD.\n";
            }
        } while (!DateUtils::isValidDate(due));

        cout << "Enter category (or press Enter for 'General'): ";
        getline(cin, category);
        if (category.empty()) category = "General";

        Task newTask(desc, prio, due, category);

        cout << "Is this a recurring task? (y/n): ";
        cin >> recurring;
        if (recurring == 'y' || recurring == 'Y') {
            int recurType;
            cout << "Recurring type: 1. Daily 2. Weekly 3. Monthly: ";
            cin >> recurType;
            switch (recurType) {
                case 1: newTask.setRecurring("daily"); break;
                case 2: newTask.setRecurring("weekly"); break;
                case 3: newTask.setRecurring("monthly"); break;
            }
        }

        tasks.push_back(newTask);
        addToHistory(Action::ADD, newTask);
        cout << "Task added successfully!\n";
    }

    void editTask() {
        if (tasks.empty()) {
            cout << "No tasks to edit.\n";
            return;
        }

        displayTasks();
        int idx;
        cout << "Enter task number to edit: ";
        cin >> idx;

        if (idx < 1 || idx > (int)tasks.size()) {
            cout << "Invalid task number.\n";
            return;
        }

        Task oldTask = tasks[idx - 1];

        cout << "What would you like to edit?\n";
        cout << "1. Description\n2. Priority\n3. Due Date\n4. Category\n";
        cout << "Choose option: ";

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                string newDesc;
                cout << "Enter new description: ";
                getline(cin, newDesc);
                tasks[idx - 1].setDescription(newDesc);
                break;
            }
            case 2: {
                int newPrio;
                do {
                    cout << "Enter new priority (1-3): ";
                    cin >> newPrio;
                } while (newPrio < 1 || newPrio > 3);
                tasks[idx - 1].setPriority(newPrio);
                break;
            }
            case 3: {
                string newDate;
                do {
                    cout << "Enter new due date (YYYY-MM-DD): ";
                    cin >> newDate;
                } while (!DateUtils::isValidDate(newDate));
                tasks[idx - 1].setDueDate(newDate);
                break;
            }
            case 4: {
                string newCategory;
                cout << "Enter new category: ";
                getline(cin, newCategory);
                tasks[idx - 1].setCategory(newCategory);
                break;
            }
            default:
                cout << "Invalid option.\n";
                return;
        }

        addToHistory(Action::EDIT, oldTask, idx - 1);
        cout << "Task updated successfully!\n";
    }

    void searchTasks() {
        if (tasks.empty()) {
            cout << "No tasks to search.\n";
            return;
        }

        cout << "Search by: 1. Description 2. Category 3. Priority 4. Status\n";
        cout << "Choose option: ";
        int choice;
        cin >> choice;
        cin.ignore();

        vector<int> results;

        switch (choice) {
            case 1: {
                string keyword;
                cout << "Enter keyword: ";
                getline(cin, keyword);
                transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);

                for (size_t i = 0; i < tasks.size(); ++i) {
                    string desc = tasks[i].getDescription();
                    transform(desc.begin(), desc.end(), desc.begin(), ::tolower);
                    if (desc.find(keyword) != string::npos) {
                        results.push_back(i);
                    }
                }
                break;
            }
            case 2: {
                string category;
                cout << "Enter category: ";
                getline(cin, category);

                for (size_t i = 0; i < tasks.size(); ++i) {
                    if (tasks[i].getCategory() == category) {
                        results.push_back(i);
                    }
                }
                break;
            }
            case 3: {
                int priority;
                cout << "Enter priority (1-3): ";
                cin >> priority;

                for (size_t i = 0; i < tasks.size(); ++i) {
                    if (tasks[i].getPriority() == priority) {
                        results.push_back(i);
                    }
                }
                break;
            }
            case 4: {
                cout << "Status: 1. Completed 2. Pending 3. Overdue\n";
                int status;
                cin >> status;

                for (size_t i = 0; i < tasks.size(); ++i) {
                    if (status == 1 && tasks[i].getIsCompleted()) results.push_back(i);
                    else if (status == 2 && !tasks[i].getIsCompleted() && !tasks[i].isOverdue()) results.push_back(i);
                    else if (status == 3 && tasks[i].isOverdue()) results.push_back(i);
                }
                break;
            }
        }

        if (results.empty()) {
            cout << "No matching tasks found.\n";
        } else {
            cout << "\nSearch Results:\n";
            for (int idx : results) {
                displayTask(idx);
            }
        }
    }

    void showStatistics() {
        if (tasks.empty()) {
            cout << "No tasks to analyze.\n";
            return;
        }

        int completed = 0, pending = 0, overdue = 0;
        int high = 0, medium = 0, low = 0;
        map<string, int> categoryCount;

        for (const auto& task : tasks) {
            if (task.getIsCompleted()) completed++;
            else if (task.isOverdue()) overdue++;
            else pending++;

            switch (task.getPriority()) {
                case 1: high++; break;
                case 2: medium++; break;
                case 3: low++; break;
            }

            categoryCount[task.getCategory()]++;
        }

        cout << "\n===== Task Statistics =====\n";
        cout << "Total Tasks: " << tasks.size() << "\n";
        cout << "Completed: " << completed << " (" << (completed * 100 / (int)tasks.size()) << "%)\n";
        cout << "Pending: " << pending << "\n";
        cout << "Overdue: " << overdue << "\n\n";

        cout << "Priority Distribution:\n";
        cout << "High: " << high << ", Medium: " << medium << ", Low: " << low << "\n\n";

        cout << "Tasks by Category:\n";
        for (const auto& pair : categoryCount) {
            cout << pair.first << ": " << pair.second << "\n";
        }
    }

    void showReminders() {
        cout << "\n===== Reminders =====\n";

        vector<int> overdue, dueSoon;
        for (size_t i = 0; i < tasks.size(); ++i) {
            if (tasks[i].isOverdue()) overdue.push_back(i);
            else if (tasks[i].isDueSoon()) dueSoon.push_back(i);
        }

        if (!overdue.empty()) {
            cout << "OVERDUE TASKS:\n";
            for (int idx : overdue) {
                cout << "⚠️  ";
                displayTask(idx);
            }
        }

        if (!dueSoon.empty()) {
            cout << "\nDUE SOON (next 3 days):\n";
            for (int idx : dueSoon) {
                cout << "⏰ ";
                displayTask(idx);
            }
        }

        if (overdue.empty() && dueSoon.empty()) {
            cout << "No urgent tasks. Great job! 🎉\n";
        }
    }

    void undoLastAction() {
        if (actionHistory.empty()) {
            cout << "No actions to undo.\n";
            return;
        }

        Action lastAction = actionHistory.back();
        actionHistory.pop_back();

        switch (lastAction.type) {
            case Action::ADD:
                // Remove the last added task
                for (auto it = tasks.begin(); it != tasks.end(); ++it) {
                    if (it->getId() == lastAction.task.getId()) {
                        tasks.erase(it);
                        break;
                    }
                }
                cout << "Task addition undone.\n";
                break;

            case Action::DELETE:
                // Re-add the deleted task
                if (lastAction.index >= 0 && lastAction.index <= (int)tasks.size()) {
                    tasks.insert(tasks.begin() + lastAction.index, lastAction.task);
                    cout << "Task deletion undone.\n";
                }
                break;

            case Action::COMPLETE:
                // Mark task as pending
                for (auto& task : tasks) {
                    if (task.getId() == lastAction.task.getId()) {
                        task.markPending();
                        cout << "Task completion undone.\n";
                        break;
                    }
                }
                break;

            case Action::EDIT:
                // Restore previous version
                if (lastAction.index >= 0 && lastAction.index < (int)tasks.size()) {
                    tasks[lastAction.index] = lastAction.task;
                    cout << "Task edit undone.\n";
                }
                break;
        }
    }

    // Display methods
    void displayTask(size_t index) const {
        if (index >= tasks.size()) return;

        const Task& task = tasks[index];
        cout << index + 1 << ". [" << (task.getIsCompleted() ? "✓" : " ") << "] ";
        cout << task.getDescription();
        cout << " (Priority: " << task.getPriorityString();
        cout << ", Due: " << task.getDueDate();
        cout << ", Category: " << task.getCategory();
        if (task.getIsRecurring()) {
            cout << ", Recurring: " << task.getRecurringType();
        }
        if (task.isOverdue()) cout << " - OVERDUE!";
        else if (task.isDueSoon()) cout << " - Due Soon!";
        cout << ")\n";
    }

    void displayTasks() const {
        if (tasks.empty()) {
            cout << "No tasks to display.\n";
            return;
        }
        cout << "\nTo-Do List for " << currentUser << ":\n";
        for (size_t i = 0; i < tasks.size(); ++i) {
            displayTask(i);
        }
    }

    void displayTasksSortedByDueDate() const {
        if (tasks.empty()) {
            cout << "No tasks to display.\n";
            return;
        }
        vector<Task> sortedTasks = tasks;
        sort(sortedTasks.begin(), sortedTasks.end(), [](const Task& a, const Task& b) {
            return a.getDueDate() < b.getDueDate();
        });

        cout << "\nTo-Do List (Sorted by Due Date):\n";
        for (size_t i = 0; i < sortedTasks.size(); ++i) {
            cout << i + 1 << ". [" << (sortedTasks[i].getIsCompleted() ? "✓" : " ") << "] ";
            cout << sortedTasks[i].getDescription();
            cout << " (Priority: " << sortedTasks[i].getPriorityString();
            cout << ", Due: " << sortedTasks[i].getDueDate();
            cout << ", Category: " << sortedTasks[i].getCategory() << ")\n";
        }
    }

    void displayTasksSortedByPriority() const {
        if (tasks.empty()) {
            cout << "No tasks to display.\n";
            return;
        }
        vector<Task> sortedTasks = tasks;
        sort(sortedTasks.begin(), sortedTasks.end(), [](const Task& a, const Task& b) {
            return a.getPriority() < b.getPriority();
        });

        cout << "\nTo-Do List (Sorted by Priority):\n";
        for (size_t i = 0; i < sortedTasks.size(); ++i) {
            cout << i + 1 << ". [" << (sortedTasks[i].getIsCompleted() ? "✓" : " ") << "] ";
            cout << sortedTasks[i].getDescription();
            cout << " (Priority: " << sortedTasks[i].getPriorityString();
            cout << ", Due: " << sortedTasks[i].getDueDate();
            cout << ", Category: " << sortedTasks[i].getCategory() << ")\n";
        }
    }

    void deleteTask() {
        if (tasks.empty()) {
            cout << "No tasks to delete.\n";
            return;
        }
        displayTasks();
        int idx;
        cout << "Enter the task number to delete: ";
        cin >> idx;
        if (idx < 1 || idx > (int)tasks.size()) {
            cout << "Invalid task number.\n";
            return;
        }

        Task deletedTask = tasks[idx - 1];
        tasks.erase(tasks.begin() + idx - 1);
        addToHistory(Action::DELETE, deletedTask, idx - 1);
        cout << "Task deleted successfully!\n";
    }

    void markTaskCompleted() {
        if (tasks.empty()) {
            cout << "No tasks to mark as completed.\n";
            return;
        }
        displayTasks();
        int idx;
        cout << "Enter the task number to mark as completed: ";
        cin >> idx;
        if (idx < 1 || idx > (int)tasks.size()) {
            cout << "Invalid task number.\n";
            return;
        }

        Task oldTask = tasks[idx - 1];
        tasks[idx - 1].markCompleted();
        addToHistory(Action::COMPLETE, oldTask);

        // Handle recurring tasks
        if (tasks[idx - 1].getIsRecurring()) {
            Task nextTask = tasks[idx - 1].createNextOccurrence();
            tasks.push_back(nextTask);
            cout << "Task marked as completed! Next occurrence created.\n";
        } else {
            cout << "Task marked as completed!\n";
        }
    }

private:
    void addToHistory(Action::Type type, const Task& task, int index = -1) {
        actionHistory.push_back(Action(type, task, index));
        if (actionHistory.size() > MAX_HISTORY) {
            actionHistory.erase(actionHistory.begin());
        }
    }

    void saveTasks() {
        if (currentUser.empty()) return;

        string filename = currentUser + "_tasks.txt";
        ofstream file(filename);
        if (file.is_open()) {
            for (const auto& task : tasks) {
                file << task.getId() << "|" << task.getDescription() << "|"
                     << task.getIsCompleted() << "|" << task.getPriority() << "|"
                     << task.getDueDate() << "|" << task.getCategory() << "|"
                     << task.getIsRecurring() << "|" << task.getRecurringType() << "\n";
            }
            file.close();
        }
    }

    void loadTasks() {
        if (currentUser.empty()) return;

        tasks.clear();
        string filename = currentUser + "_tasks.txt";
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                istringstream ss(line);
                string token;
                vector<string> tokens;

                while (getline(ss, token, '|')) {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 6) {
                    Task task(tokens[1], stoi(tokens[3]), tokens[4], tokens[5]);
                    if (tokens[2] == "1") task.markCompleted();
                    if (tokens.size() >= 8 && tokens[6] == "1") {
                        task.setRecurring(tokens[7]);
                    }
                    tasks.push_back(task);
                }
            }
            file.close();
        }
    }

    void saveUsers() {
        ofstream file("users.txt");
        if (file.is_open()) {
            for (const auto& pair : users) {
                if (!pair.second.getUsername().empty()) {
                    file << pair.second.getUsername() << "|" << "encrypted_password" << "\n";
                }
            }
            file.close();
        }
    }

    void loadUsers() {
        ifstream file("users.txt");
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                istringstream ss(line);
                string username, password;
                getline(ss, username, '|');
                getline(ss, password, '|');
                users.insert(make_pair(username, User(username, password)));
            }
            file.close();
        }
    }
};

// Main menu functions
void displayLoginMenu() {
    cout << "\n===== Welcome to Enhanced To-Do List =====\n";
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "3. Exit\n";
    cout << "Choose an option: ";
}

void displayMainMenu() {
    cout << "\n===== Enhanced To-Do List Menu =====\n";
    cout << "1. Add Task\n";
    cout << "2. Display Tasks\n";
    cout << "3. Edit Task\n";
    cout << "4. Delete Task\n";
    cout << "5. Mark Task as Completed\n";
    cout << "6. Search/Filter Tasks\n";
    cout << "7. Display Tasks Sorted by Due Date\n";
    cout << "8. Display Tasks Sorted by Priority\n";
    cout << "9. Task Statistics\n";
    cout << "10. Show Reminders\n";
    cout << "11. Undo Last Action\n";
    cout << "12. Logout\n";
    cout << "13. Exit\n";
    cout << "Choose an option: ";
}

int main() {
    ToDoList todo;
    int choice;

    cout << "🚀 Welcome to the Enhanced C++ To-Do List Application!\n";

    // Login/Registration loop
    while (!todo.isLoggedIn()) {
        displayLoginMenu();
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                string username, password;
                cout << "Username: ";
                getline(cin, username);
                cout << "Password: ";
                getline(cin, password);

                if (todo.loginUser(username, password)) {
                    cout << "Login successful! Welcome back, " << username << "!\n";
                    todo.showReminders(); // Show reminders on login
                } else {
                    cout << "Invalid credentials. Please try again.\n";
                }
                break;
            }
            case 2: {
                string username, password;
                cout << "Choose username: ";
                getline(cin, username);
                cout << "Choose password: ";
                getline(cin, password);

                if (todo.registerUser(username, password)) {
                    cout << "Registration successful! Please login.\n";
                } else {
                    cout << "Username already exists. Please choose another.\n";
                }
                break;
            }
            case 3:
                cout << "Thank you for using Enhanced To-Do List! Goodbye!\n";
                return 0;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    }

    // Main application loop
    do {
        displayMainMenu();
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: todo.addTask(); break;
            case 2: todo.displayTasks(); break;
            case 3: todo.editTask(); break;
            case 4: todo.deleteTask(); break;
            case 5: todo.markTaskCompleted(); break;
            case 6: todo.searchTasks(); break;
            case 7: todo.displayTasksSortedByDueDate(); break;
            case 8: todo.displayTasksSortedByPriority(); break;
            case 9: todo.showStatistics(); break;
            case 10: todo.showReminders(); break;
            case 11: todo.undoLastAction(); break;
            case 12:
                todo.logoutUser();
                cout << "Logged out successfully. Goodbye!\n";
                return 0;
            case 13:
                cout << "Saving data and exiting...\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 13);

    return 0;
}
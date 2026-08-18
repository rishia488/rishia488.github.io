//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Arishia Jackson
// Course      : CS-300 / CS-499 Enhancement Two
// Description : Advising Assistance Program using a custom hash table
//============================================================================

#include <algorithm>
#include <chrono>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

string trim(const string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

string toUpperCase(string value) {
    transform(value.begin(), value.end(), value.begin(),
              [](unsigned char ch) { return static_cast<char>(toupper(ch)); });
    return value;
}

string normalizeCourseNumber(const string& value) {
    return toUpperCase(trim(value));
}

bool isValidCourseNumber(const string& value) {
    if (value.size() < 5 || value.size() > 12) {
        return false;
    }

    bool hasLetter = false;
    bool hasDigit = false;
    for (unsigned char ch : value) {
        if (isalpha(ch)) {
            hasLetter = true;
        } else if (isdigit(ch)) {
            hasDigit = true;
        } else {
            return false;
        }
    }
    return hasLetter && hasDigit;
}

class CourseHashTable {
private:
    static constexpr size_t DEFAULT_BUCKET_COUNT = 101;
    vector<list<Course>> buckets;
    size_t courseCount = 0;

    size_t hashKey(const string& key) const {
        unsigned long hash = 5381;
        for (unsigned char ch : key) {
            hash = ((hash << 5) + hash) + ch;
        }
        return hash % buckets.size();
    }

public:
    explicit CourseHashTable(size_t bucketCount = DEFAULT_BUCKET_COUNT)
        : buckets(max<size_t>(bucketCount, 1)) {}

    void clear() {
        for (auto& bucket : buckets) {
            bucket.clear();
        }
        courseCount = 0;
    }

    bool insert(const Course& course) {
        const size_t index = hashKey(course.courseNumber);
        for (const Course& existing : buckets[index]) {
            if (existing.courseNumber == course.courseNumber) {
                return false;
            }
        }
        buckets[index].push_back(course);
        ++courseCount;
        return true;
    }

    Course* find(const string& courseNumber) {
        const size_t index = hashKey(courseNumber);
        for (Course& course : buckets[index]) {
            if (course.courseNumber == courseNumber) {
                return &course;
            }
        }
        return nullptr;
    }

    const Course* find(const string& courseNumber) const {
        const size_t index = hashKey(courseNumber);
        for (const Course& course : buckets[index]) {
            if (course.courseNumber == courseNumber) {
                return &course;
            }
        }
        return nullptr;
    }

    bool update(const string& originalNumber, const Course& replacement) {
        if (originalNumber != replacement.courseNumber && find(replacement.courseNumber) != nullptr) {
            return false;
        }
        if (!remove(originalNumber)) {
            return false;
        }
        return insert(replacement);
    }

    bool remove(const string& courseNumber) {
        const size_t index = hashKey(courseNumber);
        auto& bucket = buckets[index];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->courseNumber == courseNumber) {
                bucket.erase(it);
                --courseCount;
                return true;
            }
        }
        return false;
    }

    vector<Course> getAllSorted() const {
        vector<Course> result;
        result.reserve(courseCount);
        for (const auto& bucket : buckets) {
            for (const Course& course : bucket) {
                result.push_back(course);
            }
        }
        sort(result.begin(), result.end(), [](const Course& left, const Course& right) {
            return left.courseNumber < right.courseNumber;
        });
        return result;
    }

    size_t size() const { return courseCount; }
    size_t bucketCount() const { return buckets.size(); }

    size_t usedBucketCount() const {
        return count_if(buckets.begin(), buckets.end(),
                        [](const list<Course>& bucket) { return !bucket.empty(); });
    }

    size_t collisionCount() const {
        size_t collisions = 0;
        for (const auto& bucket : buckets) {
            if (bucket.size() > 1) {
                collisions += bucket.size() - 1;
            }
        }
        return collisions;
    }
};

class CourseManager {
private:
    CourseHashTable courses;
    bool dataLoaded = false;

    static vector<string> splitCsvLine(const string& line) {
        vector<string> fields;
        string field;
        stringstream stream(line);
        while (getline(stream, field, ',')) {
            fields.push_back(trim(field));
        }
        return fields;
    }

    static vector<string> readPrerequisites() {
        cout << "Enter prerequisites separated by commas, or press Enter for none: ";
        string line;
        getline(cin, line);
        vector<string> prerequisites;
        for (string value : splitCsvLine(line)) {
            value = normalizeCourseNumber(value);
            if (!value.empty()) {
                prerequisites.push_back(value);
            }
        }
        return prerequisites;
    }

    bool prerequisitesExist(const vector<string>& prerequisites, const string& ignoredCourse = "") const {
        for (const string& prerequisite : prerequisites) {
            if (prerequisite == ignoredCourse) {
                continue;
            }
            if (courses.find(prerequisite) == nullptr) {
                cout << "Error: Prerequisite " << prerequisite << " does not exist.\n";
                return false;
            }
        }
        return true;
    }

    static void printCourse(const Course& course) {
        cout << course.courseNumber << ", " << course.courseTitle << '\n';
        if (course.prerequisites.empty()) {
            cout << "Prerequisites: None\n";
        } else {
            cout << "Prerequisites: ";
            for (size_t i = 0; i < course.prerequisites.size(); ++i) {
                if (i > 0) cout << ", ";
                cout << course.prerequisites[i];
            }
            cout << '\n';
        }
    }

public:
    bool loadCourses(const string& filename) {
        ifstream file(filename);
        if (!file) {
            cerr << "Error: Unable to open " << filename << ".\n";
            return false;
        }

        CourseHashTable loadedCourses;
        vector<Course> pending;
        string line;
        size_t lineNumber = 0;

        while (getline(file, line)) {
            ++lineNumber;
            line = trim(line);
            if (line.empty()) continue;

            vector<string> fields = splitCsvLine(line);
            if (fields.size() < 2) {
                cerr << "Error on line " << lineNumber << ": A course number and title are required.\n";
                return false;
            }

            Course course;
            course.courseNumber = normalizeCourseNumber(fields[0]);
            course.courseTitle = trim(fields[1]);

            if (!isValidCourseNumber(course.courseNumber) || course.courseTitle.empty()) {
                cerr << "Error on line " << lineNumber << ": Invalid course number or missing title.\n";
                return false;
            }

            for (size_t i = 2; i < fields.size(); ++i) {
                const string prerequisite = normalizeCourseNumber(fields[i]);
                if (!prerequisite.empty()) {
                    if (!isValidCourseNumber(prerequisite)) {
                        cerr << "Error on line " << lineNumber << ": Invalid prerequisite "
                             << prerequisite << ".\n";
                        return false;
                    }
                    course.prerequisites.push_back(prerequisite);
                }
            }

            if (!loadedCourses.insert(course)) {
                cerr << "Error on line " << lineNumber << ": Duplicate course "
                     << course.courseNumber << ".\n";
                return false;
            }
            pending.push_back(course);
        }

        unordered_set<string> validNumbers;
        for (const Course& course : pending) validNumbers.insert(course.courseNumber);
        for (const Course& course : pending) {
            for (const string& prerequisite : course.prerequisites) {
                if (validNumbers.count(prerequisite) == 0) {
                    cerr << "Error: " << course.courseNumber << " references missing prerequisite "
                         << prerequisite << ".\n";
                    return false;
                }
            }
        }

        courses = move(loadedCourses);
        dataLoaded = true;
        cout << courses.size() << " courses successfully loaded from " << filename << ".\n";
        return true;
    }

    void displayCourses() const {
        if (!dataLoaded || courses.size() == 0) {
            cout << "No courses available. Load the data first.\n";
            return;
        }
        cout << "\nCourse List (Alphanumeric Order)\n";
        cout << "--------------------------------\n";
        for (const Course& course : courses.getAllSorted()) {
            cout << course.courseNumber << ", " << course.courseTitle << '\n';
        }
    }

    void searchCourse() const {
        cout << "Enter the course number: ";
        string courseNumber;
        getline(cin, courseNumber);
        courseNumber = normalizeCourseNumber(courseNumber);
        const Course* course = courses.find(courseNumber);
        if (course == nullptr) {
            cout << "Course " << courseNumber << " was not found.\n";
            return;
        }
        printCourse(*course);
    }

    void addCourse() {
        Course course;
        cout << "Enter the new course number: ";
        getline(cin, course.courseNumber);
        course.courseNumber = normalizeCourseNumber(course.courseNumber);
        if (!isValidCourseNumber(course.courseNumber)) {
            cout << "Error: Use letters and numbers only, such as CSCI300.\n";
            return;
        }
        if (courses.find(course.courseNumber) != nullptr) {
            cout << "Error: That course already exists.\n";
            return;
        }
        cout << "Enter the course title: ";
        getline(cin, course.courseTitle);
        course.courseTitle = trim(course.courseTitle);
        if (course.courseTitle.empty()) {
            cout << "Error: The course title cannot be empty.\n";
            return;
        }
        course.prerequisites = readPrerequisites();
        if (!prerequisitesExist(course.prerequisites)) return;
        courses.insert(course);
        dataLoaded = true;
        cout << course.courseNumber << " was added.\n";
    }

    void editCourse() {
        cout << "Enter the course number to edit: ";
        string originalNumber;
        getline(cin, originalNumber);
        originalNumber = normalizeCourseNumber(originalNumber);
        const Course* existing = courses.find(originalNumber);
        if (existing == nullptr) {
            cout << "Course " << originalNumber << " was not found.\n";
            return;
        }

        Course replacement = *existing;
        cout << "Enter the replacement course number, or press Enter to keep "
             << replacement.courseNumber << ": ";
        string value;
        getline(cin, value);
        if (!trim(value).empty()) replacement.courseNumber = normalizeCourseNumber(value);
        if (!isValidCourseNumber(replacement.courseNumber)) {
            cout << "Error: Invalid course number.\n";
            return;
        }

        cout << "Enter the replacement title, or press Enter to keep the current title: ";
        getline(cin, value);
        if (!trim(value).empty()) replacement.courseTitle = trim(value);

        cout << "Replace prerequisites? (Y/N): ";
        getline(cin, value);
        if (!value.empty() && toupper(static_cast<unsigned char>(value[0])) == 'Y') {
            replacement.prerequisites = readPrerequisites();
        }

        if (!prerequisitesExist(replacement.prerequisites, originalNumber)) return;
        if (!courses.update(originalNumber, replacement)) {
            cout << "Error: The replacement course number already exists.\n";
            return;
        }
        cout << replacement.courseNumber << " was updated.\n";
    }

    void deleteCourse() {
        cout << "Enter the course number to delete: ";
        string courseNumber;
        getline(cin, courseNumber);
        courseNumber = normalizeCourseNumber(courseNumber);
        if (courses.find(courseNumber) == nullptr) {
            cout << "Course " << courseNumber << " was not found.\n";
            return;
        }
        for (const Course& course : courses.getAllSorted()) {
            if (find(course.prerequisites.begin(), course.prerequisites.end(), courseNumber)
                != course.prerequisites.end()) {
                cout << "Error: " << courseNumber << " cannot be deleted because it is a prerequisite for "
                     << course.courseNumber << ".\n";
                return;
            }
        }
        courses.remove(courseNumber);
        cout << courseNumber << " was deleted.\n";
    }

    void displayPerformanceResults() const {
        if (courses.size() == 0) {
            cout << "No courses are available for benchmarking.\n";
            return;
        }
        constexpr int repetitions = 10000;
        const vector<Course> allCourses = courses.getAllSorted();
        const string existingKey = allCourses[allCourses.size() / 2].courseNumber;
        volatile size_t matchCount = 0;

        const auto hashStart = chrono::high_resolution_clock::now();
        for (int i = 0; i < repetitions; ++i) {
            if (courses.find(existingKey) != nullptr) ++matchCount;
        }
        const auto hashEnd = chrono::high_resolution_clock::now();

        const auto linearStart = chrono::high_resolution_clock::now();
        for (int i = 0; i < repetitions; ++i) {
            for (const Course& course : allCourses) {
                if (course.courseNumber == existingKey) {
                    ++matchCount;
                    break;
                }
            }
        }
        const auto linearEnd = chrono::high_resolution_clock::now();

        const auto sortStart = chrono::high_resolution_clock::now();
        const vector<Course> sortedCopy = courses.getAllSorted();
        const auto sortEnd = chrono::high_resolution_clock::now();

        const auto hashNs = chrono::duration_cast<chrono::nanoseconds>(hashEnd - hashStart).count();
        const auto linearNs = chrono::duration_cast<chrono::nanoseconds>(linearEnd - linearStart).count();
        const auto sortNs = chrono::duration_cast<chrono::nanoseconds>(sortEnd - sortStart).count();

        cout << "\nPerformance Results\n";
        cout << "-------------------\n";
        cout << "Courses stored: " << courses.size() << '\n';
        cout << "Hash buckets: " << courses.bucketCount() << '\n';
        cout << "Buckets in use: " << courses.usedBucketCount() << '\n';
        cout << "Hash collisions: " << courses.collisionCount() << '\n';
        cout << repetitions << " hash-table searches: " << hashNs << " ns\n";
        cout << repetitions << " linear vector searches: " << linearNs << " ns\n";
        cout << "Copy and alphanumeric sort: " << sortNs << " ns\n";
        cout << "Benchmark target: " << existingKey << '\n';
        (void)sortedCopy;
        (void)matchCount;
    }
};

void displayMenu() {
    cout << "\nABCU Advising Assistance Program\n";
    cout << "1. Load Course Data\n";
    cout << "2. Display All Courses\n";
    cout << "3. Search for a Course\n";
    cout << "4. Add a Course\n";
    cout << "5. Edit a Course\n";
    cout << "6. Delete a Course\n";
    cout << "7. Display Performance Results\n";
    cout << "9. Exit\n";
    cout << "Enter a selection: ";
}

int main() {
    CourseManager manager;
    const string defaultFilename = "ABCU_Advising_Program_Input.csv";
    string input;

    while (true) {
        displayMenu();
        getline(cin, input);

        int selection = 0;
        try {
            size_t processed = 0;
            selection = stoi(trim(input), &processed);
            if (processed != trim(input).size()) throw invalid_argument("extra characters");
        } catch (...) {
            cout << "Invalid selection. Enter 1-7 or 9.\n";
            continue;
        }

        switch (selection) {
            case 1: {
                cout << "Enter the CSV filename, or press Enter to use " << defaultFilename << ": ";
                string filename;
                getline(cin, filename);
                filename = trim(filename);
                manager.loadCourses(filename.empty() ? defaultFilename : filename);
                break;
            }
            case 2: manager.displayCourses(); break;
            case 3: manager.searchCourse(); break;
            case 4: manager.addCourse(); break;
            case 5: manager.editCourse(); break;
            case 6: manager.deleteCourse(); break;
            case 7: manager.displayPerformanceResults(); break;
            case 9:
                cout << "Thank you for using the course planner.\n";
                return 0;
            default:
                cout << "Invalid selection. Enter 1-7 or 9.\n";
        }
    }
}

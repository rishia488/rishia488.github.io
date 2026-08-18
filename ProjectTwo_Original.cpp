//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Arishia Jackson
// Course      : CS-300
// Description : Sorting Code            
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

// Structure to store course information
struct Course {
    string courseNumber;     // Course identifier
    string courseTitle;      // Course name
    vector<string> prerequisites; // List of prerequisite courses
};

// Convert a string to uppercase for consistency
string toUpperCase(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// Class to manage course data
class CourseManager {
private:
    map<string, Course> courses; // Stores courses in sorted order automatically

public:
    // Load course data from a file
    bool loadCourses(const string& filename) {
        ifstream file(filename);
        if (!file) {
            cerr << "Error: Unable to open file.\n";
            return false;
        }

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string courseNumber, courseTitle, prereq;

            getline(ss, courseNumber, ',');
            getline(ss, courseTitle, ',');

            Course course;
            course.courseNumber = toUpperCase(courseNumber);
            course.courseTitle = courseTitle;

            while (getline(ss, prereq, ',')) {
                course.prerequisites.push_back(toUpperCase(prereq));
            }

            courses[course.courseNumber] = course;
        }

        cout << "Courses successfully loaded.\n";
        return true;
    }

    // Display all courses in sorted order
    void displayCourses() const {
        if (courses.empty()) {
            cout << "No courses available. Please load the data first.\n";
            return;
        }

        cout << "Course List (Sorted):\n";
        for (const auto& pair : courses) {
            cout << pair.second.courseNumber << ", " << pair.second.courseTitle << endl;
        }
        cout << endl;
    }
};

// Main function
int main() {
    CourseManager manager;
    string filename = "ABCU_Advising_Program_Input.csv";

    if (manager.loadCourses(filename)) {
        cout << "Courses successfully loaded.\n";
        manager.displayCourses();
    }

    return 0;
}

#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <vector>
#include <limits>
#include <cstdlib>
#include <cctype>
using namespace std;

struct Patient {
    int id;
    string name;
    int age;
    string phone;
    Patient* next;
};

// Information about one appointment.
// priority: 1 = Emergency, 2 = Urgent, 3 = Normal
// time: HH:MM in 24-hour format (example: 09:30, 14:45)
struct Appointment {
    int appointmentId;
    int patientId;
    string date;
    string time;
    string status;
    int priority;
};

// Turns DD-MM-YYYY into a number YYYYMMDD so dates can be compared.
// Example: 05-12-2026 becomes 20261205 (a smaller number means an earlier date).
int dateKey(const string& d) {
    int day = atoi(d.substr(0, 2).c_str());
    int month = atoi(d.substr(3, 2).c_str());
    int year = atoi(d.substr(6, 4).c_str());

    return year * 10000 + month * 100 + day;
}

// Turns HH:MM (24-hour) into minutes since midnight so times can be compared.
// Example: 14:30 becomes 870. A badly formatted time goes last (9999).
int timeKey(const string& t) {
    if (t.length() != 5 || t[2] != ':')
        return 9999;

    int hour = atoi(t.substr(0, 2).c_str());
    int minute = atoi(t.substr(3, 2).c_str());

    return hour * 60 + minute;
}

// Rule for the priority queue (who is served first):
// 1. The earlier date goes first.
// 2. On the same date, the more urgent one goes first (1 = Emergency).
// 3. If date and priority are the same, the earlier time goes first.
// 4. If everything is the same, the one booked earlier (smaller ID) goes first.
struct ComparePriority {
    bool operator()(const Appointment& a, const Appointment& b) {
        int dateA = dateKey(a.date);
        int dateB = dateKey(b.date);

        if (dateA != dateB)
            return dateA > dateB;

        if (a.priority != b.priority)
            return a.priority > b.priority;

        int timeA = timeKey(a.time);
        int timeB = timeKey(b.time);

        if (timeA != timeB)
            return timeA > timeB;

        return a.appointmentId > b.appointmentId;
    }
};

Patient* head = NULL;
queue<Appointment> appointments;   // all appointments in arrival order (record)
priority_queue<Appointment, vector<Appointment>, ComparePriority> waiting; // served by date, urgency, time
int nextAppointmentId = 1;

string priorityName(int p) {
    if (p == 1) return "Emergency";
    if (p == 2) return "Urgent";
    return "Normal";
}

// ---------- INPUT HELPERS ----------

// Reads a whole line and keeps asking until it is a valid whole number.
int readInt(const string& prompt) {
    string line;

    while (true) {
        cout << prompt;
        if (!getline(cin, line))
            exit(0);                       // input ended (Ctrl+Z / Ctrl+D)

        bool ok = !line.empty() && line.length() <= 9;
        size_t start = (ok && line[0] == '-') ? 1 : 0;

        if (start == line.length())
            ok = false;

        for (size_t i = start; ok && i < line.length(); i++) {
            if (!isdigit((unsigned char)line[i]))
                ok = false;
        }

        if (ok)
            return atoi(line.c_str());

        cout << "Invalid input. Please enter a number.\n";
    }
}

// An ID must be a positive number.
int readId(const string& prompt) {
    int id;

    do {
        id = readInt(prompt);
        if (id <= 0)
            cout << "ID must be a positive number.\n";
    } while (id <= 0);

    return id;
}

// A name may contain only letters and spaces (no digits or symbols).
bool isValidName(const string& name) {
    bool hasLetter = false;

    for (size_t i = 0; i < name.length(); i++) {
        unsigned char c = name[i];

        if (isalpha(c))
            hasLetter = true;
        else if (c != ' ')
            return false;
    }

    return hasLetter;
}

string readName(const string& prompt) {
    string name;

    while (true) {
        cout << prompt;
        if (!getline(cin, name))
            exit(0);

        if (isValidName(name))
            return name;

        cout << "Invalid name. Use letters and spaces only (no numbers).\n";
    }
}

int readAge(const string& prompt) {
    int age;

    while (true) {
        age = readInt(prompt);

        if (age <= 0)
            cout << "Age cannot be zero or negative.\n";
        else if (age > 120)
            cout << "Age is too large. Enter a value up to 120.\n";
        else
            return age;
    }
}

// A phone number must be digits only, with at most 11 digits.
bool isValidPhone(const string& phone) {
    if (phone.empty() || phone.length() > 11)
        return false;

    for (size_t i = 0; i < phone.length(); i++) {
        if (!isdigit((unsigned char)phone[i]))
            return false;
    }

    return true;
}

string readPhone(const string& prompt) {
    string phone;

    while (true) {
        cout << prompt;
        if (!getline(cin, phone))
            exit(0);

        if (isValidPhone(phone))
            return phone;

        cout << "Invalid phone number. Use digits only, maximum 11 digits.\n";
    }
}

string toLower(string text) {
    for (size_t i = 0; i < text.length(); i++)
        text[i] = tolower((unsigned char)text[i]);

    return text;
}

// ---------- PATIENT FUNCTIONS ----------

Patient* findPatient(int id) {
    Patient* current = head;

    while (current != NULL) {
        if (current->id == id)
            return current;

        current = current->next;
    }

    return NULL;
}

void addPatient() {
    int id = readId("Enter patient ID: ");

    if (findPatient(id) != NULL) {
        cout << "This patient ID already exists.\n";
        return;
    }

    Patient* p = new Patient;
    p->id = id;
    p->name = readName("Enter patient name: ");
    p->age = readAge("Enter age: ");
    p->phone = readPhone("Enter phone number (max 11 digits): ");

    p->next = head;
    head = p;

    cout << "Patient added successfully.\n";
}

void displayPatients() {
    if (head == NULL) {
        cout << "No patient records found.\n";
        return;
    }

    Patient* current = head;

    cout << "\n--- Patient Records ---\n";
    while (current != NULL) {
        cout << "ID: " << current->id
             << " | Name: " << current->name
             << " | Age: " << current->age
             << " | Phone: " << current->phone << "\n";

        current = current->next;
    }
}

void searchPatient() {
    cout << "1. Search by ID\n";
    cout << "2. Search by Name\n";
    int choice = readInt("Choose: ");

    if (choice == 1) {
        int id = readId("Enter patient ID to search: ");
        Patient* p = findPatient(id);

        if (p == NULL) {
            cout << "Patient not found.\n";
        } else {
            cout << "Patient found: " << p->name
                 << " | Age: " << p->age
                 << " | Phone: " << p->phone << "\n";
        }
    } else if (choice == 2) {
        string name;
        cout << "Enter patient name to search: ";
        getline(cin, name);

        Patient* current = head;
        bool found = false;

        while (current != NULL) {
            if (toLower(current->name) == toLower(name)) {
                cout << "Patient found: ID " << current->id
                     << " | " << current->name
                     << " | Age: " << current->age
                     << " | Phone: " << current->phone << "\n";
                found = true;
            }
            current = current->next;
        }

        if (!found)
            cout << "Patient not found.\n";
    } else {
        cout << "Invalid choice.\n";
    }
}

void updatePatient() {
    int id = readId("Enter patient ID to update: ");

    Patient* p = findPatient(id);

    if (p == NULL) {
        cout << "Patient not found.\n";
        return;
    }

    p->name = readName("Enter new name: ");
    p->age = readAge("Enter new age: ");
    p->phone = readPhone("Enter new phone number (max 11 digits): ");

    cout << "Patient record updated.\n";
}

void deletePatient() {
    int id = readId("Enter patient ID to delete: ");

    Patient* current = head;
    Patient* previous = NULL;

    while (current != NULL && current->id != id) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        cout << "Patient not found.\n";
        return;
    }

    // Do not delete a patient who has a scheduled appointment.
    queue<Appointment> temp = appointments;
    while (!temp.empty()) {
        Appointment a = temp.front();
        temp.pop();

        if (a.patientId == id && a.status == "Scheduled") {
            cout << "Cannot delete: patient has a scheduled appointment.\n";
            return;
        }
    }

    if (previous == NULL)
        head = current->next;
    else
        previous->next = current->next;

    delete current;
    cout << "Patient deleted successfully.\n";
}

void sortPatients() {
    if (head == NULL || head->next == NULL) {
        cout << "Not enough records to sort.\n";
        return;
    }

    // Bubble sort: arrange patient records by ID.
    Patient* i;
    Patient* j;

    for (i = head; i->next != NULL; i = i->next) {
        for (j = head; j->next != NULL; j = j->next) {
            if (j->id > j->next->id) {
                int tempId = j->id;
                j->id = j->next->id;
                j->next->id = tempId;

                string tempName = j->name;
                j->name = j->next->name;
                j->next->name = tempName;

                int tempAge = j->age;
                j->age = j->next->age;
                j->next->age = tempAge;

                string tempPhone = j->phone;
                j->phone = j->next->phone;
                j->next->phone = tempPhone;
            }
        }
    }

    cout << "Patients sorted by ID.\n";
    displayPatients();
}

// ---------- APPOINTMENT QUEUE FUNCTIONS ----------

// Checks that a date is in DD-MM-YYYY format and is a real calendar date.
bool isValidDate(const string& d) {
    if (d.length() != 10 || d[2] != '-' || d[5] != '-')
        return false;

    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5)
            continue;
        if (d[i] < '0' || d[i] > '9')
            return false;
    }

    int day = atoi(d.substr(0, 2).c_str());
    int month = atoi(d.substr(3, 2).c_str());
    int year = atoi(d.substr(6, 4).c_str());

    if (year < 2000 || year > 2100 || month < 1 || month > 12)
        return false;

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // February has 29 days in a leap year.
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
        daysInMonth[1] = 29;

    return day >= 1 && day <= daysInMonth[month - 1];
}

// Checks that a time is in HH:MM 24-hour format and is a real time.
bool isValidTime(const string& t) {
    if (t.length() != 5 || t[2] != ':')
        return false;

    for (int i = 0; i < 5; i++) {
        if (i == 2)
            continue;
        if (t[i] < '0' || t[i] > '9')
            return false;
    }

    int hour = atoi(t.substr(0, 2).c_str());
    int minute = atoi(t.substr(3, 2).c_str());

    return hour <= 23 && minute <= 59;
}

void scheduleAppointment() {
    Appointment a;

    a.patientId = readId("Enter patient ID: ");

    if (findPatient(a.patientId) == NULL) {
        cout << "Patient does not exist. Register the patient first.\n";
        return;
    }

    a.priority = readInt("Enter priority (1 = Emergency, 2 = Urgent, 3 = Normal): ");

    if (a.priority < 1 || a.priority > 3) {
        cout << "Invalid priority. Appointment was not scheduled.\n";
        return;
    }

    // Keep asking until the date is valid.
    do {
        cout << "Enter appointment date (DD-MM-YYYY): ";
        getline(cin, a.date);

        if (!isValidDate(a.date))
            cout << "Invalid date. Use DD-MM-YYYY, for example 25-12-2026.\n";
    } while (!isValidDate(a.date));

    // Keep asking until the time is valid.
    do {
        cout << "Enter appointment time (HH:MM, 24-hour): ";
        getline(cin, a.time);

        if (!isValidTime(a.time))
            cout << "Invalid time. Use HH:MM, for example 09:30 or 14:45.\n";
    } while (!isValidTime(a.time));

    // No duplicate: same patient, same date and time, still scheduled.
    queue<Appointment> temp = appointments;
    while (!temp.empty()) {
        Appointment x = temp.front();
        temp.pop();

        if (x.patientId == a.patientId && x.date == a.date &&
            x.time == a.time && x.status == "Scheduled") {
            cout << "This patient already has an appointment at that date and time.\n";
            return;
        }
    }

    a.appointmentId = nextAppointmentId;
    nextAppointmentId++;

    a.status = "Scheduled";
    appointments.push(a);   // keep the record
    waiting.push(a);        // add to the priority queue

    cout << "Appointment scheduled. Appointment ID: "
         << a.appointmentId << " (" << priorityName(a.priority) << ")\n";
}

void displayAppointments() {
    if (appointments.empty()) {
        cout << "Appointment queue is empty.\n";
        return;
    }

    queue<Appointment> temp = appointments;

    cout << "\n--- Appointment Queue (arrival order) ---\n";
    while (!temp.empty()) {
        Appointment a = temp.front();
        temp.pop();

        cout << "Appointment ID: " << a.appointmentId
             << " | Patient ID: " << a.patientId
             << " | Date: " << a.date
             << " | Time: " << a.time
             << " | Priority: " << priorityName(a.priority)
             << " | Status: " << a.status << "\n";
    }
}

// Shows the scheduled appointments in the order they will be served.
void displayWaitingList() {
    // Copy the priority queue so the original is not changed.
    priority_queue<Appointment, vector<Appointment>, ComparePriority> temp = waiting;
    bool any = false;

    int position = 1;

    cout << "\n--- Priority Waiting List (date, then urgency, then time) ---\n";

    while (!temp.empty()) {
        Appointment a = temp.top();
        temp.pop();

        // Skip appointments that were cancelled or completed.
        bool stillScheduled = false;
        queue<Appointment> r = appointments;
        while (!r.empty()) {
            Appointment x = r.front();
            r.pop();
            if (x.appointmentId == a.appointmentId && x.status == "Scheduled")
                stillScheduled = true;
        }

        if (stillScheduled) {
            any = true;
            cout << position << ". Appointment ID: " << a.appointmentId
                 << " | Patient ID: " << a.patientId
                 << " | Date: " << a.date
                 << " | Time: " << a.time
                 << " | Priority: " << priorityName(a.priority) << "\n";
            position++;
        }
    }

    if (!any)
        cout << "No scheduled appointments are waiting.\n";
}

void updateAppointmentStatus() {
    if (appointments.empty()) {
        cout << "Appointment queue is empty.\n";
        return;
    }

    int id = readId("Enter appointment ID: ");

    cout << "1. Mark as Completed\n";
    cout << "2. Cancel Appointment\n";
    int choice = readInt("Choose: ");

    if (choice != 1 && choice != 2) {
        cout << "Invalid choice.\n";
        return;
    }

    queue<Appointment> temp;
    bool found = false;

    while (!appointments.empty()) {
        Appointment a = appointments.front();
        appointments.pop();

        if (a.appointmentId == id) {
            found = true;

            if (a.status != "Scheduled") {
                cout << "This appointment is already " << a.status << ".\n";
            } else if (choice == 1) {
                a.status = "Completed";
                cout << "Appointment marked as completed.\n";
            } else {
                a.status = "Cancelled";
                cout << "Appointment cancelled.\n";
            }
        }

        temp.push(a);
    }

    appointments = temp;

    if (!found)
        cout << "Appointment ID not found.\n";
}

// Changes an appointment to Completed only if it is still Scheduled.
// Returns true if the change was made.
bool completeIfScheduled(int id) {
    queue<Appointment> temp;
    bool changed = false;

    while (!appointments.empty()) {
        Appointment a = appointments.front();
        appointments.pop();

        if (a.appointmentId == id && a.status == "Scheduled") {
            a.status = "Completed";
            changed = true;
        }

        temp.push(a);
    }

    appointments = temp;
    return changed;
}

// Serves the earliest date first; on the same date, the most urgent first;
// then the earliest time.
void processNextAppointment() {
    while (!waiting.empty()) {
        Appointment top = waiting.top();
        waiting.pop();

        // If it was already cancelled/completed, skip it and try the next one.
        if (completeIfScheduled(top.appointmentId)) {
            cout << "Next appointment processed: Appointment ID "
                 << top.appointmentId << " | Patient ID " << top.patientId
                 << " | Date: " << top.date
                 << " | Time: " << top.time
                 << " | Priority: " << priorityName(top.priority) << "\n";
            return;
        }
    }

    cout << "There are no scheduled appointments to process.\n";
}

// ---------- FILE HANDLING ----------

void saveData() {
    ofstream patientFile("patients.txt");
    Patient* current = head;

    while (current != NULL) {
        patientFile << current->id << "|"
                    << current->name << "|"
                    << current->age << "|"
                    << current->phone << "\n";

        current = current->next;
    }
    patientFile.close();

    ofstream appointmentFile("appointments.txt");
    queue<Appointment> temp = appointments;

    while (!temp.empty()) {
        Appointment a = temp.front();
        temp.pop();

        appointmentFile << a.appointmentId << "|"
                        << a.patientId << "|"
                        << a.date << "|"
                        << a.time << "|"
                        << a.status << "|"
                        << a.priority << "\n";
    }
    appointmentFile.close();

    cout << "Data saved to files.\n";
}

void loadData() {
    ifstream patientFile("patients.txt");

    if (patientFile.is_open()) {
        string line;

        while (getline(patientFile, line)) {
            Patient* p = new Patient;
            size_t pos1 = line.find('|');
            size_t pos2 = line.find('|', pos1 + 1);
            size_t pos3 = line.find('|', pos2 + 1);

            if (pos1 == string::npos || pos2 == string::npos ||
                pos3 == string::npos) {
                delete p;
                continue;
            }

            p->id = atoi(line.substr(0, pos1).c_str());
            p->name = line.substr(pos1 + 1, pos2 - pos1 - 1);
            p->age = atoi(line.substr(pos2 + 1, pos3 - pos2 - 1).c_str());
            p->phone = line.substr(pos3 + 1);

            p->next = head;
            head = p;
        }

        patientFile.close();
    }

    ifstream appointmentFile("appointments.txt");

    if (appointmentFile.is_open()) {
        string line;

        while (getline(appointmentFile, line)) {
            size_t p1 = line.find('|');
            size_t p2 = line.find('|', p1 + 1);
            size_t p3 = line.find('|', p2 + 1);
            size_t p4 = line.find('|', p3 + 1);
            size_t p5 = line.find('|', p4 + 1);

            if (p1 == string::npos || p2 == string::npos ||
                p3 == string::npos || p4 == string::npos)
                continue;

            Appointment a;
            a.appointmentId = atoi(line.substr(0, p1).c_str());
            a.patientId = atoi(line.substr(p1 + 1, p2 - p1 - 1).c_str());
            a.date = line.substr(p2 + 1, p3 - p2 - 1);
            a.time = line.substr(p3 + 1, p4 - p3 - 1);

            if (p5 == string::npos) {
                // Old file format without priority: treat as Normal.
                a.status = line.substr(p4 + 1);
                a.priority = 3;
            } else {
                a.status = line.substr(p4 + 1, p5 - p4 - 1);
                a.priority = atoi(line.substr(p5 + 1).c_str());
            }

            appointments.push(a);

            // Rebuild the priority queue from appointments still waiting.
            if (a.status == "Scheduled")
                waiting.push(a);

            if (a.appointmentId >= nextAppointmentId)
                nextAppointmentId = a.appointmentId + 1;
        }

        appointmentFile.close();
    }
}

// ---------- MAIN MENU ----------

int main() {
    loadData();

    int choice;

    do {
        cout << "\n";
        cout << "==================================================\n";
        cout << "          HOSPITAL MANAGEMENT SYSTEM              \n";
        cout << "==================================================\n";
        cout << "                 PATIENT MANAGEMENT               \n";
        cout << "--------------------------------------------------\n";
        cout << "  1.  Register New Patient\n";
        cout << "  2.  View All Patients\n";
        cout << "  3.  Search Patient\n";
        cout << "  4.  Update Patient Record\n";
        cout << "  5.  Delete Patient Record\n";
        cout << "  6.  Sort Patients by ID\n";
        cout << "\n";
        cout << "               APPOINTMENT MANAGEMENT             \n";
        cout << "--------------------------------------------------\n";
        cout << "  7.  Schedule Appointment\n";
        cout << "  8.  View Appointment Queue\n";
        cout << "  9.  Complete / Cancel Appointment\n";
        cout << "  10. Process Next Appointment (date + priority + time)\n";
        cout << "  11. View Priority Waiting List\n";
        cout << "\n";
        cout << "               DATA AND EXIT                      \n";
        cout << "--------------------------------------------------\n";
        cout << "  12. Save Data\n";
        cout << "  0.  Save and Exit\n";
        cout << "==================================================\n";
        choice = readInt("  Enter your choice: ");

        switch (choice) {
            case 1: addPatient(); break;
            case 2: displayPatients(); break;
            case 3: searchPatient(); break;
            case 4: updatePatient(); break;
            case 5: deletePatient(); break;
            case 6: sortPatients(); break;
            case 7: scheduleAppointment(); break;
            case 8: displayAppointments(); break;
            case 9: updateAppointmentStatus(); break;
            case 10: processNextAppointment(); break;
            case 11: displayWaitingList(); break;
            case 12: saveData(); break;
            case 0:
                saveData();
                cout << "Program closed. Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }

    } while (choice != 0);

    // Free linked-list memory before closing.
    while (head != NULL) {
        Patient* temp = head;
        head = head->next;
        delete temp;
    }

    return 0;
}

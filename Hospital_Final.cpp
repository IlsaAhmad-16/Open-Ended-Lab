#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <vector>
#include <limits>
#include <cstdlib>
using namespace std;

// Each patient is one node in the linked list.
struct Patient {
    int id;
    string name;
    int age;
    string phone;
    Patient* next;
};

// Information about one appointment.
// priority: 1 = Emergency, 2 = Urgent, 3 = Normal
struct Appointment {
    int appointmentId;
    int patientId;
    string date;
    string time;
    string status;
    int priority;
};

// Rule for the priority queue: smaller priority number comes out first.
// If two appointments have the same priority, the earlier one (smaller ID) goes first.
struct ComparePriority {
    bool operator()(const Appointment& a, const Appointment& b) {
        if (a.priority != b.priority)
            return a.priority > b.priority;
        return a.appointmentId > b.appointmentId;
    }
};

Patient* head = NULL;
queue<Appointment> appointments;   // all appointments in arrival order (record)
priority_queue<Appointment, vector<Appointment>, ComparePriority> waiting; // served by urgency
int nextAppointmentId = 1;

string priorityName(int p) {
    if (p == 1) return "Emergency";
    if (p == 2) return "Urgent";
    return "Normal";
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
    Patient* p = new Patient;

    cout << "Enter patient ID: ";
    cin >> p->id;

    if (findPatient(p->id) != NULL) {
        cout << "This patient ID already exists.\n";
        delete p;
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Enter patient name: ";
    getline(cin, p->name);

    cout << "Enter age: ";
    cin >> p->age;

    if (p->age <= 0) {
        cout << "Invalid age. Patient was not added.\n";
        delete p;
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Enter phone number: ";
    getline(cin, p->phone);

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
    int id;
    cout << "Enter patient ID to search: ";
    cin >> id;

    Patient* p = findPatient(id);

    if (p == NULL) {
        cout << "Patient not found.\n";
    } else {
        cout << "Patient found: " << p->name
             << " | Age: " << p->age
             << " | Phone: " << p->phone << "\n";
    }
}

void updatePatient() {
    int id;
    cout << "Enter patient ID to update: ";
    cin >> id;

    Patient* p = findPatient(id);

    if (p == NULL) {
        cout << "Patient not found.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Enter new name: ";
    getline(cin, p->name);

    cout << "Enter new age: ";
    cin >> p->age;

    if (p->age <= 0) {
        cout << "Invalid age. Record was not updated.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Enter new phone number: ";
    getline(cin, p->phone);

    cout << "Patient record updated.\n";
}

void deletePatient() {
    int id;
    cout << "Enter patient ID to delete: ";
    cin >> id;

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

void scheduleAppointment() {
    Appointment a;

    cout << "Enter patient ID: ";
    cin >> a.patientId;

    if (findPatient(a.patientId) == NULL) {
        cout << "Patient does not exist. Register the patient first.\n";
        return;
    }

    cout << "Enter priority (1 = Emergency, 2 = Urgent, 3 = Normal): ";
    cin >> a.priority;

    if (a.priority < 1 || a.priority > 3) {
        cout << "Invalid priority. Appointment was not scheduled.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Enter appointment date (DD-MM-YYYY): ";
    getline(cin, a.date);

    cout << "Enter appointment time: ";
    getline(cin, a.time);

    if (a.date.empty() || a.time.empty()) {
        cout << "Date and time cannot be empty.\n";
        return;
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

    cout << "\n--- Priority Waiting List (next to be served first) ---\n";

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
            cout << "Appointment ID: " << a.appointmentId
                 << " | Patient ID: " << a.patientId
                 << " | Priority: " << priorityName(a.priority)
                 << " | Date: " << a.date
                 << " | Time: " << a.time << "\n";
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

    int id;
    int choice;

    cout << "Enter appointment ID: ";
    cin >> id;

    cout << "1. Mark as Completed\n";
    cout << "2. Cancel Appointment\n";
    cout << "Choose: ";
    cin >> choice;

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

// Serves the most urgent appointment first (Emergency, then Urgent, then Normal).
void processNextAppointment() {
    while (!waiting.empty()) {
        Appointment top = waiting.top();
        waiting.pop();

        // If it was already cancelled/completed, skip it and try the next one.
        if (completeIfScheduled(top.appointmentId)) {
            cout << "Next appointment processed: Appointment ID "
                 << top.appointmentId << " | Patient ID " << top.patientId
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
        cout << "  10. Process Next Appointment (by priority)\n";
        cout << "  11. View Priority Waiting List\n";
        cout << "\n";
        cout << "               DATA AND EXIT                      \n";
        cout << "--------------------------------------------------\n";
        cout << "  12. Save Data\n";
        cout << "  0.  Save and Exit\n";
        cout << "==================================================\n";
        cout << "  Enter your choice: ";
        cin >> choice;

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

#include <iostream>
#include <string>
#include <vector>

#include "Doctor.h"
#include "Appointment.h"
#include "FileManager.h"
#include "PrimaryIndex.h"
#include "SecondaryIndex.h"
#include "CRUD.h"
#include "Integration.h"
#include "QueryEngine.h"

using namespace std;

static string readInput(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        return "";
    }
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = line.find_last_not_of(" \t\r\n");
    return line.substr(first, last - first + 1);
}

static void displayMenu() {
    cout << "\n==================================================\n";
    cout << "      Healthcare Management System (IS321)        \n";
    cout << "==================================================\n";
    cout << " 6.  Add New Doctor\n";
    cout << " 7.  Add New Appointment\n";
    cout << " 8.  Update Doctor Name (Doctor ID)\n";
    cout << " 9.  Update Doctor ID [Key Update Test]\n";
    cout << " 10. Update Appointment Date (Appointment ID)\n";
    cout << " 11. Delete Appointment (Appointment ID)\n";
    cout << " 12. Delete Doctor (Doctor ID)\n";
    cout << " 13. Print Doctor Info (Doctor ID)\n";
    cout << " 14. Print Appointment Info (Appointment ID)\n";
    cout << " 15. Write Query\n";
    cout << " 16. Exit\n";
    cout << "==================================================\n";
}

int main() {
    const string doctorFile        = "doctors.bin";
    const string appointmentFile   = "appointments.bin";
    const string docNameListFile   = "doctor_name_list.bin";
    const string docSpecListFile   = "doctor_specialty_list.bin";
    const string appDocListFile    = "appointment_doctor_list.bin";

    initializeFile(doctorFile);
    initializeFile(appointmentFile);

    PrimaryIndex doctorPrimary;
    PrimaryIndex appointmentPrimary;
    SecondaryIndex doctorNameSecondary(docNameListFile);
    SecondaryIndex doctorSpecialtySecondary(docSpecListFile);
    SecondaryIndex appointmentDoctorSecondary(appDocListFile);

    FileStorageAdapter storage(doctorFile, appointmentFile);
    IndexManagerAdapter indexManager(doctorPrimary,
                                     appointmentPrimary,
                                     doctorNameSecondary,
                                     doctorSpecialtySecondary,
                                     appointmentDoctorSecondary);

    cout << "Initializing Healthcare Management System...\n";
    rebuildAllIndexes(doctorPrimary,
                      appointmentPrimary,
                      doctorNameSecondary,
                      doctorSpecialtySecondary,
                      appointmentDoctorSecondary,
                      doctorFile,
                      appointmentFile);
    cout << "Indexes loaded successfully. System ready.\n";

    CRUDOperations crud(storage, indexManager);
    QueryEngine queryEngine(storage,
                            doctorPrimary,
                            appointmentPrimary,
                            doctorNameSecondary,
                            doctorSpecialtySecondary,
                            appointmentDoctorSecondary);

    bool running = true;
    while (running) {
        displayMenu();
        string choiceStr = readInput("Enter your choice (6-16): ");
        if (choiceStr.empty()) {
            cout << "Please enter a valid choice.\n";
            continue;
        }

        int choice = 0;
        try {
            choice = stoi(choiceStr);
        } catch (...) {
            cout << "Invalid input. Please enter a number between 6 and 16.\n";
            continue;
        }

        string message;
        switch (choice) {
            case 6: {
                cout << "\n--- Add New Doctor ---\n";
                string id = readInput("Enter Doctor ID (max 15 chars): ");
                string name = readInput("Enter Doctor Name (max 30 chars): ");
                string specialty = readInput("Enter Doctor Specialty (max 20 chars): ");
                string address = readInput("Enter Address (max 30 chars): ");

                crud.addDoctor(id, name, specialty, address, message);
                cout << "Result: " << message << "\n";
                break;
            }
            case 7: {
                cout << "\n--- Add New Appointment ---\n";
                string id = readInput("Enter Appointment ID (max 15 chars): ");
                string date = readInput("Enter Appointment Date (max 30 chars): ");
                string doctorId = readInput("Enter Doctor ID (max 15 chars): ");

                crud.addAppointment(id, date, doctorId, message);
                cout << "Result: " << message << "\n";
                break;
            }
            case 8: {
                cout << "\n--- Update Doctor Name ---\n";
                string id = readInput("Enter Doctor ID: ");
                string newName = readInput("Enter New Doctor Name: ");

                crud.updateDoctorName(id, newName, message);
                cout << "Result: " << message << "\n";
                break;
            }
            case 9: {
                cout << "\n--- Update Doctor ID [Key Update Test] ---\n";
                string oldId = readInput("Enter Current Doctor ID: ");
                string newId = readInput("Enter New Doctor ID: ");

                crud.updateDoctorId(oldId, newId, message);
                cout << "Result: " << message << "\n";
                break;
            }
            case 10: {
                cout << "\n--- Update Appointment Date ---\n";
                string id = readInput("Enter Appointment ID: ");
                string newDate = readInput("Enter New Appointment Date: ");

                crud.updateAppointmentDate(id, newDate, message);
                cout << "Result: " << message << "\n";
                break;
            }
            case 11: {
                cout << "\n--- Delete Appointment ---\n";
                string id = readInput("Enter Appointment ID to delete: ");

                crud.deleteAppointment(id, message);
                cout << "Result: " << message << "\n";
                break;
            }
            case 12: {
                cout << "\n--- Delete Doctor ---\n";
                string id = readInput("Enter Doctor ID to delete: ");

                crud.deleteDoctor(id, message);
                cout << "Result: " << message << "\n";
                break;
            }
            case 13: {
                cout << "\n--- Print Doctor Info ---\n";
                string id = readInput("Enter Doctor ID: ");
                int offset = doctorPrimary.search(id);
                if (offset < 0) {
                    cout << "Result: Doctor ID '" << id << "' not found.\n";
                } else {
                    Doctor doc;
                    if (storage.readDoctorAt(offset, doc)) {
                        cout << "----------------------------------------\n";
                        QueryEngine::printDoctor(doc);
                        cout << "----------------------------------------\n";
                    } else {
                        cout << "Error: Could not read Doctor record from storage.\n";
                    }
                }
                break;
            }
            case 14: {
                cout << "\n--- Print Appointment Info ---\n";
                string id = readInput("Enter Appointment ID: ");
                int offset = appointmentPrimary.search(id);
                if (offset < 0) {
                    cout << "Result: Appointment ID '" << id << "' not found.\n";
                } else {
                    Appointment app;
                    if (storage.readAppointmentAt(offset, app)) {
                        cout << "----------------------------------------\n";
                        QueryEngine::printAppointment(app);
                        cout << "----------------------------------------\n";
                    } else {
                        cout << "Error: Could not read Appointment record from storage.\n";
                    }
                }
                break;
            }
            case 15: {
                cout << "\n--- Write Query ---\n";
                cout << "Enter SQL Query (e.g. SELECT all FROM Doctors WHERE Doctor ID='D1';):\n";
                string queryStr = readInput("Query: ");
                queryEngine.executeQuery(queryStr);
                break;
            }
            case 16: {
                cout << "\nExiting Healthcare Management System. Goodbye!\n";
                running = false;
                break;
            }
            default:
                cout << "Invalid choice: " << choice << ". Please select an option between 6 and 16.\n";
                break;
        }
    }

    return 0;
}

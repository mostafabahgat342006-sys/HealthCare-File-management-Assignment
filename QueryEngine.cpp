#include "QueryEngine.h"
#include "Integration.h"
#include <iostream>
#include <cctype>

using namespace std;

static string toUpperStr(const string& s) {
    string res = s;
    for (size_t i = 0; i < res.size(); i++) {
        res[i] = (char)toupper((unsigned char)res[i]);
    }
    return res;
}

static string trimStr(const string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

static size_t findKeywordOutsideQuotes(const string& text, const string& keyword) {
    string upperText = toUpperStr(text);
    string upperKw = toUpperStr(keyword);

    bool inQuotes = false;
    for (size_t i = 0; i + upperKw.size() <= upperText.size(); i++) {
        if (upperText[i] == '\'') {
            inQuotes = !inQuotes;
        } else if (!inQuotes) {
            if (upperText.substr(i, upperKw.size()) == upperKw) {
                return i;
            }
        }
    }
    return string::npos;
}

static bool parseSingleCondition(const string& condStr, QueryCondition& cond, string& error) {
    size_t eqPos = string::npos;
    bool inQuote = false;
    for (size_t i = 0; i < condStr.size(); i++) {
        if (condStr[i] == '\'') {
            inQuote = !inQuote;
        } else if (!inQuote && condStr[i] == '=') {
            eqPos = i;
            break;
        }
    }

    if (eqPos == string::npos) {
        error = "Invalid condition syntax in: \"" + condStr + "\". Expected format: Field='Value'.";
        return false;
    }

    string rawField = trimStr(condStr.substr(0, eqPos));
    string rawVal = trimStr(condStr.substr(eqPos + 1));

    if (rawField.empty()) {
        error = "Missing field name in condition: \"" + condStr + "\".";
        return false;
    }

    if (rawVal.size() < 2 || rawVal.front() != '\'' || rawVal.back() != '\'') {
        error = "Value for field '" + rawField + "' must be enclosed in single quotes, e.g. 'Value'.";
        return false;
    }

    cond.field = toUpperStr(rawField);
    cond.value = rawVal.substr(1, rawVal.size() - 2);
    return true;
}

QueryEngine::QueryEngine(IFileStorage& storage,
                         PrimaryIndex& doctorPrimary,
                         PrimaryIndex& appointmentPrimary,
                         SecondaryIndex& doctorNameSecondary,
                         SecondaryIndex& doctorSpecialtySecondary,
                         SecondaryIndex& appointmentDoctorSecondary)
    : storage_(storage),
      doctorPrimary_(doctorPrimary),
      appointmentPrimary_(appointmentPrimary),
      doctorNameSecondary_(doctorNameSecondary),
      doctorSpecialtySecondary_(doctorSpecialtySecondary),
      appointmentDoctorSecondary_(appointmentDoctorSecondary) {}

void QueryEngine::printDoctor(const Doctor& doctor) {
    cout << "  Doctor ID       : " << trimField(doctor.id, sizeof(doctor.id)) << "\n";
    cout << "  Doctor Name     : " << trimField(doctor.name, sizeof(doctor.name)) << "\n";
    cout << "  Doctor Specialty: " << trimField(doctor.specialty, sizeof(doctor.specialty)) << "\n";
    cout << "  Address         : " << trimField(doctor.address, sizeof(doctor.address)) << "\n";
}

void QueryEngine::printAppointment(const Appointment& appointment) {
    cout << "  Appointment ID  : " << trimField(appointment.id, sizeof(appointment.id)) << "\n";
    cout << "  Appointment Date: " << trimField(appointment.date, sizeof(appointment.date)) << "\n";
    cout << "  Doctor ID       : " << trimField(appointment.doctorId, sizeof(appointment.doctorId)) << "\n";
}

ParsedQuery QueryEngine::parseQuery(const string& queryStr) {
    ParsedQuery pq;
    pq.success = false;

    string cleaned = trimStr(queryStr);
    if (cleaned.empty()) {
        pq.error = "Error: Query cannot be empty.";
        return pq;
    }

    if (cleaned.back() == ';') {
        cleaned.pop_back();
        cleaned = trimStr(cleaned);
    }

    if (cleaned.empty()) {
        pq.error = "Error: Query cannot be empty.";
        return pq;
    }

    if (toUpperStr(cleaned.substr(0, 6)) != "SELECT" ||
        (cleaned.size() > 6 && !isspace((unsigned char)cleaned[6]))) {
        pq.error = "Error: Query must start with 'SELECT'.";
        return pq;
    }

    string rest = trimStr(cleaned.substr(6));

    size_t fromPos = findKeywordOutsideQuotes(rest, " FROM ");
    if (fromPos == string::npos) {
        pq.error = "Error: Missing 'FROM' clause in query.";
        return pq;
    }

    string selectPart = trimStr(rest.substr(0, fromPos));
    rest = trimStr(rest.substr(fromPos + 6));

    if (selectPart.empty()) {
        pq.error = "Error: No columns specified in 'SELECT'.";
        return pq;
    }

    size_t wherePos = findKeywordOutsideQuotes(rest, " WHERE ");
    if (wherePos == string::npos) {
        pq.error = "Error: Missing 'WHERE' clause. The query must specify a condition.";
        return pq;
    }

    string tablePart = trimStr(rest.substr(0, wherePos));
    string wherePart = trimStr(rest.substr(wherePos + 7));

    if (tablePart.empty()) {
        pq.error = "Error: No table specified in 'FROM' clause.";
        return pq;
    }

    if (wherePart.empty()) {
        pq.error = "Error: 'WHERE' clause cannot be empty.";
        return pq;
    }

    pq.selectField = toUpperStr(selectPart);
    pq.tableName = toUpperStr(tablePart);

    if (pq.tableName != "DOCTORS" && pq.tableName != "APPOINTMENTS") {
        pq.error = "Error: Unknown table '" + tablePart + "'. Supported tables are 'Doctors' and 'Appointments'.";
        return pq;
    }

    vector<string> rawConditions;
    string currentWhere = wherePart;
    while (true) {
        size_t andPos = findKeywordOutsideQuotes(currentWhere, " AND ");
        if (andPos == string::npos) {
            rawConditions.push_back(trimStr(currentWhere));
            break;
        }
        rawConditions.push_back(trimStr(currentWhere.substr(0, andPos)));
        currentWhere = trimStr(currentWhere.substr(andPos + 5));
    }

    for (size_t i = 0; i < rawConditions.size(); i++) {
        if (rawConditions[i].empty()) {
            pq.error = "Error: Empty condition found in 'WHERE' clause.";
            return pq;
        }
        QueryCondition cond;
        string err;
        if (!parseSingleCondition(rawConditions[i], cond, err)) {
            pq.error = "Error: " + err;
            return pq;
        }
        pq.conditions.push_back(cond);
    }

    pq.success = true;
    return pq;
}

bool QueryEngine::executeDoctorsQuery(const ParsedQuery& pq) {
    if (pq.selectField != "ALL" && pq.selectField != "DOCTOR NAME") {
        cout << "Error: Unsupported SELECT field '" << pq.selectField << "' for Doctors. Supported: 'all', 'Doctor Name'.\n";
        return false;
    }

    if (pq.conditions.size() == 1 && pq.conditions[0].field == "DOCTOR ID") {
        string docId = pq.conditions[0].value;
        cout << "[Query Engine] Using Doctor Primary Index (Binary Search) for Doctor ID: '" << docId << "'...\n";

        int offset = doctorPrimary_.search(docId);
        if (offset < 0) {
            cout << "Result: No Doctor found with Doctor ID = '" << docId << "'.\n";
            return true;
        }

        Doctor doc;
        if (!storage_.readDoctorAt(offset, doc)) {
            cout << "Error: Could not read Doctor record from data file at offset " << offset << ".\n";
            return false;
        }

        cout << "----------------------------------------\n";
        if (pq.selectField == "DOCTOR NAME") {
            cout << "  Doctor Name     : " << trimField(doc.name, sizeof(doc.name))
                 << "  (Doctor ID: " << trimField(doc.id, sizeof(doc.id)) << ")\n";
        } else {
            printDoctor(doc);
        }
        cout << "----------------------------------------\n";
        cout << "(1 record found)\n";
        return true;
    }

    if (pq.conditions.size() == 1 && pq.conditions[0].field == "DOCTOR NAME") {
        string docName = pq.conditions[0].value;
        cout << "[Query Engine] Using Doctor Name Secondary Index for Doctor Name: '" << docName << "'...\n";

        vector<string> matchedIds = doctorNameSecondary_.search(docName);
        if (matchedIds.empty()) {
            cout << "Result: No Doctors found with Doctor Name = '" << docName << "'.\n";
            return true;
        }

        cout << "[Query Engine] Resolving " << matchedIds.size()
             << " matching Doctor ID(s) using Doctor Primary Index...\n";

        int count = 0;
        cout << "----------------------------------------\n";
        for (size_t i = 0; i < matchedIds.size(); i++) {
            int offset = doctorPrimary_.search(matchedIds[i]);
            if (offset < 0) continue;

            Doctor doc;
            if (storage_.readDoctorAt(offset, doc)) {
                if (pq.selectField == "DOCTOR NAME") {
                    cout << "  Doctor Name     : " << trimField(doc.name, sizeof(doc.name))
                         << "  (Doctor ID: " << trimField(doc.id, sizeof(doc.id)) << ")\n";
                } else {
                    printDoctor(doc);
                    if (i + 1 < matchedIds.size()) cout << "  ---\n";
                }
                count++;
            }
        }
        cout << "----------------------------------------\n";
        cout << "(" << count << " record(s) found)\n";
        return true;
    }

    if (pq.conditions.size() == 1 && pq.conditions[0].field == "DOCTOR SPECIALTY") {
        string specialty = pq.conditions[0].value;
        cout << "[Query Engine] Using Doctor Specialty Secondary Index for Specialty: '" << specialty << "'...\n";

        vector<string> matchedIds = doctorSpecialtySecondary_.search(specialty);
        if (matchedIds.empty()) {
            cout << "Result: No Doctors found with Doctor Specialty = '" << specialty << "'.\n";
            return true;
        }

        cout << "[Query Engine] Resolving " << matchedIds.size()
             << " matching Doctor ID(s) using Doctor Primary Index...\n";

        int count = 0;
        cout << "----------------------------------------\n";
        for (size_t i = 0; i < matchedIds.size(); i++) {
            int offset = doctorPrimary_.search(matchedIds[i]);
            if (offset < 0) continue;

            Doctor doc;
            if (storage_.readDoctorAt(offset, doc)) {
                printDoctor(doc);
                if (i + 1 < matchedIds.size()) cout << "  ---\n";
                count++;
            }
        }
        cout << "----------------------------------------\n";
        cout << "(" << count << " record(s) found)\n";
        return true;
    }

    if (pq.conditions.size() == 2) {
        string nameVal, specVal;
        bool hasName = false, hasSpec = false;

        for (size_t i = 0; i < 2; i++) {
            if (pq.conditions[i].field == "DOCTOR NAME") {
                nameVal = pq.conditions[i].value;
                hasName = true;
            } else if (pq.conditions[i].field == "DOCTOR SPECIALTY") {
                specVal = pq.conditions[i].value;
                hasSpec = true;
            }
        }

        if (!hasName || !hasSpec) {
            cout << "Error: For Doctors with multiple conditions, expected 'Doctor Name' AND 'Doctor Specialty'.\n";
            return false;
        }

        cout << "[Query Engine] Searching Doctor Name Secondary Index for: '" << nameVal << "'...\n";
        vector<string> nameIds = doctorNameSecondary_.search(nameVal);

        cout << "[Query Engine] Searching Doctor Specialty Secondary Index for: '" << specVal << "'...\n";
        vector<string> specIds = doctorSpecialtySecondary_.search(specVal);

        cout << "[Query Engine] Computing intersection of matching Doctor IDs...\n";
        vector<string> intersectedIds;
        for (size_t i = 0; i < nameIds.size(); i++) {
            for (size_t j = 0; j < specIds.size(); j++) {
                if (nameIds[i] == specIds[j]) {
                    intersectedIds.push_back(nameIds[i]);
                    break;
                }
            }
        }

        if (intersectedIds.empty()) {
            cout << "Result: No Doctors found matching both Doctor Name = '" << nameVal
                 << "' AND Doctor Specialty = '" << specVal << "'.\n";
            return true;
        }

        cout << "[Query Engine] Resolving " << intersectedIds.size()
             << " intersected Doctor ID(s) using Doctor Primary Index...\n";

        int count = 0;
        cout << "----------------------------------------\n";
        for (size_t i = 0; i < intersectedIds.size(); i++) {
            int offset = doctorPrimary_.search(intersectedIds[i]);
            if (offset < 0) continue;

            Doctor doc;
            if (storage_.readDoctorAt(offset, doc)) {
                printDoctor(doc);
                if (i + 1 < intersectedIds.size()) cout << "  ---\n";
                count++;
            }
        }
        cout << "----------------------------------------\n";
        cout << "(" << count << " record(s) found)\n";
        return true;
    }

    cout << "Error: Unsupported condition(s) for Doctors query.\n";
    return false;
}

bool QueryEngine::executeAppointmentsQuery(const ParsedQuery& pq) {
    if (pq.selectField != "ALL") {
        cout << "Error: Unsupported SELECT field '" << pq.selectField << "' for Appointments. Supported: 'all'.\n";
        return false;
    }

    if (pq.conditions.size() == 1 && pq.conditions[0].field == "DOCTOR ID") {
        string docId = pq.conditions[0].value;
        cout << "[Query Engine] Using Appointment Doctor ID Secondary Index for Doctor ID: '" << docId << "'...\n";

        vector<string> appIds = appointmentDoctorSecondary_.search(docId);
        if (appIds.empty()) {
            cout << "Result: No Appointments found for Doctor ID = '" << docId << "'.\n";
            return true;
        }

        cout << "[Query Engine] Resolving " << appIds.size()
             << " matching Appointment ID(s) using Appointment Primary Index...\n";

        int count = 0;
        cout << "----------------------------------------\n";
        for (size_t i = 0; i < appIds.size(); i++) {
            int offset = appointmentPrimary_.search(appIds[i]);
            if (offset < 0) continue;

            Appointment app;
            if (storage_.readAppointmentAt(offset, app)) {
                printAppointment(app);
                if (i + 1 < appIds.size()) cout << "  ---\n";
                count++;
            }
        }
        cout << "----------------------------------------\n";
        cout << "(" << count << " record(s) found)\n";
        return true;
    }

    cout << "Error: Appointments table only supports searching by 'Doctor ID'.\n";
    return false;
}

bool QueryEngine::executeQuery(const string& queryString) {
    ParsedQuery pq = parseQuery(queryString);
    if (!pq.success) {
        cout << pq.error << "\n";
        return false;
    }

    if (pq.tableName == "DOCTORS") {
        return executeDoctorsQuery(pq);
    } else if (pq.tableName == "APPOINTMENTS") {
        return executeAppointmentsQuery(pq);
    }

    cout << "Error: Unknown table '" << pq.tableName << "'.\n";
    return false;
}

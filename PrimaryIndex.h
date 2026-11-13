#pragma once
#include <string>
#include <vector>
using namespace std;

struct IndexEntry {
    string id;
    int offset;
};

class PrimaryIndex {
public:
    vector<IndexEntry> entries;
    int search(string id);
    bool insert(string id, int offset);
    bool remove(string id);
    bool updateKey(string oldId, string newId);
    void rebuildDoctors(string filename);
    void rebuildAppointments(string filename);
    void print();
};

void rebuildDoctorIndex(PrimaryIndex& doctorIndex, string filename);
void rebuildAppointmentIndex(PrimaryIndex& appointmentIndex, string filename);

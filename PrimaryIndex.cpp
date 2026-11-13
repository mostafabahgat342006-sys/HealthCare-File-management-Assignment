#include "PrimaryIndex.h"
#include "Doctor.h"
#include "Appointment.h"
#include <fstream>
#include <cstring>
#include <iostream>
using namespace std;

int PrimaryIndex::search(string id)
{

    int low = 0;
    int high = entries.size() - 1;

    while (low <= high)
    {

        int mid = low + (high - low) / 2;

        if (entries[mid].id == id)
            return entries[mid].offset;

        else if (entries[mid].id < id)
            low = mid + 1;

        else
            high = mid - 1;
    }

    return -1;
}

bool PrimaryIndex::insert(string id, int offset)
{
    int pos = 0;

    while (pos < (int)entries.size() && entries[pos].id < id)
    {
        pos++;
    }

    if (pos < (int)entries.size() && entries[pos].id == id)
        return false;

    IndexEntry e;
    e.id = id;
    e.offset = offset;

    entries.insert(entries.begin() + pos, e);
    return true;
}

bool PrimaryIndex::remove(string id)
{
    for (int i = 0; i < (int)entries.size(); i++)
    {
        if (entries[i].id == id)
        {
            entries.erase(entries.begin() + i);
            return true;
        }
    }
    return false;
}

bool PrimaryIndex::updateKey(string oldId, string newId)
{
    int offset = search(oldId);

    if (offset == -1)
        return false;

    if (search(newId) != -1)
        return false;

    remove(oldId);

    insert(newId, offset);
    return true;
}

static string cleanId(const char* s, int length)
{
    int end = length;
    while (end > 0 && (s[end - 1] == ' ' || s[end - 1] == '\0'))
        --end;
    return string(s, end);
}

void rebuildDoctorIndex(PrimaryIndex& doctorIndex, string filename)
{
    doctorIndex.entries.clear();

    ifstream file(filename, ios::binary);

    if (!file)
        return;

    file.seekg(sizeof(int));

    Doctor doctor;
    int offset;

    while (true)
    {
        offset = (int)file.tellg();

        file.read((char*)&doctor, sizeof(Doctor));

        if (!file)
            break;

        if (doctor.id[0] != '*') {
            doctorIndex.insert(cleanId(doctor.id, sizeof(doctor.id)), offset);
        }
    }

    file.close();
}

void rebuildAppointmentIndex(PrimaryIndex& appointmentIndex, string filename)
{
    appointmentIndex.entries.clear();

    ifstream file(filename, ios::binary);

    if (!file)
        return;

    file.seekg(sizeof(int));

    Appointment appointment;
    int offset;

    while (true)
    {
        offset = (int)file.tellg();

        file.read((char*)&appointment, sizeof(Appointment));

        if (!file)
            break;

        if (appointment.id[0] != '*')
        {
            appointmentIndex.insert(cleanId(appointment.id, sizeof(appointment.id)), offset);
        }
    }

    file.close();
}

void PrimaryIndex::rebuildDoctors(string filename)
{
    rebuildDoctorIndex(*this, filename);
}

void PrimaryIndex::rebuildAppointments(string filename)
{
    rebuildAppointmentIndex(*this, filename);
}

void PrimaryIndex::print()
{
    for (int i = 0; i < (int)entries.size(); i++)
        cout << entries[i].id << " -> " << entries[i].offset << '\n';
}

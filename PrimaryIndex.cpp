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

void PrimaryIndex::insert(string id, int offset)
{

    int pos = 0;

    while (pos < entries.size() && entries[pos].id < id)
    {
        pos++;
    }

    IndexEntry e;
    e.id = id;
    e.offset = offset;

    entries.insert(entries.begin() + pos, e);
}

void PrimaryIndex::remove(string id)
{

    for (int i = 0; i < entries.size(); i++)
        {

        if (entries[i].id == id)
        {
            entries.erase(entries.begin() + i);
            return;
        }
    }
}

void PrimaryIndex::updateKey(string oldId, string newId)
{

    int offset = search(oldId);

    if (offset == -1)
        return;

    if (search(newId) != -1)
        return;

    remove(oldId);

    insert(newId, offset);
}

void rebuildDoctorIndex(PrimaryIndex& doctorIndex, string filename)
{

    ifstream file(filename, ios::binary);

    if (!file)
        return;

    file.seekg(sizeof(int));

    Doctor doctor;
    int offset;

    while (true)
    {

        offset = file.tellg();

        file.read((char*)&doctor, sizeof(Doctor));

        if (!file)
            break;

        if (doctor.id[0] != '*') {
            doctorIndex.insert(doctor.id, offset);
        }
    }

    file.close();
}

void rebuildAppointmentIndex(PrimaryIndex& appointmentIndex,string filename)
{

    ifstream file(filename, ios::binary);

    if (!file)
        return;

    file.seekg(sizeof(int));

    Appointment appointment;
    int offset;

    while (true)
    {

        offset = file.tellg();

        file.read((char*)&appointment, sizeof(Appointment));

        if (!file)
            break;

        if (appointment.id[0] != '*')
        {
            appointmentIndex.insert(appointment.id, offset);
        }
    }

    file.close();
}

void PrimaryIndex::print()
{
    for (int i = 0; i < (int)entries.size(); i++)
        cout << entries[i].id << " -> " << entries[i].offset << '\n';
}

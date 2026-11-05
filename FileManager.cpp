#include "FileManager.h"
#include <fstream>
#include <cstring>
using namespace std;

void initializeFile(string filename)
{
    fstream file(filename, ios::in | ios::binary);

    if (!file)
    {
        file.close();
        file.open(filename, ios::out | ios::binary);

        int head = -1;
        file.write((char*)&head, sizeof(int));
    }

    file.close();
}

int readHeader(string filename)
{

    ifstream file(filename, ios::binary);

    int head;

    file.read((char*)&head, sizeof(int));

    file.close();

    return head;
}

int writeDoctor(Doctor doctor, string filename)
{

    fstream file(filename, ios::in | ios::out | ios::binary);

    int head;
    file.seekg(0);
    file.read((char*)&head, sizeof(int));

    int offset;

    if (head == -1)
    {
        file.seekp(0, ios::end);
        offset = file.tellp();
    }
    else
    {
        offset = head;

        int next;
        file.seekg(head + 1);
        file.read((char*)&next, sizeof(int));

        file.seekp(0);
        file.write((char*)&next, sizeof(int));
    }

    file.seekp(offset);
    file.write((char*)&doctor, sizeof(Doctor));

    file.close();

    return offset;
}

int writeAppointment(Appointment appointment, string filename)
{

    fstream file(filename, ios::in | ios::out | ios::binary);

    int head;
    file.seekg(0);
    file.read((char*)&head, sizeof(int));

    int offset;

    if (head == -1)
    {
        file.seekp(0, ios::end);
        offset = file.tellp();
    }
    else
    {
        offset = head;

        int next;
        file.seekg(head + 1);
        file.read((char*)&next, sizeof(int));

        file.seekp(0);
        file.write((char*)&next, sizeof(int));
    }

    file.seekp(offset);
    file.write((char*)&appointment, sizeof(Appointment));

    file.close();

    return offset;
}

Doctor readDoctor(int offset, string filename)
{
    Doctor doctor = {};

    ifstream file(filename, ios::binary);

    file.seekg(offset);

    file.read((char*)&doctor, sizeof(Doctor));

    file.close();

    return doctor;
}

Appointment readAppointment(int offset, string filename)
{

    Appointment appointment = {};

    ifstream file(filename, ios::binary);

    file.seekg(offset);

    file.read((char*)&appointment, sizeof(Appointment));

    file.close();

    return appointment;
}

void deleteDoctorAt(int offset, string filename)
{
    fstream file(filename, ios::in | ios::out | ios::binary);

    int head;
    file.seekg(0);
    file.read((char*)&head, sizeof(int));

    char mark = '*';
    file.seekp(offset);
    file.write(&mark, 1);

    file.seekp(offset + 1);
    file.write((char*)&head, sizeof(int));

    file.seekp(0);
    file.write((char*)&offset, sizeof(int));

    file.close();
}

void deleteAppointmentAt(int offset, string filename)
{

    fstream file(filename, ios::in | ios::out | ios::binary);

    int head;
    file.seekg(0);
    file.read((char*)&head, sizeof(int));

    char mark = '*';
    file.seekp(offset);
    file.write(&mark, 1);

    file.seekp(offset + 1);
    file.write((char*)&head, sizeof(int));

    file.seekp(0);
    file.write((char*)&offset, sizeof(int));

    file.close();
}
#pragma once
#include <string>
#include "Doctor.h"
#include "Appointment.h"
using namespace std;

void initializeFile(string filename);
int readHeader(string filename);
int writeDoctor(Doctor doctor, string filename);
int writeAppointment(Appointment appointment, string filename);
bool writeDoctorAt(int offset, const Doctor& doctor, string filename);
bool writeAppointmentAt(int offset, const Appointment& appointment, string filename);
bool readDoctor(int offset, Doctor& doctor, string filename);
bool readAppointment(int offset, Appointment& appointment, string filename);
Doctor readDoctor(int offset, string filename);
Appointment readAppointment(int offset, string filename);
bool deleteDoctorAt(int offset, string filename);
bool deleteAppointmentAt(int offset, string filename);

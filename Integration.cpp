#include "Integration.h"
#include <cstring>

std::string trimField(const char* field, int length) {
    int end = length;
    while (end > 0 && (field[end - 1] == ' ' || field[end - 1] == '\0'))
        --end;
    return std::string(field, end);
}

FileStorageAdapter::FileStorageAdapter(const std::string& doctorFile,
                                       const std::string& appointmentFile)
    : doctorFile_(doctorFile), appointmentFile_(appointmentFile) {}

bool FileStorageAdapter::readDoctorAt(int byteOffset, Doctor& out) {
    return readDoctor(byteOffset, out, doctorFile_);
}

bool FileStorageAdapter::readAppointmentAt(int byteOffset, Appointment& out) {
    return readAppointment(byteOffset, out, appointmentFile_);
}

int FileStorageAdapter::addDoctorRecord(const Doctor& doctor) {
    return writeDoctor(doctor, doctorFile_);
}

int FileStorageAdapter::addAppointmentRecord(const Appointment& appointment) {
    return writeAppointment(appointment, appointmentFile_);
}

bool FileStorageAdapter::writeDoctorAt(int byteOffset, const Doctor& doctor) {
    return ::writeDoctorAt(byteOffset, doctor, doctorFile_);
}

bool FileStorageAdapter::writeAppointmentAt(int byteOffset, const Appointment& appointment) {
    return ::writeAppointmentAt(byteOffset, appointment, appointmentFile_);
}

bool FileStorageAdapter::deleteDoctorRecord(int byteOffset) {
    return deleteDoctorAt(byteOffset, doctorFile_);
}

bool FileStorageAdapter::deleteAppointmentRecord(int byteOffset) {
    return deleteAppointmentAt(byteOffset, appointmentFile_);
}

IndexManagerAdapter::IndexManagerAdapter(PrimaryIndex& doctorPrimary,
                                         PrimaryIndex& appointmentPrimary,
                                         SecondaryIndex& doctorNameSecondary,
                                         SecondaryIndex& doctorSpecialtySecondary,
                                         SecondaryIndex& appointmentDoctorSecondary)
    : doctorPrimary_(doctorPrimary),
      appointmentPrimary_(appointmentPrimary),
      doctorNameSecondary_(doctorNameSecondary),
      doctorSpecialtySecondary_(doctorSpecialtySecondary),
      appointmentDoctorSecondary_(appointmentDoctorSecondary) {}

bool IndexManagerAdapter::doctorIdExists(const std::string& doctorId) const {
    return doctorPrimary_.search(doctorId) != -1;
}

bool IndexManagerAdapter::appointmentIdExists(const std::string& appointmentId) const {
    return appointmentPrimary_.search(appointmentId) != -1;
}

int IndexManagerAdapter::findDoctorOffset(const std::string& doctorId) const {
    return doctorPrimary_.search(doctorId);
}

int IndexManagerAdapter::findAppointmentOffset(const std::string& appointmentId) const {
    return appointmentPrimary_.search(appointmentId);
}

bool IndexManagerAdapter::insertDoctorPrimary(const std::string& doctorId, int offset) {
    return doctorPrimary_.insert(doctorId, offset);
}

bool IndexManagerAdapter::insertAppointmentPrimary(const std::string& appointmentId, int offset) {
    return appointmentPrimary_.insert(appointmentId, offset);
}

bool IndexManagerAdapter::eraseDoctorPrimary(const std::string& doctorId) {
    return doctorPrimary_.remove(doctorId);
}

bool IndexManagerAdapter::eraseAppointmentPrimary(const std::string& appointmentId) {
    return appointmentPrimary_.remove(appointmentId);
}

bool IndexManagerAdapter::changeDoctorPrimary(const std::string& oldId,
                                             const std::string& newId,
                                             int sameOffset) {
    (void)sameOffset;
    return doctorPrimary_.updateKey(oldId, newId);
}

bool IndexManagerAdapter::insertDoctorSecondary(const Doctor& doctor) {
    std::string id = trimField(doctor.id, sizeof(doctor.id));
    std::string name = trimField(doctor.name, sizeof(doctor.name));
    std::string specialty = trimField(doctor.specialty, sizeof(doctor.specialty));

    bool b1 = doctorNameSecondary_.insert(name, id);
    bool b2 = doctorSpecialtySecondary_.insert(specialty, id);
    return b1 && b2;
}

bool IndexManagerAdapter::eraseDoctorSecondary(const Doctor& doctor) {
    std::string id = trimField(doctor.id, sizeof(doctor.id));
    std::string name = trimField(doctor.name, sizeof(doctor.name));
    std::string specialty = trimField(doctor.specialty, sizeof(doctor.specialty));

    bool b1 = doctorNameSecondary_.remove(name, id);
    bool b2 = doctorSpecialtySecondary_.remove(specialty, id);
    return b1 && b2;
}

bool IndexManagerAdapter::updateDoctorNameSecondary(const std::string& doctorId,
                                                   const std::string& oldName,
                                                   const std::string& newName) {
    return doctorNameSecondary_.update(oldName, newName, doctorId);
}

bool IndexManagerAdapter::updateDoctorSpecialtySecondary(const std::string& doctorId,
                                                        const std::string& oldSpecialty,
                                                        const std::string& newSpecialty) {
    return doctorSpecialtySecondary_.update(oldSpecialty, newSpecialty, doctorId);
}

bool IndexManagerAdapter::insertAppointmentDoctorSecondary(const std::string& appointmentId,
                                                          const std::string& doctorId) {
    return appointmentDoctorSecondary_.insert(doctorId, appointmentId);
}

bool IndexManagerAdapter::eraseAppointmentDoctorSecondary(const std::string& appointmentId,
                                                          const std::string& doctorId) {
    return appointmentDoctorSecondary_.remove(doctorId, appointmentId);
}

bool IndexManagerAdapter::updateAppointmentDoctorSecondary(const std::string& appointmentId,
                                                          const std::string& oldDoctorId,
                                                          const std::string& newDoctorId) {
    return appointmentDoctorSecondary_.update(oldDoctorId, newDoctorId, appointmentId);
}

std::vector<std::string>
IndexManagerAdapter::appointmentIdsForDoctor(const std::string& doctorId) const {
    return appointmentDoctorSecondary_.search(doctorId);
}

void rebuildAllIndexes(PrimaryIndex& doctorPrimary,
                       PrimaryIndex& appointmentPrimary,
                       SecondaryIndex& doctorNameSecondary,
                       SecondaryIndex& doctorSpecialtySecondary,
                       SecondaryIndex& appointmentDoctorSecondary,
                       const std::string& doctorFile,
                       const std::string& appointmentFile) {
    rebuildDoctorIndex(doctorPrimary, doctorFile);
    rebuildAppointmentIndex(appointmentPrimary, appointmentFile);
    rebuildDoctorNameIndex(doctorNameSecondary, doctorFile);
    rebuildDoctorSpecialtyIndex(doctorSpecialtySecondary, doctorFile);
    rebuildAppointmentDoctorIndex(appointmentDoctorSecondary, appointmentFile);
}

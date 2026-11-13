#pragma once

#include <string>
#include <vector>
#include "Doctor.h"
#include "Appointment.h"
#include "CRUD.h"
#include "FileManager.h"
#include "PrimaryIndex.h"
#include "SecondaryIndex.h"

std::string trimField(const char* field, int length);

class FileStorageAdapter : public IFileStorage {
private:
    std::string doctorFile_;
    std::string appointmentFile_;

public:
    FileStorageAdapter(const std::string& doctorFile, const std::string& appointmentFile);

    bool readDoctorAt(int byteOffset, Doctor& out) override;
    bool readAppointmentAt(int byteOffset, Appointment& out) override;
    int addDoctorRecord(const Doctor& doctor) override;
    int addAppointmentRecord(const Appointment& appointment) override;
    bool writeDoctorAt(int byteOffset, const Doctor& doctor) override;
    bool writeAppointmentAt(int byteOffset, const Appointment& appointment) override;
    bool deleteDoctorRecord(int byteOffset) override;
    bool deleteAppointmentRecord(int byteOffset) override;
};

class IndexManagerAdapter : public IIndexManager {
private:
    PrimaryIndex& doctorPrimary_;
    PrimaryIndex& appointmentPrimary_;
    SecondaryIndex& doctorNameSecondary_;
    SecondaryIndex& doctorSpecialtySecondary_;
    SecondaryIndex& appointmentDoctorSecondary_;

public:
    IndexManagerAdapter(PrimaryIndex& doctorPrimary,
                        PrimaryIndex& appointmentPrimary,
                        SecondaryIndex& doctorNameSecondary,
                        SecondaryIndex& doctorSpecialtySecondary,
                        SecondaryIndex& appointmentDoctorSecondary);

    bool doctorIdExists(const std::string& doctorId) const override;
    bool appointmentIdExists(const std::string& appointmentId) const override;
    int findDoctorOffset(const std::string& doctorId) const override;
    int findAppointmentOffset(const std::string& appointmentId) const override;
    bool insertDoctorPrimary(const std::string& doctorId, int offset) override;
    bool insertAppointmentPrimary(const std::string& appointmentId, int offset) override;
    bool eraseDoctorPrimary(const std::string& doctorId) override;
    bool eraseAppointmentPrimary(const std::string& appointmentId) override;
    bool changeDoctorPrimary(const std::string& oldId,
                             const std::string& newId,
                             int sameOffset) override;

    bool insertDoctorSecondary(const Doctor& doctor) override;
    bool eraseDoctorSecondary(const Doctor& doctor) override;
    bool updateDoctorNameSecondary(const std::string& doctorId,
                                   const std::string& oldName,
                                   const std::string& newName) override;
    bool updateDoctorSpecialtySecondary(const std::string& doctorId,
                                        const std::string& oldSpecialty,
                                        const std::string& newSpecialty) override;
    bool insertAppointmentDoctorSecondary(const std::string& appointmentId,
                                          const std::string& doctorId) override;
    bool eraseAppointmentDoctorSecondary(const std::string& appointmentId,
                                          const std::string& doctorId) override;
    bool updateAppointmentDoctorSecondary(const std::string& appointmentId,
                                          const std::string& oldDoctorId,
                                          const std::string& newDoctorId) override;

    std::vector<std::string> appointmentIdsForDoctor(const std::string& doctorId) const override;
};

void rebuildAllIndexes(PrimaryIndex& doctorPrimary,
                       PrimaryIndex& appointmentPrimary,
                       SecondaryIndex& doctorNameSecondary,
                       SecondaryIndex& doctorSpecialtySecondary,
                       SecondaryIndex& appointmentDoctorSecondary,
                       const std::string& doctorFile,
                       const std::string& appointmentFile);

#ifndef MEMBER4_CRUD_H
#define MEMBER4_CRUD_H

#include <string>
#include <vector>
#include "Doctor.h"
#include "Appointment.h"

class IFileStorage {
public:
    virtual ~IFileStorage() {}

    virtual bool readDoctorAt(int byteOffset, Doctor& out) = 0;
    virtual bool readAppointmentAt(int byteOffset, Appointment& out) = 0;

    virtual int addDoctorRecord(const Doctor& doctor) = 0;
    virtual int addAppointmentRecord(const Appointment& appointment) = 0;

    virtual bool writeDoctorAt(int byteOffset, const Doctor& doctor) = 0;
    virtual bool writeAppointmentAt(int byteOffset, const Appointment& appointment) = 0;

    virtual bool deleteDoctorRecord(int byteOffset) = 0;
    virtual bool deleteAppointmentRecord(int byteOffset) = 0;
};

class IIndexManager {
public:
    virtual ~IIndexManager() {}

    virtual bool doctorIdExists(const std::string& doctorId) const = 0;
    virtual bool appointmentIdExists(const std::string& appointmentId) const = 0;
    virtual int findDoctorOffset(const std::string& doctorId) const = 0;
    virtual int findAppointmentOffset(const std::string& appointmentId) const = 0;
    virtual bool insertDoctorPrimary(const std::string& doctorId, int offset) = 0;
    virtual bool insertAppointmentPrimary(const std::string& appointmentId, int offset) = 0;
    virtual bool eraseDoctorPrimary(const std::string& doctorId) = 0;
    virtual bool eraseAppointmentPrimary(const std::string& appointmentId) = 0;
    virtual bool changeDoctorPrimary(const std::string& oldId,
                                     const std::string& newId,
                                     int sameOffset) = 0;

    virtual bool insertDoctorSecondary(const Doctor& doctor) = 0;
    virtual bool eraseDoctorSecondary(const Doctor& doctor) = 0;
    virtual bool updateDoctorNameSecondary(const std::string& doctorId,
                                           const std::string& oldName,
                                           const std::string& newName) = 0;
    virtual bool updateDoctorSpecialtySecondary(const std::string& doctorId,
                                                const std::string& oldSpecialty,
                                                const std::string& newSpecialty) = 0;
    virtual bool insertAppointmentDoctorSecondary(const std::string& appointmentId,
                                                  const std::string& doctorId) = 0;
    virtual bool eraseAppointmentDoctorSecondary(const std::string& appointmentId,
                                                  const std::string& doctorId) = 0;
    virtual bool updateAppointmentDoctorSecondary(const std::string& appointmentId,
                                                  const std::string& oldDoctorId,
                                                  const std::string& newDoctorId) = 0;

    virtual std::vector<std::string>
    appointmentIdsForDoctor(const std::string& doctorId) const = 0;
};

class CRUDOperations {
public:
    CRUDOperations(IFileStorage& storage, IIndexManager& indexes);

    bool addDoctor(const std::string& id, const std::string& name,
                   const std::string& specialty, const std::string& address,
                   std::string& message);

    bool addAppointment(const std::string& id, const std::string& date,
                        const std::string& doctorId, std::string& message);

    bool deleteAppointment(const std::string& appointmentId,
                           std::string& message);
    bool deleteDoctor(const std::string& doctorId, std::string& message);

    bool updateDoctorName(const std::string& doctorId,
                          const std::string& newName, std::string& message);
    bool updateDoctorId(const std::string& oldId,
                        const std::string& newId, std::string& message);
    bool updateAppointmentDate(const std::string& appointmentId,
                               const std::string& newDate, std::string& message);

private:
    IFileStorage& storage_;
    IIndexManager& indexes_;

    static std::string fieldToString(const char* field, int length);
    static bool validateField(const std::string& value, int maxLength,
                               const std::string& fieldName,
                               std::string& message);
    static void copyField(char* destination, int length,
                          const std::string& value);
};

#endif

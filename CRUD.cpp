#include "CRUD.h"

#include <cstring>

CRUDOperations::CRUDOperations(IFileStorage& storage, IIndexManager& indexes)
    : storage_(storage), indexes_(indexes) {}

std::string CRUDOperations::fieldToString(const char* field, int length) {
    int end = length;
    while (end > 0 && (field[end - 1] == ' ' || field[end - 1] == '\0'))
        --end;
    return std::string(field, end);
}

bool CRUDOperations::validateField(const std::string& value, int maxLength,
                                   const std::string& fieldName,
                                   std::string& message) {
    if (value.empty()) {
        message = fieldName + " cannot be empty.";
        return false;
    }
    if (static_cast<int>(value.size()) > maxLength) {
        message = fieldName + " cannot exceed " +
                  std::to_string(maxLength) + " characters.";
        return false;
    }
    if (value[0] == '*') {
        message = fieldName + " cannot start with '*'.";
        return false;
    }
    message.clear();
    return true;
}

void CRUDOperations::copyField(char* destination, int length,
                               const std::string& value) {
    std::memset(destination, ' ', length);
    std::memcpy(destination, value.data(), value.size());
}

bool CRUDOperations::addDoctor(const std::string& id,
                               const std::string& name,
                               const std::string& specialty,
                               const std::string& address,
                               std::string& message) {
    if (!validateField(id, 15, "Doctor ID", message) ||
        !validateField(name, 30, "Doctor Name", message) ||
        !validateField(specialty, 20, "Doctor Specialty", message) ||
        !validateField(address, 30, "Address", message))
        return false;

    if (indexes_.doctorIdExists(id)) {
        message = "Doctor ID already exists.";
        return false;
    }

    Doctor doctor;
    copyField(doctor.id, 15, id);
    copyField(doctor.name, 30, name);
    copyField(doctor.specialty, 20, specialty);
    copyField(doctor.address, 30, address);

    const int offset = storage_.addDoctorRecord(doctor);
    if (offset < 0) {
        message = "Could not write Doctor record.";
        return false;
    }

    if (!indexes_.insertDoctorPrimary(id, offset) ||
        !indexes_.insertDoctorSecondary(doctor)) {
        message = "Doctor was written, but an index update failed. "
                  "The team must rebuild indexes from the data files.";
        return false;
    }

    message = "Doctor added successfully.";
    return true;
}

bool CRUDOperations::addAppointment(const std::string& id,
                                    const std::string& date,
                                    const std::string& doctorId,
                                    std::string& message) {
    if (!validateField(id, 15, "Appointment ID", message) ||
        !validateField(date, 30, "Appointment Date", message) ||
        !validateField(doctorId, 15, "Doctor ID", message))
        return false;

    if (indexes_.appointmentIdExists(id)) {
        message = "Appointment ID already exists.";
        return false;
    }
    if (!indexes_.doctorIdExists(doctorId)) {
        message = "Cannot add appointment: Doctor ID does not exist.";
        return false;
    }

    Appointment appointment;
    copyField(appointment.id, 15, id);
    copyField(appointment.date, 30, date);
    copyField(appointment.doctorId, 15, doctorId);

    const int offset = storage_.addAppointmentRecord(appointment);
    if (offset < 0) {
        message = "Could not write Appointment record.";
        return false;
    }

    if (!indexes_.insertAppointmentPrimary(id, offset) ||
        !indexes_.insertAppointmentDoctorSecondary(id, doctorId)) {
        message = "Appointment was written, but an index update failed. "
                  "The team must rebuild indexes from the data files.";
        return false;
    }

    message = "Appointment added successfully.";
    return true;
}

bool CRUDOperations::deleteAppointment(const std::string& appointmentId,
                                       std::string& message) {
    const int offset = indexes_.findAppointmentOffset(appointmentId);
    if (offset < 0) {
        message = "Appointment ID was not found.";
        return false;
    }

    Appointment appointment;
    if (!storage_.readAppointmentAt(offset, appointment)) {
        message = "Could not read Appointment record.";
        return false;
    }

    const std::string doctorId = fieldToString(appointment.doctorId, 15);
    if (!storage_.deleteAppointmentRecord(offset)) {
        message = "Could not logically delete Appointment record.";
        return false;
    }

    indexes_.eraseAppointmentPrimary(appointmentId);
    indexes_.eraseAppointmentDoctorSecondary(appointmentId, doctorId);

    message = "Appointment deleted successfully.";
    return true;
}

bool CRUDOperations::deleteDoctor(const std::string& doctorId,
                                  std::string& message) {
    const int offset = indexes_.findDoctorOffset(doctorId);
    if (offset < 0) {
        message = "Doctor ID was not found.";
        return false;
    }

    const std::vector<std::string> linkedAppointments =
        indexes_.appointmentIdsForDoctor(doctorId);
    if (!linkedAppointments.empty()) {
        message = "Cannot delete Doctor: delete the doctor's appointments first.";
        return false;
    }

    Doctor doctor;
    if (!storage_.readDoctorAt(offset, doctor)) {
        message = "Could not read Doctor record.";
        return false;
    }

    if (!storage_.deleteDoctorRecord(offset)) {
        message = "Could not logically delete Doctor record.";
        return false;
    }

    indexes_.eraseDoctorPrimary(doctorId);
    indexes_.eraseDoctorSecondary(doctor);

    message = "Doctor deleted successfully.";
    return true;
}

bool CRUDOperations::updateDoctorName(const std::string& doctorId,
                                      const std::string& newName,
                                      std::string& message) {
    if (!validateField(newName, 30, "Doctor Name", message))
        return false;

    const int offset = indexes_.findDoctorOffset(doctorId);
    if (offset < 0) {
        message = "Doctor ID was not found.";
        return false;
    }

    Doctor doctor;
    if (!storage_.readDoctorAt(offset, doctor)) {
        message = "Could not read Doctor record.";
        return false;
    }

    const std::string oldName = fieldToString(doctor.name, 30);
    if (oldName == newName) {
        message = "The new name is the same as the current name.";
        return false;
    }

    copyField(doctor.name, 30, newName);
    if (!storage_.writeDoctorAt(offset, doctor)) {
        message = "Could not update Doctor record.";
        return false;
    }

    if (!indexes_.updateDoctorNameSecondary(doctorId, oldName, newName)) {
        message = "Doctor name was updated, but its secondary index update failed.";
        return false;
    }

    message = "Doctor name updated successfully.";
    return true;
}

bool CRUDOperations::updateDoctorId(const std::string& oldId,
                                    const std::string& newId,
                                    std::string& message) {
    if (!validateField(newId, 15, "New Doctor ID", message))
        return false;

    const int doctorOffset = indexes_.findDoctorOffset(oldId);
    if (doctorOffset < 0) {
        message = "Current Doctor ID was not found.";
        return false;
    }
    if (oldId == newId) {
        message = "New Doctor ID is the same as the current ID.";
        return false;
    }
    if (indexes_.doctorIdExists(newId)) {
        message = "New Doctor ID already exists.";
        return false;
    }

    Doctor doctor;
    if (!storage_.readDoctorAt(doctorOffset, doctor)) {
        message = "Could not read Doctor record.";
        return false;
    }

    const std::vector<std::string> appointmentIds =
        indexes_.appointmentIdsForDoctor(oldId);

    copyField(doctor.id, 15, newId);
    if (!storage_.writeDoctorAt(doctorOffset, doctor)) {
        message = "Could not update Doctor ID in the data file.";
        return false;
    }

    if (!indexes_.changeDoctorPrimary(oldId, newId, doctorOffset)) {
        message = "Doctor ID was written, but primary-index update failed. "
                  "Rebuild indexes from the data files.";
        return false;
    }

    Doctor oldDoctor = doctor;
    copyField(oldDoctor.id, 15, oldId);
    indexes_.eraseDoctorSecondary(oldDoctor);
    if (!indexes_.insertDoctorSecondary(doctor)) {
        message = "Doctor ID changed, but Doctor secondary-index update failed.";
        return false;
    }

    for (std::size_t i = 0; i < appointmentIds.size(); ++i) {
        const int appointmentOffset =
            indexes_.findAppointmentOffset(appointmentIds[i]);
        if (appointmentOffset < 0) {
            message = "Doctor ID changed, but a linked Appointment was not found.";
            return false;
        }

        Appointment appointment;
        if (!storage_.readAppointmentAt(appointmentOffset, appointment)) {
            message = "Doctor ID changed, but a linked Appointment could not be read.";
            return false;
        }

        copyField(appointment.doctorId, 15, newId);
        if (!storage_.writeAppointmentAt(appointmentOffset, appointment) ||
            !indexes_.updateAppointmentDoctorSecondary(
                appointmentIds[i], oldId, newId)) {
            message = "Doctor ID changed, but an Appointment reference/index update failed.";
            return false;
        }
    }

    message = "Doctor ID updated successfully. Linked appointments were updated too.";
    return true;
}

bool CRUDOperations::updateAppointmentDate(
    const std::string& appointmentId, const std::string& newDate,
    std::string& message) {
    if (!validateField(newDate, 30, "Appointment Date", message))
        return false;

    const int offset = indexes_.findAppointmentOffset(appointmentId);
    if (offset < 0) {
        message = "Appointment ID was not found.";
        return false;
    }

    Appointment appointment;
    if (!storage_.readAppointmentAt(offset, appointment)) {
        message = "Could not read Appointment record.";
        return false;
    }

    const std::string oldDate = fieldToString(appointment.date, 30);
    if (oldDate == newDate) {
        message = "The new date is the same as the current date.";
        return false;
    }

    copyField(appointment.date, 30, newDate);
    if (!storage_.writeAppointmentAt(offset, appointment)) {
        message = "Could not update Appointment date.";
        return false;
    }

    message = "Appointment date updated successfully.";
    return true;
}

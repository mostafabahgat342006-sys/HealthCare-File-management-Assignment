#pragma once

#include <string>
#include <vector>
#include "Doctor.h"
#include "Appointment.h"
#include "CRUD.h"
#include "PrimaryIndex.h"
#include "SecondaryIndex.h"

struct QueryCondition {
    std::string field;
    std::string value;
};

struct ParsedQuery {
    bool success;
    std::string error;
    std::string selectField;
    std::string tableName;
    std::vector<QueryCondition> conditions;
};

class QueryEngine {
private:
    IFileStorage& storage_;
    PrimaryIndex& doctorPrimary_;
    PrimaryIndex& appointmentPrimary_;
    SecondaryIndex& doctorNameSecondary_;
    SecondaryIndex& doctorSpecialtySecondary_;
    SecondaryIndex& appointmentDoctorSecondary_;

    ParsedQuery parseQuery(const std::string& queryStr);
    bool executeDoctorsQuery(const ParsedQuery& pq);
    bool executeAppointmentsQuery(const ParsedQuery& pq);

public:
    QueryEngine(IFileStorage& storage,
                PrimaryIndex& doctorPrimary,
                PrimaryIndex& appointmentPrimary,
                SecondaryIndex& doctorNameSecondary,
                SecondaryIndex& doctorSpecialtySecondary,
                SecondaryIndex& appointmentDoctorSecondary);

    bool executeQuery(const std::string& queryString);

    static void printDoctor(const Doctor& doctor);
    static void printAppointment(const Appointment& appointment);
};

#pragma once
struct Appointment {
    char id[15] = {};
    char date[30] = {};
    char doctorId[15] = {};
};
static_assert(sizeof(Appointment) == 60);

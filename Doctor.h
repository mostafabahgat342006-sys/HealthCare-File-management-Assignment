#pragma once
struct Doctor {
    char id[15] = {};
    char name[30] = {};
    char specialty[20] = {};
    char address[30] = {};
};
static_assert(sizeof(Doctor) == 95);

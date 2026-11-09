#pragma once
#include <string>
#include <vector>
using namespace std;

struct ListNode {
    char primaryKey[15];
    int  next;
};
static_assert(sizeof(ListNode) == 20, "ListNode size changed");

struct SecondaryEntry {
    string key;
    int    head;
};

class SecondaryIndex {
public:
    vector<SecondaryEntry> entries;

    explicit SecondaryIndex(string listFilename);

    void reset();
    vector<string> search(string key);
    bool insert(string key, string primaryKey);
    bool remove(string key, string primaryKey);
    bool update(string oldKey, string newKey, string primaryKey);
    bool replacePrimaryKey(string key, string oldPk, string newPk);
    void print();

private:
    string listFile;

    int  lowerBound(const string& key) const;
    bool contains(const string& key, const string& pk);
    bool readNode(int offset, ListNode& node);
    void writeNode(int offset, const ListNode& node);
    int  allocNode(const ListNode& node);
    void freeNode(int offset);
};

void rebuildDoctorNameIndex(SecondaryIndex& index, string doctorFile);
void rebuildDoctorSpecialtyIndex(SecondaryIndex& index, string doctorFile);
void rebuildAppointmentDoctorIndex(SecondaryIndex& index, string appointmentFile);

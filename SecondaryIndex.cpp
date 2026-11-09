#include "SecondaryIndex.h"
#include "Doctor.h"
#include "Appointment.h"
#include <fstream>
#include <cstring>
#include <iostream>
using namespace std;

static ListNode makeNode(const string& pk, int next)
{
    ListNode n;
    memset(&n, 0, sizeof(ListNode));
    strncpy(n.primaryKey, pk.c_str(), sizeof(n.primaryKey) - 1);
    n.next = next;
    return n;
}

SecondaryIndex::SecondaryIndex(string listFilename) : listFile(listFilename)
{
    reset();
}

void SecondaryIndex::reset()
{
    entries.clear();
    ofstream file(listFile, ios::out | ios::binary | ios::trunc);
    int head = -1;
    file.write((char*)&head, sizeof(int));
}

int SecondaryIndex::lowerBound(const string& key) const
{
    int lo = 0, hi = (int)entries.size();
    while (lo < hi)
    {
        int mid = lo + (hi - lo) / 2;
        if (entries[mid].key < key) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

bool SecondaryIndex::readNode(int offset, ListNode& node)
{
    ifstream file(listFile, ios::binary);
    file.seekg(offset);
    file.read((char*)&node, sizeof(ListNode));
    return (bool)file;
}

void SecondaryIndex::writeNode(int offset, const ListNode& node)
{
    fstream file(listFile, ios::in | ios::out | ios::binary);
    file.seekp(offset);
    file.write((char*)&node, sizeof(ListNode));
}

int SecondaryIndex::allocNode(const ListNode& node)
{
    fstream file(listFile, ios::in | ios::out | ios::binary);

    int head;
    file.seekg(0);
    file.read((char*)&head, sizeof(int));

    int offset;
    if (head == -1)
    {
        file.seekp(0, ios::end);
        offset = (int)file.tellp();
    }
    else
    {
        offset = head;
        ListNode freed;
        file.seekg(offset);
        file.read((char*)&freed, sizeof(ListNode));

        int nextFree = freed.next;
        file.seekp(0);
        file.write((char*)&nextFree, sizeof(int));
    }

    file.seekp(offset);
    file.write((char*)&node, sizeof(ListNode));
    return offset;
}

void SecondaryIndex::freeNode(int offset)
{
    fstream file(listFile, ios::in | ios::out | ios::binary);

    int head;
    file.seekg(0);
    file.read((char*)&head, sizeof(int));

    ListNode n = makeNode("*", head);
    file.seekp(offset);
    file.write((char*)&n, sizeof(ListNode));

    file.seekp(0);
    file.write((char*)&offset, sizeof(int));
}

vector<string> SecondaryIndex::search(string key)
{
    vector<string> result;

    int pos = lowerBound(key);
    if (pos >= (int)entries.size() || entries[pos].key != key)
        return result;

    int cur = entries[pos].head;
    ListNode node;
    while (cur != -1 && readNode(cur, node))
    {
        result.push_back(string(node.primaryKey));
        cur = node.next;
    }
    return result;
}

bool SecondaryIndex::contains(const string& key, const string& pk)
{
    vector<string> ids = search(key);
    for (size_t i = 0; i < ids.size(); i++)
        if (ids[i] == pk) return true;
    return false;
}

bool SecondaryIndex::insert(string key, string primaryKey)
{
    if (primaryKey.empty() || primaryKey.size() >= sizeof(ListNode::primaryKey))
        return false;

    int pos = lowerBound(key);
    bool exists = pos < (int)entries.size() && entries[pos].key == key;

    if (!exists)
    {
        SecondaryEntry e;
        e.key = key;
        e.head = allocNode(makeNode(primaryKey, -1));
        entries.insert(entries.begin() + pos, e);
        return true;
    }

    int prev = -1, cur = entries[pos].head;
    ListNode node;
    while (cur != -1 && readNode(cur, node))
    {
        string curPk = node.primaryKey;
        if (curPk == primaryKey) return false;
        if (curPk > primaryKey) break;
        prev = cur;
        cur = node.next;
    }

    int off = allocNode(makeNode(primaryKey, cur));
    if (prev == -1)
        entries[pos].head = off;
    else
    {
        readNode(prev, node);
        node.next = off;
        writeNode(prev, node);
    }
    return true;
}

bool SecondaryIndex::remove(string key, string primaryKey)
{
    int pos = lowerBound(key);
    if (pos >= (int)entries.size() || entries[pos].key != key)
        return false;

    int prev = -1, cur = entries[pos].head;
    ListNode node;
    while (cur != -1 && readNode(cur, node))
    {
        if (string(node.primaryKey) == primaryKey)
        {
            int next = node.next;
            if (prev == -1)
                entries[pos].head = next;
            else
            {
                ListNode p;
                readNode(prev, p);
                p.next = next;
                writeNode(prev, p);
            }
            freeNode(cur);

            if (entries[pos].head == -1)
                entries.erase(entries.begin() + pos);
            return true;
        }
        prev = cur;
        cur = node.next;
    }
    return false;
}

bool SecondaryIndex::update(string oldKey, string newKey, string primaryKey)
{
    if (!contains(oldKey, primaryKey)) return false;
    if (oldKey == newKey) return true;
    if (contains(newKey, primaryKey)) return false;

    remove(oldKey, primaryKey);
    return insert(newKey, primaryKey);
}

bool SecondaryIndex::replacePrimaryKey(string key, string oldPk, string newPk)
{
    if (!contains(key, oldPk)) return false;
    if (oldPk == newPk) return true;
    if (contains(key, newPk)) return false;

    remove(key, oldPk);
    return insert(key, newPk);
}

void SecondaryIndex::print()
{
    for (size_t i = 0; i < entries.size(); i++)
    {
        cout << entries[i].key << " -> ";
        vector<string> ids = search(entries[i].key);
        for (size_t j = 0; j < ids.size(); j++)
            cout << ids[j] << (j + 1 < ids.size() ? ", " : "");
        cout << '\n';
    }
}

static string fieldToString(const char* s, size_t n)
{
    const void* z = memchr(s, 0, n);
    return string(s, z ? (size_t)((const char*)z - s) : n);
}

template <typename Rec, typename KeyFn>
static void rebuildFrom(SecondaryIndex& index, const string& filename, KeyFn keyOf)
{
    index.reset();

    ifstream file(filename, ios::binary);
    if (!file)
        return;

    file.seekg(sizeof(int));

    Rec rec;
    while (file.read((char*)&rec, sizeof(Rec)))
    {
        if (rec.id[0] == '*')
            continue;
        index.insert(keyOf(rec), fieldToString(rec.id, sizeof(rec.id)));
    }
}

void rebuildDoctorNameIndex(SecondaryIndex& index, string doctorFile)
{
    rebuildFrom<Doctor>(index, doctorFile,
        [](const Doctor& d) { return fieldToString(d.name, sizeof(d.name)); });
}

void rebuildDoctorSpecialtyIndex(SecondaryIndex& index, string doctorFile)
{
    rebuildFrom<Doctor>(index, doctorFile,
        [](const Doctor& d) { return fieldToString(d.specialty, sizeof(d.specialty)); });
}

void rebuildAppointmentDoctorIndex(SecondaryIndex& index, string appointmentFile)
{
    rebuildFrom<Appointment>(index, appointmentFile,
        [](const Appointment& a) { return fieldToString(a.doctorId, sizeof(a.doctorId)); });
}

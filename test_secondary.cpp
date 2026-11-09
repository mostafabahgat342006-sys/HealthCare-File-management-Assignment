#include "SecondaryIndex.h"
#include <cassert>
#include <iostream>
using namespace std;

int main()
{
    SecondaryIndex byName("test_name.bin");

    byName.insert("Ahmed", "D3");
    byName.insert("Ahmed", "D1");
    byName.insert("Sara",  "D2");
    byName.insert("Ahmed", "D2");
    assert(!byName.insert("Ahmed", "D1"));

    vector<string> r = byName.search("Ahmed");
    assert(r.size() == 3 && r[0] == "D1" && r[1] == "D2" && r[2] == "D3");
    assert(byName.search("Nobody").empty());

    assert(byName.remove("Ahmed", "D2"));
    assert(byName.remove("Ahmed", "D1"));
    r = byName.search("Ahmed");
    assert(r.size() == 1 && r[0] == "D3");

    byName.insert("Ahmed", "D9");
    assert(byName.search("Ahmed").size() == 2);

    assert(byName.update("Sara", "Salma", "D2"));
    assert(byName.search("Sara").empty());
    assert(byName.search("Salma").size() == 1);

    assert(byName.replacePrimaryKey("Ahmed", "D3", "D4"));
    r = byName.search("Ahmed");
    assert(r.size() == 2 && r[0] == "D4" && r[1] == "D9");

    assert(byName.remove("Salma", "D2"));
    for (size_t i = 1; i < byName.entries.size(); i++)
        assert(byName.entries[i - 1].key < byName.entries[i].key);

    byName.print();
    cout << "All tests passed\n";
}

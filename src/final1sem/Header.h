#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Структура контакта с тегами
struct Contact {
    string name;
    string phone;
    string email;
    set<string> tags;
};

// Прототипы функций
string normalize(const string& s);
bool validName(const string& name);
bool validPhone(const string& phone);
bool validEmail(const string& email);

vector<Contact> readJSON(const string& filename);
void writeJSON(const string& filename, const vector<Contact>& contacts);

vector<Contact> findContactsByPrefix(const vector<Contact>& contacts, const string& prefix, long long& elapsedMs);
void sortContactsByName(vector<Contact>& contacts, long long& elapsedMs);
int deduplicateContacts(vector<Contact>& contacts, long long& elapsedMs);

void showContacts(const vector<Contact>& contacts);
void addContact(vector<Contact>& contacts);
void deleteContact(vector<Contact>& contacts, const string& name);

#endif
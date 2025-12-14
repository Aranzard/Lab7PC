#ifndef PHONEBOOK_H
#define PHONEBOOK_H

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
    vector<string> tags;
};

// Функции валидации
string normalize(const string& s);
bool validName(const string& name);
bool validPhone(const string& phone);
bool validEmail(const string& email);

// Функции работы с JSON
vector<Contact> readJSON(const string& filename);
string escapeJson(const string& s);
void writeJSON(const string& filename, const vector<Contact>& contacts);

// Функции работы с контактами
vector<Contact> findContactsByPrefix(const vector<Contact>& contacts,
    const string& prefix,
    long long& elapsedMs);
void sortContactsByName(vector<Contact>& contacts, long long& elapsedMs);
int deduplicateContacts(vector<Contact>& contacts, long long& elapsedMs);
void showContacts(const vector<Contact>& contacts, const string& title = "ВСЕ КОНТАКТЫ");
void addContact(vector<Contact>& contacts);
void deleteContact(vector<Contact>& contacts, const string& name);

#endif // PHONEBOOK_H
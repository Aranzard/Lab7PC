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
#include <set>
using namespace std;
using namespace std::chrono;

// Структура контакта
struct Contact {
    string name;
    string phone;
    string email;
    set<string> tags;
};

// Прототипы функций
string normalize(const string& s); // Приводит строку к нижнему регистру
bool validName(const string& name); // Проверяет, что имя не пустое
bool validPhone(const string& phone); // Проверяет формат телефона: 1(123)456-78-90
bool validEmail(const string& email); // Проверяет формат email: n@n.n

vector<Contact> readJSON(const string& filename); // Считывает контакты из JSON файла в вектор. Проверяет валидность перед добавлением
void writeJSON(const string& filename, const vector<Contact>& contacts); // Сохраняет контакты в указанный JSON файл

vector<Contact> findContactsByPrefix(const vector<Contact>& contacts, const string& prefix, long long& elapsedMs); // Ищет контакты по началу имени
void sortContactsByName(vector<Contact>& contacts, long long& elapsedMs); // Сортирует контакты по имени (регистронезависимо)

string normalizeAndSortName(const string& name); // Разбивает имя на слова, сортирует слова и собирает обратно
int deduplicateContacts(vector<Contact>& contacts, long long& elapsedMs); // Удаляет одинаковые имена после нормализации

void showContacts(const vector<Contact>& contacts); // Отображает все контакты
void addContact(vector<Contact>& contacts); // добавляет новый контакт, проверяя валидность
void deleteContact(vector<Contact>& contacts, const string& name); // Удаляет контакт по точному имени

#endif
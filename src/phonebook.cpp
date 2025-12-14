#include "phonebook.h"

// Функция для нормализации строки (нижний регистр)
string normalize(const string& s) {
    string result = s;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Проверка валидности имени
bool validName(const string& name) {
    return !name.empty();
}

// Проверка валидности телефона
bool validPhone(const string& phone) {
    if (phone.length() < 7) return false;

    int digitCount = 0;
    for (char c : phone) {
        if (isdigit(c)) digitCount++;
        else if (c != '+' && c != '-' && c != '(' && c != ')' && c != ' ')
            return false;
    }
    return digitCount >= 7;
}

// Проверка валидности email
bool validEmail(const string& email) {
    size_t at = email.find('@');
    if (at == string::npos || at == 0) return false;

    size_t dot = email.find('.', at);
    if (dot == string::npos || dot == at + 1 || dot == email.length() - 1)
        return false;

    // Проверка на пробелы
    if (email.find(' ') != string::npos) return false;

    return true;
}

// Функция для чтения JSON файла
vector<Contact> readJSON(const string& filename) {
    auto start = high_resolution_clock::now();
    vector<Contact> contacts;
    ifstream file(filename);

    if (!file.is_open()) {
        auto end = high_resolution_clock::now();
        auto elapsed = duration_cast<milliseconds>(end - start).count();
        cout << "Файл не найден. Будет создан новый. (" << elapsed << " мс)\n";
        return contacts;
    }

    // Читаем весь файл
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // Удаляем пробелы и переносы строк для упрощения парсинга
    string json;
    for (char c : content) {
        if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
            json += c;
        }
    }

    // Проверяем, что это JSON массив
    if (json.empty() || json[0] != '[' || json[json.size() - 1] != ']') {
        auto end = high_resolution_clock::now();
        auto elapsed = duration_cast<milliseconds>(end - start).count();
        cout << "Ошибка: Неверный формат JSON (" << elapsed << " мс)\n";
        return contacts;
    }

    // Удаляем внешние скобки
    json = json.substr(1, json.size() - 2);

    size_t pos = 0;
    while (pos < json.size()) {
        // Ищем начало объекта
        if (json[pos] != '{') {
            pos++;
            continue;
        }

        // Ищем конец объекта
        size_t end = json.find('}', pos);
        if (end == string::npos) break;

        string obj = json.substr(pos + 1, end - pos - 1);
        Contact c;

        // Извлекаем поля
        size_t fieldStart = 0;
        while (fieldStart < obj.size()) {
            // Ищем ключ
            size_t keyStart = obj.find('"', fieldStart);
            if (keyStart == string::npos) break;
            size_t keyEnd = obj.find('"', keyStart + 1);
            if (keyEnd == string::npos) break;

            string key = obj.substr(keyStart + 1, keyEnd - keyStart - 1);

            // Ищем двоеточие после ключа
            size_t colonPos = obj.find(':', keyEnd + 1);
            if (colonPos == string::npos) break;

            // Ищем значение
            size_t valueStart = colonPos + 1;
            string value;

            if (obj[valueStart] == '"') {
                // Строковое значение
                size_t valueEnd = obj.find('"', valueStart + 1);
                if (valueEnd == string::npos) break;
                value = obj.substr(valueStart + 1, valueEnd - valueStart - 1);
                fieldStart = valueEnd + 1;
            }
            else if (obj[valueStart] == '[') {
                // Массив (теги)
                size_t arrayEnd = obj.find(']', valueStart);
                if (arrayEnd == string::npos) break;

                string arrayStr = obj.substr(valueStart + 1, arrayEnd - valueStart - 1);
                size_t tagPos = 0;

                while (tagPos < arrayStr.size()) {
                    size_t tagStart = arrayStr.find('"', tagPos);
                    if (tagStart == string::npos) break;
                    size_t tagEnd = arrayStr.find('"', tagStart + 1);
                    if (tagEnd == string::npos) break;

                    string tag = arrayStr.substr(tagStart + 1, tagEnd - tagStart - 1);
                    if (!tag.empty()) c.tags.push_back(tag);

                    tagPos = tagEnd + 1;
                }

                fieldStart = arrayEnd + 1;
                continue;
            }
            else {
                // Другие типы значений не поддерживаются
                fieldStart = valueStart + 1;
                continue;
            }

            // Сохраняем значение в соответствующее поле
            if (key == "name") c.name = value;
            else if (key == "phone") c.phone = value;
            else if (key == "email") c.email = value;

            // Ищем запятую или конец объекта
            fieldStart = obj.find(',', fieldStart);
            if (fieldStart == string::npos) break;
            fieldStart++;
        }

        // Проверяем валидность и добавляем контакт
        if (validName(c.name) && validPhone(c.phone) && validEmail(c.email)) {
            contacts.push_back(c);
        }

        pos = end + 1;
    }

    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end - start).count();
    cout << "Загружено контактов: " << contacts.size() << " (" << elapsed << " мс)\n";

    return contacts;
}

// Функция для экранирования JSON строк
string escapeJson(const string& s) {
    string result;
    for (char c : s) {
        switch (c) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c; break;
        }
    }
    return result;
}

// Функция для записи JSON файла
void writeJSON(const string& filename, const vector<Contact>& contacts) {
    auto start = high_resolution_clock::now();

    ofstream file(filename);

    file << "[\n";
    for (size_t i = 0; i < contacts.size(); ++i) {
        file << "  {\n";
        file << "    \"name\": \"" << escapeJson(contacts[i].name) << "\",\n";
        file << "    \"phone\": \"" << escapeJson(contacts[i].phone) << "\",\n";
        file << "    \"email\": \"" << escapeJson(contacts[i].email) << "\",\n";
        file << "    \"tags\": [";

        for (size_t j = 0; j < contacts[i].tags.size(); ++j) {
            file << "\"" << escapeJson(contacts[i].tags[j]) << "\"";
            if (j != contacts[i].tags.size() - 1) file << ", ";
        }

        file << "]\n  }";
        if (i != contacts.size() - 1) file << ",";
        file << "\n";
    }
    file << "]\n";

    file.close();

    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end - start).count();
    cout << "Данные сохранены в " << filename << " (" << elapsed << " мс)\n";
}

// Поиск по префиксу с замером времени
vector<Contact> findContactsByPrefix(const vector<Contact>& contacts,
    const string& prefix,
    long long& elapsedMs) {
    auto start = high_resolution_clock::now();
    vector<Contact> result;

    string normalizedPrefix = normalize(prefix);

    for (const auto& contact : contacts) {
        string normalizedName = normalize(contact.name);

        if (normalizedName.length() >= normalizedPrefix.length()) {
            if (normalizedName.find(normalizedPrefix) == 0) {
                result.push_back(contact);
            }
        }
    }

    auto end = high_resolution_clock::now();
    elapsedMs = duration_cast<milliseconds>(end - start).count();

    return result;
}

// Сортировка по имени
void sortContactsByName(vector<Contact>& contacts, long long& elapsedMs) {
    auto start = high_resolution_clock::now();

    sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
        string nameA = normalize(a.name);
        string nameB = normalize(b.name);
        return nameA < nameB;
        });

    auto end = high_resolution_clock::now();
    elapsedMs = duration_cast<milliseconds>(end - start).count();
}

// Дедупликация похожих имен
int deduplicateContacts(vector<Contact>& contacts, long long& elapsedMs) {
    auto start = high_resolution_clock::now();
    int removed = 0;

    for (size_t i = 0; i < contacts.size(); ++i) {
        for (size_t j = i + 1; j < contacts.size(); ++j) {
            string name1 = normalize(contacts[i].name);
            string name2 = normalize(contacts[j].name);

            // Разбиваем на слова
            stringstream ss1(name1), ss2(name2);
            vector<string> words1, words2;
            string word;

            while (ss1 >> word) words1.push_back(word);
            while (ss2 >> word) words2.push_back(word);

            // Проверяем одинаковое количество слов
            if (words1.size() != words2.size()) continue;

            // Сортируем слова и сравниваем
            sort(words1.begin(), words1.end());
            sort(words2.begin(), words2.end());

            if (words1 == words2) {
                contacts.erase(contacts.begin() + j);
                --j;
                ++removed;
            }
        }
    }

    auto end = high_resolution_clock::now();
    elapsedMs = duration_cast<milliseconds>(end - start).count();

    return removed;
}

// Показ контактов
void showContacts(const vector<Contact>& contacts, const string& title) {
    auto start = high_resolution_clock::now();

    if (contacts.empty()) {
        auto end = high_resolution_clock::now();
        auto elapsed = duration_cast<milliseconds>(end - start).count();
        cout << "Контакты не найдены. (" << elapsed << " мс)\n";
        return;
    }

    cout << "\n=== " << title << " (" << contacts.size() << ") ===\n";
    for (const auto& contact : contacts) {
        cout << "\nИмя: " << contact.name;
        cout << "\nТелефон: " << contact.phone;
        cout << "\nEmail: " << contact.email;
        if (!contact.tags.empty()) {
            cout << "\nТеги: ";
            for (const auto& tag : contact.tags) {
                cout << tag << " ";
            }
        }
        cout << "\n" << string(40, '-');
    }

    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end - start).count();
    cout << "\n\nВремя отображения: " << elapsed << " мс\n";
}

// Добавление нового контактa
void addContact(vector<Contact>& contacts) {
    auto start = high_resolution_clock::now();
    Contact c;

    // Ввод имени
    do {
        cout << "Введите имя: ";
        getline(cin, c.name);
        if (!validName(c.name)) {
            cout << "Ошибка: Имя не может быть пустым\n";
        }
    } while (!validName(c.name));

    // Ввод телефона
    do {
        cout << "Введите телефон: ";
        getline(cin, c.phone);
        if (!validPhone(c.phone)) {
            cout << "Ошибка: Некорректный формат телефона\n";
        }
    } while (!validPhone(c.phone));

    // Ввод email
    do {
        cout << "Введите email: ";
        getline(cin, c.email);
        if (!validEmail(c.email)) {
            cout << "Ошибка: Некорректный формат email\n";
        }
    } while (!validEmail(c.email));

    // Ввод тегов
    cout << "Введите теги (через запятую, оставьте пустым для пропуска): ";
    string tagsInput;
    getline(cin, tagsInput);

    if (!tagsInput.empty()) {
        stringstream ss(tagsInput);
        string tag;
        while (getline(ss, tag, ',')) {
            // Удаляем пробелы в начале и конце
            tag.erase(0, tag.find_first_not_of(' '));
            tag.erase(tag.find_last_not_of(' ') + 1);
            if (!tag.empty()) {
                c.tags.push_back(tag);
            }
        }
    }

    contacts.push_back(c);

    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end - start).count();
    cout << "Контакт добавлен. (" << elapsed << " мс)\n";
}

// Удаление контакта по имени
void deleteContact(vector<Contact>& contacts, const string& name) {
    auto start = high_resolution_clock::now();

    bool found = false;
    for (size_t i = 0; i < contacts.size(); ++i) {
        if (contacts[i].name == name) {
            contacts.erase(contacts.begin() + i);
            found = true;
            break;
        }
    }

    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end - start).count();

    cout << (found ? "Контакт удален" : "Контакт не найден")
        << ". (" << elapsed << " мс)\n";
}
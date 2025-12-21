#include "Header.h"

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
    if (phone.length() != 15) return false;

    // Проверяем фиксированные символы
    if (phone[1] != '(' || phone[5] != ')' || phone[9] != '-' || phone[12] != '-') {
        return false;
    }

    // Проверяем цифры на позициях
    int digitPositions[] = { 0, 2, 3, 4, 6, 7, 8, 10, 11, 13, 14 };
    for (int pos : digitPositions) {
        if (!isdigit(phone[pos])) return false;
    }

    return true;
}

// Проверка валидности email
bool validEmail(const string& email) {
    if (email.empty()) return false;

    // Проверка на пробелы
    if (email.find(' ') != string::npos) {
        return false;
    }

    // Количество @
    int atCount = 0;
    for (char c : email) {
        if (c == '@') atCount++;
    }

    // Должен быть ровно один @
    if (atCount != 1) {
        return false;
    }

    size_t at = email.find('@');

    // @ не может быть в начале или конце
    if (at == 0 || at == email.length() - 1) {
        return false;
    }

    // Должна быть хотя бы одна точка после @
    size_t dot = email.find('.', at);
    if (dot == string::npos || dot == at + 1 || dot == email.length() - 1) {
        return false;
    }

    // Проверяем, что нет двух точек подряд
    for (size_t i = at + 1; i < email.length() - 1; ++i) {
        if (email[i] == '.' && email[i + 1] == '.') {
            return false;
        }
    }

    return true;
}
// Функция для чтения JSON файла
vector<Contact> readJSON(const string& filename) {
    auto start = high_resolution_clock::now();
    vector<Contact> contacts;
    ifstream file(filename);

    if (!file) {
        auto elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
        cout << "Файл не найден (" << elapsed << " мс)\n";
        return contacts;
    }

    // Читаем файл построчно для парсинга
    string line;
    Contact current;
    bool inContact = false;
    bool inTags = false;
    string tagsContent;

    while (getline(file, line)) {
        // Удаляем лишние пробелы
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line == "[") {
            continue; // Начало массива
        }

        if (line == "]" || line == "],") {
            break; // Конец массива
        }

        if (line == "{") {
            current = Contact(); // Начинаем новый контакт
            inContact = true;
            continue;
        }

        if (line == "}" || line == "},") {
            if (inContact && validName(current.name) && validPhone(current.phone) && validEmail(current.email)) {
                contacts.push_back(current);
            }
            inContact = false;
            continue;
        }

        if (!inContact) {
            continue;
        }

        // Парсим поля контакта
        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string key = line.substr(0, colonPos);
            string value = line.substr(colonPos + 1);

            // Очищаем ключ и значение от кавычек и пробелов
            key.erase(0, key.find_first_not_of(" \t\""));
            key.erase(key.find_last_not_of(" \t\",") + 1);

            value.erase(0, value.find_first_not_of(" \t\""));
            value.erase(value.find_last_not_of(" \t\",") + 1);

            if (key == "name") {
                current.name = value;
            }
            else if (key == "phone") {
                current.phone = value;
            }
            else if (key == "email") {
                current.email = value;
            }
            else if (key == "tags" && value[0] == '[') {
                // Парсим теги
                string tagsStr = value.substr(1, value.length() - 2); // Убираем [ и ]
                size_t pos = 0;

                while (pos < tagsStr.length()) {
                    size_t start = tagsStr.find('"', pos);
                    if (start == string::npos) break;

                    size_t end = tagsStr.find('"', start + 1);
                    if (end == string::npos) break;

                    string tag = tagsStr.substr(start + 1, end - start - 1);
                    if (!tag.empty()) {
                        current.tags.insert(tag);
                    }

                    pos = end + 1;
                }
            }
        }
    }

    file.close();

    auto elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    cout << "Загружено: " << contacts.size() << " контактов (" << elapsed << " мс)\n";

    return contacts;
}

// Функция для записи JSON файла
void writeJSON(const string& filename, const vector<Contact>& contacts) {
    auto start = high_resolution_clock::now();

    ofstream file(filename);

    file << "[\n";
    for (int i = 0; i < contacts.size(); ++i) {
        file << "  {\n";
        file << "    \"name\": \"" << contacts[i].name << "\",\n";
        file << "    \"phone\": \"" << contacts[i].phone << "\",\n";
        file << "    \"email\": \"" << contacts[i].email << "\",\n";
        file << "    \"tags\": [";

        int j = 0;
        for ( auto& tag : contacts[i].tags) {
            if (j > 0) file << ", ";
            file << "\"" << tag << "\"";
            j++;
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

// Поиск по префиксу
vector<Contact> findContactsByPrefix(const vector<Contact>& contacts,
    const string& prefix,
    long long& elapsedMs) {
    auto start = high_resolution_clock::now();
    vector<Contact> result;

    string normalizedPrefix = normalize(prefix);

    for ( auto& contact : contacts) {
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

    sort(contacts.begin(), contacts.end(), []( Contact& a,  Contact& b) {
        string nameA = normalize(a.name);
        string nameB = normalize(b.name);
        return nameA < nameB;
        });

    auto end = high_resolution_clock::now();
    elapsedMs = duration_cast<milliseconds>(end - start).count();
}

// Дедупликация похожих имен
/*int deduplicateContacts(vector<Contact>& contacts, long long& elapsedMs) {
    auto start = high_resolution_clock::now();
    int removed = 0;

    for (int i = 0; i < contacts.size(); ++i) {
        for (int j = i + 1; j < contacts.size(); ++j) {
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
    */
string normalizeAndSortName(const string& name) {
    stringstream ss(name);
    vector<string> words;
    string word;

    while (ss >> word) {
        words.push_back(word);
    }

    sort(words.begin(), words.end());

    string result;
    for (const auto& w : words) {
        if (!result.empty()) result += " ";
        result += w;
    }

    return result;
}

int deduplicateContacts(vector<Contact>& contacts, long long& elapsedMs) {
    auto start = high_resolution_clock::now();
    int removed = 0;

    // Используем set для хранения нормализованных имен
    set<string> seenNames;  // Изменено с unordered_set
    vector<Contact> uniqueContacts;

    // Резервируем память для уменьшения реаллокаций
    uniqueContacts.reserve(contacts.size());

    for (const auto& contact : contacts) {
        // Нормализуем и сортируем имя
        string normalizedName = normalizeAndSortName(contact.name);

        // Проверяем, видели ли мы уже такое имя
        if (seenNames.find(normalizedName) == seenNames.end()) {
            // Не видели - добавляем контакт и имя в set
            seenNames.insert(normalizedName);
            uniqueContacts.push_back(contact);
        }
        else {
            // Видели - увеличиваем счетчик удаленных
            ++removed;
        }
    }

    // Заменяем исходный вектор уникальными контактами
    contacts = uniqueContacts;

    auto end = high_resolution_clock::now();
    elapsedMs = duration_cast<milliseconds>(end - start).count();

    return removed;
}

// Показ контактов
void showContacts(const vector<Contact>& contacts) {
    auto start = high_resolution_clock::now();

    if (contacts.empty()) {
        auto end = high_resolution_clock::now();
        auto elapsed = duration_cast<milliseconds>(end - start).count();
        cout << "Контакты не найдены. (" << elapsed << " мс)\n";
        return;
    }

    cout << "\n=== " << "ВСЕ КОНТАКТЫ" << " (" << contacts.size() << ") ===\n";
    for ( auto& contact : contacts) {
        cout << "\nИмя: " << contact.name;
        cout << "\nТелефон: " << contact.phone;
        cout << "\nEmail: " << contact.email;
        if (!contact.tags.empty()) {
            cout << "\nТеги: ";
            for ( auto& tag : contact.tags) {
                cout << tag << " ";
            }
        }
        cout << "\n" << string(40, '-');
    }

    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end - start).count();
    cout << "\n\nВремя отображения: " << elapsed << " мс\n";
}

// Добавление нового контакта
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
        cout << "Введите телефон (формат: n(nnn)nnn-nn-nn на месте n должны быть любые цифры): ";
        getline(cin, c.phone);
        if (!validPhone(c.phone)) {
            cout << "Ошибка: Телефон должен быть в формате 1(123)456-78-90\n";
            cout << "Пример: 1(916)555-44-33\n";
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
            // Удаление пробелов в начале и конце
            tag.erase(0, tag.find_first_not_of(' '));
            tag.erase(tag.find_last_not_of(' ') + 1);
            if (!tag.empty()) {
                c.tags.insert(tag);
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
    for (int i = 0; i < contacts.size(); ++i) {
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
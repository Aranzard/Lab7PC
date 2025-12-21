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
    int at = email.find('@');
    if (at == string::npos || at == 0) return false;

    int dot = email.find('.', at);
    if (dot == string::npos || dot == at + 1 || dot == email.length() - 1) return false;

    return email.find(' ') == string::npos;
}

// Функция для чтения JSON файла
vector<Contact> readJSON(const string& filename) {
    auto start = high_resolution_clock::now();
    vector<Contact> contacts;
    ifstream file(filename);
    
    if (!file) {
        cout << "Файл не найден (" 
             << duration_cast<milliseconds>(high_resolution_clock::now() - start).count() 
             << " мс)\n";
        return contacts;
    }
    
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    // Быстрый парсинг по ключевым словам
    int pos = 0;
    Contact c;
    string* currentField = nullptr;
    
    while (pos < content.length()) {
        if (content.find("\"name\"", pos) == pos) {
            pos += 7;
            int start = content.find('"', pos);
            int end = content.find('"', start + 1);
            if (start != string::npos && end != string::npos) {
                c.name = content.substr(start + 1, end - start - 1);
            }
            pos = end + 1;
        }
        else if (content.find("\"phone\"", pos) == pos) {
            pos += 8;
            int start = content.find('"', pos);
            int end = content.find('"', start + 1);
            if (start != string::npos && end != string::npos) {
                c.phone = content.substr(start + 1, end - start - 1);
            }
            pos = end + 1;
        }
        else if (content.find("\"email\"", pos) == pos) {
            pos += 8;
            int start = content.find('"', pos);
            int end = content.find('"', start + 1);
            if (start != string::npos && end != string::npos) {
                c.email = content.substr(start + 1, end - start - 1);
            }
            pos = end + 1;
        }
        else if (content.find("\"tags\"", pos) == pos) {
            pos += 6;
            int start = content.find('[', pos);
            int end = content.find(']', start + 1);
            if (start != string::npos && end != string::npos) {
                string tags = content.substr(start + 1, end - start - 1);
                int tagPos = 0;
                while ((tagPos = tags.find('"', tagPos)) != string::npos) {
                    int tagEnd = tags.find('"', tagPos + 1);
                    if (tagEnd != string::npos) {
                        c.tags.insert(tags.substr(tagPos + 1, tagEnd - tagPos - 1));
                        tagPos = tagEnd + 1;
                    }
                }
            }
            pos = end + 1;
        }
        else if (content.find('}', pos) == pos && !c.name.empty()) {
            if (validName(c.name) && validPhone(c.phone) && validEmail(c.email)) {
                contacts.push_back(c);
            }
            c = Contact();
            pos++;
        }
        else {
            pos++;
        }
    }
    
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
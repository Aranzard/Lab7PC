#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

struct Contact {
    string name, phone, email;
    set<string> tags;

    bool valid() const {
        if (name.empty()) return false;

        // Проверка телефона
        for (char c : phone)
            if (!isdigit(c) && strchr("+-() ", c) == nullptr)
                return false;
        if (phone.length() < 7) return false;

        // Проверка email
        size_t at = email.find('@');
        size_t dot = email.find('.', at);
        return at != string::npos && dot != string::npos && dot > at;
    }
};

// Функция для получения пути к файлу в папке data
string getDataFilePath(const string& filename) {
    return "../data/" + filename;  // Поднимаемся на уровень вверх из src, затем в data
}

vector<Contact> readJSON(const string& filename) {
    string filepath = getDataFilePath(filename);
    vector<Contact> contacts;
    ifstream in(filepath);

    if (!in.is_open()) {
        cout << "Файл не найден. Будет создан новый в папке data.\n";
        return contacts;
    }

    string line;
    while (getline(in, line)) {
        size_t start = line.find('{');
        size_t end = line.find('}');
        if (start == string::npos || end == string::npos) continue;

        string obj = line.substr(start + 1, end - start - 1);
        Contact c;

        // Извлечение данных
        auto extract = [&](const string& key) {
            size_t p1 = obj.find(key);
            if (p1 == string::npos) return string();
            p1 += key.length();
            size_t p2 = obj.find("\"", p1);
            return obj.substr(p1, p2 - p1);
            };

        c.name = extract("\"name\":\"");
        c.phone = extract("\"phone\":\"");
        c.email = extract("\"email\":\"");

        // Теги
        size_t t1 = obj.find("\"tags\":[");
        if (t1 != string::npos) {
            t1 += 8;
            size_t t2 = obj.find("]", t1);
            string tags = obj.substr(t1, t2 - t1);

            size_t pos = 0;
            while ((pos = tags.find('"', pos)) != string::npos) {
                size_t endq = tags.find('"', pos + 1);
                string tag = tags.substr(pos + 1, endq - pos - 1);
                if (!tag.empty()) c.tags.insert(tag);
                pos = endq + 1;
            }
        }

        if (c.valid()) contacts.push_back(c);
    }

    return contacts;
}

void addContact(vector<Contact>& contacts) {
    Contact c;

    cout << "Имя: "; getline(cin, c.name);
    cout << "Телефон: "; getline(cin, c.phone);
    cout << "Email: "; getline(cin, c.email);
    cout << "Теги (через запятую): ";

    string tags;
    getline(cin, tags);

    size_t pos = 0;
    while ((pos = tags.find(',')) != string::npos) {
        string tag = tags.substr(0, pos);
        tag.erase(0, tag.find_first_not_of(' '));
        tag.erase(tag.find_last_not_of(' ') + 1);
        if (!tag.empty()) c.tags.insert(tag);
        tags.erase(0, pos + 1);
    }

    if (!tags.empty()) {
        tags.erase(0, tags.find_first_not_of(' '));
        tags.erase(tags.find_last_not_of(' ') + 1);
        if (!tags.empty()) c.tags.insert(tags);
    }

    contacts.push_back(c);
    cout << "Добавлено.\n";
}

vector<Contact> findPrefix(const vector<Contact>& contacts, const string& prefix) {
    vector<Contact> result;
    string lowerPrefix = prefix;
    transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);

    for (const auto& c : contacts) {
        string name = c.name;
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(lowerPrefix) == 0) result.push_back(c);
    }
    return result;
}

void sortByName(vector<Contact>& contacts) {
    sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
        string na = a.name, nb = b.name;
        transform(na.begin(), na.end(), na.begin(), ::tolower);
        transform(nb.begin(), nb.end(), nb.begin(), ::tolower);
        return na < nb;
        });
    cout << "Отсортировано по имени.\n";
}

void sortByPhone(vector<Contact>& contacts) {
    sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
        string phoneA, phoneB;
        for (char c : a.phone) if (isdigit(c)) phoneA += c;
        for (char c : b.phone) if (isdigit(c)) phoneB += c;
        return phoneA < phoneB;
        });
    cout << "Отсортировано по номеру телефона.\n";
}

void sortByEmail(vector<Contact>& contacts) {
    sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
        string emailA = a.email, emailB = b.email;
        transform(emailA.begin(), emailA.end(), emailA.begin(), ::tolower);
        transform(emailB.begin(), emailB.end(), emailB.begin(), ::tolower);
        return emailA < emailB;
        });
    cout << "Отсортировано по email.\n";
}

int deduplicate(vector<Contact>& contacts) {
    int removed = 0;

    for (size_t i = 0; i < contacts.size(); ++i) {
        for (size_t j = i + 1; j < contacts.size(); ++j) {
            string n1 = contacts[i].name, n2 = contacts[j].name;
            transform(n1.begin(), n1.end(), n1.begin(), ::tolower);
            transform(n2.begin(), n2.end(), n2.begin(), ::tolower);

            // Разбиваем на слова
            stringstream ss1(n1), ss2(n2);
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

    return removed;
}

string escape(const string& s) {
    string r;
    for (char c : s) {
        if (c == '"') r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else r += c;
    }
    return r;
}

string toJSON(const Contact& c) {
    string json = "{\"name\":\"" + escape(c.name) +
        "\",\"phone\":\"" + escape(c.phone) +
        "\",\"email\":\"" + escape(c.email) +
        "\",\"tags\":[";

    size_t i = 0;
    for (const auto& tag : c.tags) {
        if (i++ > 0) json += ",";
        json += "\"" + escape(tag) + "\"";
    }
    json += "]}";
    return json;
}

void saveJSON(const string& filename, const vector<Contact>& contacts) {
    string filepath = getDataFilePath(filename);
    ofstream out(filepath);

    // Проверяем, открылся ли файл
    if (!out.is_open()) {
        cout << "Ошибка: не удалось создать файл " << filepath << endl;
        cout << "Убедитесь, что существует папка data/\n";
        return;
    }

    out << "[\n";
    for (size_t i = 0; i < contacts.size(); ++i) {
        out << "  " << toJSON(contacts[i]);
        if (i != contacts.size() - 1) out << ",";
        out << "\n";
    }
    out << "]";
    cout << "Данные сохранены в " << filepath << "\n";
}

void show(const vector<Contact>& contacts) {
    if (contacts.empty()) {
        cout << "Пусто.\n";
        return;
    }

    for (const auto& c : contacts) {
        cout << "\nИмя: " << c.name;
        cout << "\nТел: " << c.phone;
        cout << "\nПочта: " << c.email;
        if (!c.tags.empty()) {
            cout << "\nТеги: ";
            for (const auto& tag : c.tags) cout << tag << " ";
        }
        cout << "\n" << string(30, '-');
    }
    cout << "\nВсего: " << contacts.size() << endl;
}

int main() {
    setlocale(LC_ALL, "");

    // Загружаем данные из data/phonebook.json
    vector<Contact> contacts = readJSON("phonebook.json");
    cout << "=== ТЕЛЕФОННЫЙ СПРАВОЧНИК ===\n";

    while (true) {
        cout << "\n1. Показать все\n";
        cout << "2. Добавить контакт\n";
        cout << "3. Удалить контакт\n";
        cout << "4. Поиск по префиксу имени\n";
        cout << "5. Сортировать по имени\n";
        cout << "6. Сортировать по номеру телефона\n";
        cout << "7. Сортировать по email\n";
        cout << "8. Дедупликация (похожие имена)\n";
        cout << "9. Сохранить и выйти\n";
        cout << "Выберите действие: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            show(contacts);
        }
        else if (choice == 2) {
            addContact(contacts);
        }
        else if (choice == 3) {
            cout << "Имя или телефон: ";
            string key;
            getline(cin, key);

            bool found = false;
            for (size_t i = 0; i < contacts.size(); ++i) {
                if (contacts[i].name == key || contacts[i].phone == key) {
                    contacts.erase(contacts.begin() + i);
                    found = true;
                    break;
                }
            }
            cout << (found ? "Удалено\n" : "Не найдено\n");
        }
        else if (choice == 4) {
            cout << "Префикс: ";
            string prefix;
            getline(cin, prefix);

            auto found = findPrefix(contacts, prefix);
            cout << "Найдено: " << found.size() << endl;
            show(found);
        }
        else if (choice == 5) {
            sortByName(contacts);
        }
        else if (choice == 6) {
            sortByPhone(contacts);
        }
        else if (choice == 7) {
            sortByEmail(contacts);
        }
        else if (choice == 8) {
            int r = deduplicate(contacts);
            cout << "Удалено дубликатов: " << r << endl;
        }
        else if (choice == 9) {
            saveJSON("phonebook.json", contacts);
            break;
        }
        else {
            cout << "Неверный выбор\n";
        }
    }

    return 0;
}
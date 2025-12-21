#include "Header.h"

// Главная функция
int main() {
    // Настройка консоли для русского языка
    setlocale(LC_ALL, "");

    string filename, input;
    cout << "Введите имя файла для загрузки (или нажмите Enter для temp.json): ";
    getline(cin, filename);

    // Если пользователь не ввел имя файла, используем temp.json
    if (filename.empty()) {
        filename = "temp.json";
    }

    // Проверяем наличие расширения .json
    if (filename.size() < 5 || filename.substr(filename.size() - 5) != ".json") {
        filename += ".json";
    }

    // Загружаем контакты из файла
    vector<Contact> contacts;

    // Пытаемся открыть указанный файл
    ifstream testFile(filename);
    if (testFile.is_open()) {
        testFile.close();
        contacts = readJSON(filename);
    }
    else {
        // Если указанный файл не найден, пробуем temp.json
        cout << "Файл " << filename << " не найден.\n";

        if (filename != "temp.json") {
            cout << "Пробую загрузить temp.json...\n";
            contacts = readJSON("temp.json");
        }

        // Если temp.json тоже не найден, создаем пустой список
        if (contacts.empty()) {
            cout << "Создаю новый телефонный справочник.\n";
            contacts = vector<Contact>(); // Пустой список
        }
    }

    cout << "\n=== ТЕЛЕФОННЫЙ СПРАВОЧНИК ===\n";
    cout << "Загружено контактов: " << contacts.size() << endl;
    if (!filename.empty()) {
        cout << "Текущий файл: " << filename << endl;
    }

    // Функция для вывода справки
    auto showHelp = []() {
        cout << "\n=== СПРАВКА ПО КОМАНДАМ ===\n";
        cout << "help   - Показать эту справку\n";
        cout << "1      - Показать все контакты\n";
        cout << "2      - Добавить новый контакт\n";
        cout << "3      - Удалить контакт по имени\n";
        cout << "4      - Найти контакты по префиксу имени\n";
        cout << "5      - Сортировать контакты по имени\n";
        cout << "6      - Удалить дубликаты контактов\n";
        cout << "7      - Сохранить и выйти\n";
        cout << "------------------------------\n";
        cout << "Команды можно вводить как цифрами, так и текстом.\n";
        };

    // Основной цикл программы
    while (true) {
        cout << "\nМЕНЮ:\n";
        cout << "1. Показать контакты\n";
        cout << "2. Добавить\n";
        cout << "3. Удалить\n";
        cout << "4. Найти\n";
        cout << "5. Сортировать\n";
        cout << "6. Дедупликация\n";
        cout << "7. Выход\n";
        cout << "help - Справка по командам\n";

        cout << "Ваш выбор: ";
        getline(cin, input);

        // Обработка help
        if (input == "help") {
            showHelp();
            continue;
        }

        // Проверяем, что вся строка состоит из цифр
        bool allDigits = !input.empty();
        for (char c : input) {
            if (!isdigit(static_cast<unsigned char>(c))) {
                allDigits = false;
                break;
            }
        }

        if (!allDigits) {
            cout << "Ошибка! Введите число 1-7 или 'help' для справки\n";
            continue;
        }

        int choice = stoi(input);

        if (choice == 1) {
            showContacts(contacts);
        }
        else if (choice == 2) {
            addContact(contacts);
        }
        else if (choice == 3) {
            cout << "Имя для удаления: ";
            string name;
            getline(cin, name);
            deleteContact(contacts, name);
        }
        else if (choice == 4) {
            cout << "Поиск по имени (можно часть имени): ";
            string prefix;
            getline(cin, prefix);

            long long time_ms = 0;
            auto found = findContactsByPrefix(contacts, prefix, time_ms);

            cout << "Найдено: " << found.size() << endl;
            cout << "Время: " << time_ms << " мс\n";
            showContacts(found);
        }
        else if (choice == 5) {
            long long time_ms = 0;
            sortContactsByName(contacts, time_ms);
            cout << "Отсортировано. Время: " << time_ms << " мс\n";
        }
        else if (choice == 6) {
            long long time_ms = 0;
            int removed = deduplicateContacts(contacts, time_ms);
            cout << "Удалено дублей: " << removed
                << " (Время: " << time_ms << " мс)\n";
        }
        else if (choice == 7) {
            // Предлагаем сохранить в тот же файл или новый
            cout << "\n=== СОХРАНЕНИЕ ===\n";
            cout << "Текущий файл: " << filename << endl;
            cout << "1. Сохранить в текущий файл\n";
            cout << "2. Сохранить в другой файл\n";
            cout << "3. Выйти без сохранения\n";
            cout << "Выберите вариант (1-3): ";

            string saveChoice;
            getline(cin, saveChoice);

            if (saveChoice == "1") {
                writeJSON(filename, contacts);
                cout << "Контакты сохранены в " << filename << endl;
            }
            else if (saveChoice == "2") {
                cout << "Введите имя файла для сохранения: ";
                string newFilename;
                getline(cin, newFilename);

                if (newFilename.empty()) {
                    newFilename = "phonebook.json";
                }

                // Добавляем расширение .json если его нет
                if (newFilename.size() < 5 || newFilename.substr(newFilename.size() - 5) != ".json") {
                    newFilename += ".json";
                }

                writeJSON(newFilename, contacts);
                cout << "Контакты сохранены в " << newFilename << endl;
            }
            else if (saveChoice == "3") {
                cout << "Изменения не сохранены.\n";
            }
            else {
                cout << "Неверный выбор. Сохраняю в текущий файл...\n";
                writeJSON(filename, contacts);
                cout << "Контакты сохранены в " << filename << endl;
            }

            cout << "Выход из программы.\n";
            return 0;
        }
        else {
            cout << "Неверный выбор! Введите 1-7 или 'help' для справки\n";
        }
    }

    return 0;
}
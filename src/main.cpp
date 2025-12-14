#include "phonebook.h"
#include <windows.h>
#include <limits>

// Главная функция
int main(int argc, char* argv[]) {
    // Настройка консоли для русского языка
    setlocale(LC_ALL, "");
    system("chcp 1251 > nul");

    // Загружаем контакты из файла
    vector<Contact> contacts = readJSON("phonebook.json");
    cout << "=== ТЕЛЕФОННЫЙ СПРАВОЧНИК ===\n";
    cout << "Контактов: " << contacts.size() << endl;

    // Режим командной строки
    if (argc > 1 && string(argv[1]) == "--find" && argc > 2) {
        string search_name = argv[2];
        long long time_ms = 0;

        auto result = findContactsByPrefix(contacts, search_name, time_ms);

        cout << "Найдено: " << result.size() << endl;
        cout << "Время: " << time_ms << " мс\n";

        if (!result.empty()) {
            cout << "\nРезультаты:\n";
            for (const auto& contact : result) {
                cout << "  " << contact.name << " | "
                    << contact.phone << " | "
                    << contact.email << endl;
            }
        }
        return 0;
    }

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
        cout << "Ваш выбор: ";

        int choice;
        cin >> choice;

        // Проверка ввода
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Ошибка! Введите число 1-7\n";
            continue;
        }

        cin.ignore(); // очищаем буфер

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
            cout << "Поиск по имени: ";
            string prefix;
            getline(cin, prefix);

            long long time_ms = 0;
            auto found = findContactsByPrefix(contacts, prefix, time_ms);

            cout << "Найдено: " << found.size() << endl;
            cout << "Время: " << time_ms << " мс\n";
            showContacts(found, "РЕЗУЛЬТАТЫ");
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
            writeJSON("phonebook.json", contacts);
            cout << "Сохранено. Выход.\n";
            return 0;
        }
        else {
            cout << "Неверный выбор! Введите 1-7\n";
        }
    }

    return 0;
}
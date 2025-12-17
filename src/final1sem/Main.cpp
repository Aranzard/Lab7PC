#include "Header.h"

// Главная функция
int main() {
    // Настройка консоли для русского языка
    setlocale(LC_ALL, "");

    string filename, input;
    cout << "Введите какой файл считать: ";
    cin >> filename;
    getline(cin, input);
    // Загружаем контакты из файла
    vector<Contact> contacts = readJSON(filename);
    cout << "\n=== ТЕЛЕФОННЫЙ СПРАВОЧНИК ===\n";
    cout << "Контактов: " << contacts.size() << endl;

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

        getline(cin, input);

        // Проверяем, что вся строка состоит из цифр
        bool allDigits = !input.empty();
        for (char c : input) {
            if (!isdigit(c)) {
                allDigits = false;
                break;
            }
        }

        if (!allDigits) {
            cout << "Ошибка! Введите число 1-7\n";
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
            cout << "Поиск по имени: ";
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
            cout << "Список будет сохранен в файл temp.json";
            writeJSON("temp.json", contacts);
            cout << "Сохранено. Выход.\n";
            return 0;
        }
        else {
            cout << "Неверный выбор! Введите 1-7\n";
        }
    }

    return 0;
}
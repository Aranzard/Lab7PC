#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <regex>
#include <chrono>
#include <map>

using namespace std;
using namespace chrono;

struct Contact
{
    string name, phone, email;
    vector<string> tags;

    bool validate() const
    {
        if (name.empty())
            return false;

        regex phone_regex(R"(^[\d\s+\-\(\)]{7,20}$)");
        if (!regex_match(phone, phone_regex))
            return false;

        regex email_regex(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
        return regex_match(email, email_regex);
    }

    string to_str() const
    {
        return name + " | " + phone + " | " + email;
    }
};

class PhoneBook
{
private:
    vector<Contact> contacts;
    map<string, vector<int>> prefix_index;

    void build_index()
    {
        prefix_index.clear();
        for (size_t i = 0; i < contacts.size(); ++i)
        {
            string name_lower = to_lower(contacts[i].name);
            for (size_t len = 1; len <= name_lower.size(); ++len)
            {
                string prefix = name_lower.substr(0, len);
                prefix_index[prefix].push_back(i);
            }
        }
    }

    string to_lower(const string &s) const
    {
        string result = s;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    double similarity(const string &a, const string &b) const
    {
        string a_low = to_lower(a);
        string b_low = to_lower(b);
        if (a_low == b_low)
            return 1.0;

        size_t matches = 0;
        size_t min_len = min(a_low.size(), b_low.size());
        for (size_t i = 0; i < min_len; ++i)
        {
            if (a_low[i] == b_low[i])
                ++matches;
        }
        return (double)matches / max(a_low.size(), b_low.size());
    }

public:
    bool load(const string &filename)
    {
        ifstream file(filename);
        if (!file)
            return false;

        contacts.clear();
        string line;
        while (getline(file, line))
        {
            if (line.find("\"name\":") != string::npos)
            {
                Contact c;
                extract_field(line, "\"name\":", c.name);
                if (getline(file, line))
                    extract_field(line, "\"phone\":", c.phone);
                if (getline(file, line))
                    extract_field(line, "\"email\":", c.email);
                if (getline(file, line) && line.find("\"tags\":") != string::npos)
                {
                    extract_tags(line, c.tags);
                }
                if (c.validate())
                    contacts.push_back(c);
            }
        }

        build_index();
        return true;
    }

    bool save(const string &filename) const
    {
        ofstream file(filename);
        if (!file)
            return false;

        file << "[\n";
        for (size_t i = 0; i < contacts.size(); ++i)
        {
            const auto &c = contacts[i];
            file << "  {\n";
            file << "    \"name\": \"" << c.name << "\",\n";
            file << "    \"phone\": \"" << c.phone << "\",\n";
            file << "    \"email\": \"" << c.email << "\",\n";
            file << "    \"tags\": [";
            for (size_t j = 0; j < c.tags.size(); ++j)
            {
                file << "\"" << c.tags[j] << "\"";
                if (j != c.tags.size() - 1)
                    file << ", ";
            }
            file << "]\n  }";
            if (i != contacts.size() - 1)
                file << ",";
            file << "\n";
        }
        file << "]\n";
        return true;
    }

    vector<Contact> find(const string &prefix, long long &time_ms)
    {
        auto start = high_resolution_clock::now();
        vector<Contact> result;
        string prefix_low = to_lower(prefix);

        for (const auto &entry : prefix_index)
        {
            if (entry.first.find(prefix_low) == 0)
            {
                for (int idx : entry.second)
                {
                    result.push_back(contacts[idx]);
                }
            }
        }

        // Удаляем дубликаты
        sort(result.begin(), result.end(), [](const Contact &a, const Contact &b)
             { return a.name < b.name; });
        result.erase(unique(result.begin(), result.end(),
                            [](const Contact &a, const Contact &b)
                            { return a.name == b.name; }),
                     result.end());

        auto end = high_resolution_clock::now();
        time_ms = duration_cast<milliseconds>(end - start).count();
        return result;
    }

    void sort_by_name()
    {
        sort(contacts.begin(), contacts.end(),
             [](const Contact &a, const Contact &b)
             { return a.name < b.name; });
        build_index();
    }

    int deduplicate(double threshold = 0.8)
    {
        vector<Contact> unique;
        vector<bool> keep(contacts.size(), true);

        for (size_t i = 0; i < contacts.size(); ++i)
        {
            if (!keep[i])
                continue;
            unique.push_back(contacts[i]);

            for (size_t j = i + 1; j < contacts.size(); ++j)
            {
                if (keep[j] && similarity(contacts[i].name, contacts[j].name) >= threshold)
                {
                    keep[j] = false;
                }
            }
        }

        int removed = contacts.size() - unique.size();
        contacts = unique;
        build_index();
        return removed;
    }

    void print_stats() const
    {
        cout << "Контактов: " << contacts.size() << endl;

        map<string, int> tag_counts;
        for (const auto &c : contacts)
        {
            for (const auto &tag : c.tags)
            {
                tag_counts[tag]++;
            }
        }

        if (!tag_counts.empty())
        {
            cout << "\nТеги:\n";
            for (const auto &[tag, count] : tag_counts)
            {
                cout << "  " << tag << ": " << count << endl;
            }
        }
    }

private:
    void extract_field(const string &line, const string &key, string &field)
    {
        size_t start = line.find('"', line.find(key) + key.length()) + 1;
        size_t end = line.find('"', start);
        if (start != string::npos && end != string::npos)
        {
            field = line.substr(start, end - start);
        }
    }

    void extract_tags(const string &line, vector<string> &tags)
    {
        size_t start = line.find('[') + 1;
        size_t end = line.find(']');
        if (start >= end)
            return;

        string tags_str = line.substr(start, end - start);
        size_t pos = 0;
        while (pos < tags_str.size())
        {
            size_t quote1 = tags_str.find('"', pos);
            if (quote1 == string::npos)
                break;
            size_t quote2 = tags_str.find('"', quote1 + 1);
            if (quote2 == string::npos)
                break;

            tags.push_back(tags_str.substr(quote1 + 1, quote2 - quote1 - 1));
            pos = quote2 + 1;
        }
    }
};

void print_help()
{
    cout << "Использование:\n"
         << "  --input FILE    Загрузить данные\n"
         << "  --output FILE   Сохранить данные\n"
         << "  --find STR      Поиск по префиксу\n"
         << "  --sort          Сортировать по имени\n"
         << "  --dedup [0-1]   Удалить дубликаты (по умолч. 0.8)\n"
         << "  --stats         Показать статистику\n"
         << "  --help          Справка\n";
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_help();
        return 1;
    }

    PhoneBook pb;
    string input_file, output_file;

    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];

        if (arg == "--help")
        {
            print_help();
            return 0;
        }
        else if (arg == "--input" && i + 1 < argc)
        {
            input_file = argv[++i];
            if (!pb.load(input_file))
            {
                cerr << "Ошибка загрузки: " << input_file << endl;
                return 1;
            }
        }
        else if (arg == "--output" && i + 1 < argc)
        {
            output_file = argv[++i];
        }
        else if (arg == "--find" && i + 1 < argc)
        {
            if (input_file.empty())
            {
                cerr << "Сначала загрузите данные (--input)" << endl;
                return 1;
            }

            string prefix = argv[++i];
            long long time_ms;
            auto results = pb.find(prefix, time_ms);

            cout << "Найдено: " << results.size()
                 << " (время: " << time_ms << " мс)\n\n";

            for (const auto &c : results)
            {
                cout << c.to_str() << endl;
                if (!c.tags.empty())
                {
                    cout << "  Теги: ";
                    for (const auto &tag : c.tags)
                        cout << tag << " ";
                    cout << endl;
                }
                cout << endl;
            }
        }
        else if (arg == "--sort")
        {
            pb.sort_by_name();
            cout << "Отсортировано по имени\n";
        }
        else if (arg == "--dedup")
        {
            double threshold = 0.8;
            if (i + 1 < argc && string(argv[i + 1]).find("--") != 0)
            {
                try
                {
                    threshold = stod(argv[++i]);
                }
                catch (...)
                {
                    --i;
                }
            }

            int removed = pb.deduplicate(threshold);
            cout << "Удалено дубликатов: " << removed
                 << " (порог: " << threshold << ")\n";
        }
        else if (arg == "--stats")
        {
            pb.print_stats();
        }
    }

    if (!output_file.empty())
    {
        if (pb.save(output_file))
        {
            cout << "Сохранено в: " << output_file << endl;
        }
        else
        {
            cerr << "Ошибка сохранения" << endl;
            return 1;
        }
    }

    return 0;
}
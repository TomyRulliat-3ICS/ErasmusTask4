#include <iostream>
#include "sqlite3.h"
#include <string>
#include <iomanip>
#include <fstream>

using namespace std;

class Book {
public:
    int id;
    string title;
    string author;
    int year;

    void input() {
        cout << "Enter ID: "; cin >> id;
        cin.ignore();
        cout << "Enter Title: "; getline(cin, title);
        cout << "Enter Author: "; getline(cin, author);
        cout << "Enter Year: "; cin >> year;
    }
};

class RecordManager {
    sqlite3* db;

public:
    RecordManager() {
        if (sqlite3_open("data.db", &db)) {
            cerr << "Can't open DB: " << sqlite3_errmsg(db) << endl;
            exit(1);
        }

        const char* createTableSQL =
            "CREATE TABLE IF NOT EXISTS books ("
            "id INTEGER PRIMARY KEY,"
            "title TEXT,"
            "author TEXT,"
            "year INTEGER);";

        char* errMsg;
        if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }

    ~RecordManager() {
        sqlite3_close(db);
    }

    void addRecord() {
        Book b;
        b.input();
        string sql = "INSERT INTO books VALUES (" + to_string(b.id) + ", '" + b.title + "', '" + b.author + "', " + to_string(b.year) + ");";
        char* errMsg;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "Error: " << errMsg << endl;
            sqlite3_free(errMsg);
        } else {
            cout << "Record added.\n";
        }
    }

    static int displayCallback(void*, int argc, char** argv, char**) {
        cout << left << setw(5) << argv[0] << setw(30) << argv[1] << setw(20) << argv[2] << argv[3] << endl;
        return 0;
    }

    void displayAllRecords() {
        cout << left << setw(5) << "ID" << setw(30) << "Title" << setw(20) << "Author" << "Year" << endl;
        const char* sql = "SELECT * FROM books";
        char* errMsg;
        if (sqlite3_exec(db, sql, displayCallback, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "Error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }

    void searchRecordByID(int id) {
        string sql = "SELECT * FROM books WHERE id = " + to_string(id);
        cout << left << setw(5) << "ID" << setw(30) << "Title" << setw(20) << "Author" << "Year" << endl;
        char* errMsg;
        if (sqlite3_exec(db, sql.c_str(), displayCallback, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "Error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }

    void deleteRecord(int id) {
        string sql = "DELETE FROM books WHERE id = " + to_string(id);
        char* errMsg;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "Error: " << errMsg << endl;
            sqlite3_free(errMsg);
        } else {
            cout << "Record deleted.\n";
        }
    }

    void updateRecord(int id) {
        Book b;
        cout << "Enter new details:\n";
        b.input();
        string sql = "UPDATE books SET title='" + b.title + "', author='" + b.author + "', year=" + to_string(b.year) + " WHERE id=" + to_string(id);
        char* errMsg;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "Error: " << errMsg << endl;
            sqlite3_free(errMsg);
        } else {
            cout << "Record updated.\n";
        }
    }

    void exportToCSV() {
        const char* sql = "SELECT * FROM books ORDER BY id";
        sqlite3_stmt* stmt;
        ofstream out("records.csv");
        out << "ID,Title,Author,Year\n";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                out << sqlite3_column_int(stmt, 0) << ","
                    << sqlite3_column_text(stmt, 1) << ","
                    << sqlite3_column_text(stmt, 2) << ","
                    << sqlite3_column_int(stmt, 3) << "\n";
            }
        }
        sqlite3_finalize(stmt);
        out.close();
        cout << "Exported to records.csv\n";
    }
};

int main() {
    RecordManager rm;
    int choice, id;
    while (true) {
        cout << "\n--- Record Management System (SQLite) ---\n";
        cout << "1. Add Record\n2. Display All Records\n3. Search Record by ID\n4. Delete Record\n5. Update Record\n6. Export to CSV\n7. Exit\nEnter choice: ";
        cin >> choice;
        switch (choice) {
            case 1: rm.addRecord(); break;
            case 2: rm.displayAllRecords(); break;
            case 3: cout << "Enter ID to search: "; cin >> id; rm.searchRecordByID(id); break;
            case 4: cout << "Enter ID to delete: "; cin >> id; rm.deleteRecord(id); break;
            case 5: cout << "Enter ID to update: "; cin >> id; rm.updateRecord(id); break;
            case 6: rm.exportToCSV(); break;
            case 7: return 0;
            default: cout << "Invalid choice!\n";
        }
    }
}
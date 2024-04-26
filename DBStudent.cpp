#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <pqxx/pqxx>


struct Student {
    int id;
    std::string name;
    int age;
};

class StudentDatabase {
private:
    std::vector<std::shared_ptr<Student>> students; 
    pqxx::connection connection; 

public:
    StudentDatabase(const std::string& dbname, const std::string& user, const std::string& password, const std::string& host)
        : connection("dbname=" + dbname + " user=" + user + " password=" + password + " host=" + host) {

        if (!connection.is_open()) {
            std::cerr << "Failed to connect to PostgreSQL." << std::endl;
        }
    }

    void addStudent(const std::string& name, int age) {
        std::shared_ptr<Student> student = std::make_shared<Student>();
        student->name = name;
        student->age = age;

        pqxx::work txn(connection);
        pqxx::result result = txn.exec("INSERT INTO students (name, age) VALUES ('" + student->name + "', " + std::to_string(student->age) + ") RETURNING id;");
        txn.commit();

        student->id = result[0][0].as<int>();

        students.push_back(student);
    }

    void removeStudent(int id) {
        pqxx::work txn(connection);
        txn.exec("DELETE FROM students WHERE id = " + std::to_string(id) + ";");
        txn.commit();

        students.erase(std::remove_if(students.begin(), students.end(),
            [id](const std::shared_ptr<Student>& student) { return student->id == id; }),
            students.end());
    }

    std::shared_ptr<Student> getStudent(int id) {
        auto it = std::find_if(students.begin(), students.end(),
            [id](const std::shared_ptr<Student>& student) { return student->id == id; });

        if (it != students.end()) {
            return *it;
        }

        return nullptr; 
    }
};

int main() {
    StudentDatabase db("student_task", "molodoymaxim", "1234", "localhost");

    db.addStudent("Maxim", 20);

    auto student = db.getStudent(1);
    if (student) {
        std::cout << "Student ID: " << student->id << ", Name: " << student->name << ", Age: " << student->age << std::endl;
    }

    db.removeStudent(1);

    return 0;
}

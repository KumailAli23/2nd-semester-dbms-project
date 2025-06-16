// gui_main.cpp

// ImGui Includes
#include "imgui-master/imgui.h"
#include "imgui-master/backends/imgui_impl_glfw.h"
#include "imgui-master/backends/imgui_impl_opengl3.h"

#include <stdio.h> // For printf

// GLAD and GLFW Includes
// GLAD must be included before GLFW
#include "glfw-3.4.bin.WIN64/include/glad/glad.h"
#include "glfw-3.4.bin.WIN64/include/GLFW/glfw3.h"

// Standard C++ and ODBC Includes
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <cstring> // For strcpy, strlen, strcmp

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

// --- Global ODBC Handles ---
SQLHENV hEnv = SQL_NULL_HENV;
SQLHDBC hDbc = SQL_NULL_HDBC;
SQLHSTMT hStmt = SQL_NULL_HSTMT;

// --- Data Structures for UI (Declare Structs First - GLOBAL) ---
struct CustomerData { std::string cnic, name, address, phoneNumber; };
struct AllowedAnimalData { std::string animal_type; };
struct EmployeeData { int emp_id = 0; std::string cnic; std::string name; std::string workerOrTrainer; std::string address; };
struct TrainingData { int training_id = 0; std::string name; };
struct WorkerData { int worker_id = 0; std::string employeeName; std::string workerType; int age = 0;};
struct TrainerData { 
    int trainer_id = 0;      // This IS the Emp_id
    std::string employeeName; 
    int age = 0;
    std::string phone;
};
struct TrainingTrainerData { // For "Service by Trainer"
    int tt_id = 0;
    int training_id = 0;
    int trainer_id = 0;
    // For display purposes
    std::string trainingName;
    std::string trainerName; 
};
// ... (after your last struct definition, e.g., TrainingTrainerData)

struct AnimalData { 
    int animal_id = 0;
    std::string animalType;    // FK to Allowed_Animal
    std::string cnic_owner;    // FK to Customer
    std::string ownerNameDisplay; // For displaying customer's name
    std::string animalName;
    int age = 0;
    std::string startDate;     // Store as YYYY-MM-DD string
    std::string returnDate;    // Store as YYYY-MM-DD string (can be empty/NULL)
};

struct AnimalTrainingScheduleData { // For "Animal Training Schedule"
    int animal_id = 0;             // FK to Animal
    int tt_id = 0;                 // FK to Training_Trainer (Service by Trainer)
    // For display purposes
    std::string animalNameDisplay;
    std::string trainingNameDisplay; // From Training via TT_id
    std::string trainerNameDisplay;  // From Employee via TT_id
};



// --- Global UI State Variables (GLOBAL) ---
// Customer
std::vector<CustomerData> customers_list_display;
char cust_add_cnic_buf[21]="", cust_add_name_buf[256]="", cust_add_address_buf[256]="", cust_add_phone_buf[21]="";
char cust_search_cnic_buf[21]="";
int cust_selected_idx = -1; // Index in the display list, less critical if using unique ID for ops
std::string cust_selected_cnic_for_ops;
char cust_update_name_buf[256]="", cust_update_address_buf[256]="", cust_update_phone_buf[21]="";

// Allowed Animal
std::vector<AllowedAnimalData> allowed_animals_list_display;
char aa_add_type_buf[256]="";
std::string aa_selected_type_for_ops; // This will hold the type selected for deletion

// Employee
std::vector<EmployeeData> employees_list_display;
char emp_add_cnic_buf[21]="", emp_add_name_buf[256]="", emp_add_role_buf[256]="", emp_add_address_buf[256]="";
int emp_selected_id_for_ops = 0;
std::string emp_selected_cnic_display; // For displaying the CNIC of selected employee (non-editable)
char emp_update_name_buf[256]="", emp_update_role_buf[256]="", emp_update_address_buf[256]="";

// Training
std::vector<TrainingData> trainings_list_display;
char tr_add_name_buf[256]="";
int tr_selected_id_for_ops = 0;
char tr_update_name_buf[256]="";

// Worker
std::vector<WorkerData> workers_list_display;
int worker_add_selected_emp_id = 0;
std::vector<EmployeeData> available_employees_for_worker;
char worker_add_type_buf[256] = "";
int worker_add_age_val = 0;
int worker_selected_id_for_ops = 0;
std::string worker_selected_employee_name_for_ops;
char worker_update_type_buf[256] = "";
int worker_update_age_val = 0;

// Trainer
std::vector<TrainerData> trainers_list_display;
std::vector<EmployeeData> available_employees_for_trainer; // To populate dropdown
int trainer_add_selected_emp_id = 0;
int trainer_add_age_val = 0; 
char trainer_add_phone_buf[20] = "";
int trainer_selected_id_for_ops = 0; 
std::string trainer_selected_employee_name_for_ops;
int trainer_update_age_val = 0; 
char trainer_update_phone_buf[20] = "";

// Training_Trainer (Service by Trainer)
std::vector<TrainingTrainerData> service_by_trainer_list_display;
int sbt_add_selected_training_id = 0;
int sbt_add_selected_trainer_id = 0; // This will be the Trainer's Emp_id
std::vector<TrainingData> available_trainings_for_sbt; 
std::vector<TrainerData> available_trainers_for_sbt;   // List of *actual* Trainers (not all employees)
int sbt_selected_tt_id_for_ops = 0; 

// Animal
std::vector<AnimalData> animals_list_display;
char animal_add_name_buf[256] = "";
int animal_add_age_val = 0;
char animal_add_start_date_buf[12] = ""; // YYYY-MM-DD + null
char animal_add_return_date_buf[12] = "";
// For selecting FKs when adding an Animal
std::string animal_add_selected_allowed_type = "";         
std::vector<AllowedAnimalData> available_allowed_types_for_animal; // You'll populate this
std::string animal_add_selected_customer_cnic = "";      
std::vector<CustomerData> available_customers_for_animal;       // You'll populate this

int animal_selected_id_for_ops = 0; 
std::string animal_selected_name_for_display; 
// Buffers for updating animal
char animal_update_name_buf[256] = "";
int animal_update_age_val = 0;
char animal_update_start_date_buf[12] = "";
char animal_update_return_date_buf[12] = "";
std::string animal_update_selected_allowed_type = ""; 
std::string animal_update_selected_customer_cnic = "";

// Animal Training Schedule (Animal_Training_Trainer)
std::vector<AnimalTrainingScheduleData> animal_schedule_list_display;
int ats_add_selected_animal_id = 0;
int ats_add_selected_tt_id = 0; 
std::vector<AnimalData> available_animals_for_ats; 
std::vector<TrainingTrainerData> available_tt_assignments_for_ats;
// For deletion, we need both parts of the composite key
int ats_delete_selected_animal_id_key = 0;
int ats_delete_selected_tt_id_key = 0;


// Global Status Message
std::string statusMessage = "Welcome! Please connect to the database.";


// --- Helper Functions ---
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

std::string sanitizeSqlString(const std::string& s) {
    std::string sanitized = s;
    size_t pos = 0;
    while ((pos = sanitized.find("'", pos)) != std::string::npos) {
        sanitized.replace(pos, 1, "''");
        pos += 2;
    }
    return sanitized;
}

void checkSQLError(SQLHANDLE handle, SQLSMALLINT handleType, SQLRETURN retcode, const std::string& context = "") {
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) return;
    SQLCHAR sqlState[6]; SQLINTEGER nativeError; SQLCHAR messageText[SQL_MAX_MESSAGE_LENGTH]; SQLSMALLINT textLength;
    std::cerr << "ERROR: " << context << " - SQLRETURN: " << retcode << std::endl;
    SQLSMALLINT i = 1;
    while (SQLGetDiagRec(handleType, handle, i, sqlState, &nativeError, messageText, sizeof(messageText), &textLength) == SQL_SUCCESS) {
        std::cerr << "  SQLState: " << (const char*)sqlState << ", NativeError: " << nativeError << std::endl;
        std::cerr << "  Message: " << (const char*)messageText << std::endl;
        i++;
    }
    if (retcode == SQL_ERROR || retcode == SQL_INVALID_HANDLE) {
        if (context.find("Connect") != std::string::npos || context.find("Alloc") != std::string::npos) {
            std::cerr << "CRITICAL: Database connection or handle allocation failed." << std::endl;
        } else {
            std::cerr << "SQL error encountered. Context: " << context << std::endl;
        }
    }
}

bool connectDB() {
    SQLRETURN retcode;
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    if (!SQL_SUCCEEDED(retcode)) { checkSQLError(SQL_NULL_HANDLE, SQL_HANDLE_ENV, retcode, "connectDB - SQLAllocHandle for HENV"); return false; }
    retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(retcode)) { checkSQLError(hEnv, SQL_HANDLE_ENV, retcode, "connectDB - SQLSetEnvAttr for ODBC Version"); SQLFreeHandle(SQL_HANDLE_ENV, hEnv); hEnv = SQL_NULL_HENV; return false; }
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    if (!SQL_SUCCEEDED(retcode)) { checkSQLError(hEnv, SQL_HANDLE_ENV, retcode, "connectDB - SQLAllocHandle for HDBC"); SQLFreeHandle(SQL_HANDLE_ENV, hEnv); hEnv = SQL_NULL_HENV; return false; }
    SQLCHAR connStr[] = "DRIVER={SQL Server};SERVER=DESKTOP-DFK3ACO;DATABASE=DBMS_PROJECT;Trusted_Connection=yes;";
    retcode = SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(retcode)) { checkSQLError(hDbc, SQL_HANDLE_DBC, retcode, "connectDB - SQLDriverConnect"); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); hDbc = SQL_NULL_HDBC; SQLFreeHandle(SQL_HANDLE_ENV, hEnv); hEnv = SQL_NULL_HENV; return false; }
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (!SQL_SUCCEEDED(retcode)) { checkSQLError(hDbc, SQL_HANDLE_DBC, retcode, "connectDB - SQLAllocHandle for HSTMT"); SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); hDbc = SQL_NULL_HDBC; SQLFreeHandle(SQL_HANDLE_ENV, hEnv); hEnv = SQL_NULL_HENV; return false; }
    std::cout << "Successfully connected to database DBMS_PROJECT." << std::endl;
    return true;
}
void disconnectDB() {
    if (hStmt != SQL_NULL_HSTMT) { SQLFreeHandle(SQL_HANDLE_STMT, hStmt); hStmt = SQL_NULL_HSTMT; }
    if (hDbc != SQL_NULL_HDBC) { SQLDisconnect(hDbc); SQLFreeHandle(SQL_HANDLE_DBC, hDbc); hDbc = SQL_NULL_HDBC; }
    if (hEnv != SQL_NULL_HENV) { SQLFreeHandle(SQL_HANDLE_ENV, hEnv); hEnv = SQL_NULL_HENV; }
    std::cout << "Disconnected from database." << std::endl;
}
bool executeNonQuery(const std::string& sqlQueryStr) {
    if (hStmt == SQL_NULL_HSTMT) { std::cerr << "executeNonQuery Error: Statement handle is null." << std::endl; return false; }
    std::vector<SQLCHAR> sqlQueryBuffer(sqlQueryStr.begin(), sqlQueryStr.end()); sqlQueryBuffer.push_back('\0');
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQueryBuffer.data(), SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) { SQLCloseCursor(hStmt); return true; }
    else { checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "Executing query: " + sqlQueryStr); SQLCloseCursor(hStmt); return false; }
}

// --- UI-Specific Database Functions ---
// == Customer Functions ==
std::vector<CustomerData> ui_fetchAllCustomers() {
    std::vector<CustomerData> customers; if (hStmt == SQL_NULL_HSTMT) return customers;
    SQLCHAR sqlQuery[] = "SELECT CNIC, Name, Address, PhoneNumber FROM Customer;";
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLCHAR db_cnic[21], db_name[256], db_address[256], db_phone[21]; SQLLEN lenCnic=0, lenName=0, lenAddress=0, lenPhone=0;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            CustomerData cust;
            SQLGetData(hStmt, 1, SQL_C_CHAR, db_cnic, sizeof(db_cnic), &lenCnic); SQLGetData(hStmt, 2, SQL_C_CHAR, db_name, sizeof(db_name), &lenName);
            SQLGetData(hStmt, 3, SQL_C_CHAR, db_address, sizeof(db_address), &lenAddress); SQLGetData(hStmt, 4, SQL_C_CHAR, db_phone, sizeof(db_phone), &lenPhone);
            cust.cnic = (lenCnic != SQL_NULL_DATA && lenCnic > 0) ? std::string(reinterpret_cast<const char*>(db_cnic), lenCnic) : "";
            cust.name = (lenName != SQL_NULL_DATA && lenName > 0) ? std::string(reinterpret_cast<const char*>(db_name), lenName) : "";
            cust.address = (lenAddress != SQL_NULL_DATA && lenAddress > 0) ? std::string(reinterpret_cast<const char*>(db_address), lenAddress) : "";
            cust.phoneNumber = (lenPhone != SQL_NULL_DATA && lenPhone > 0) ? std::string(reinterpret_cast<const char*>(db_phone), lenPhone) : "";
            customers.push_back(cust);
        } SQLCloseCursor(hStmt);
    } else { checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllCustomers"); SQLCloseCursor(hStmt); }
    return customers;
}
bool ui_insertCustomer(const CustomerData& customer) {
    if (customer.cnic.empty() || customer.name.empty()) { statusMessage = "Error: CNIC and Name cannot be empty."; return false; }
    std::string sql = "INSERT INTO Customer (CNIC, Name, Address, PhoneNumber) VALUES ('" +
                      sanitizeSqlString(customer.cnic) + "', '" + sanitizeSqlString(customer.name) + "', '" +
                      sanitizeSqlString(customer.address) + "', '" + sanitizeSqlString(customer.phoneNumber) + "');";
    return executeNonQuery(sql);
}
std::vector<CustomerData> ui_searchCustomerByCNIC(const std::string& cnic_search_raw) {
    std::vector<CustomerData> customers; if (cnic_search_raw.empty() || hStmt == SQL_NULL_HSTMT) return customers;
    std::string cnic_search = sanitizeSqlString(cnic_search_raw);
    std::string sql_str = "SELECT CNIC, Name, Address, PhoneNumber FROM Customer WHERE CNIC = '" + cnic_search + "';";
    std::vector<SQLCHAR> sqlQuery(sql_str.begin(), sql_str.end()); sqlQuery.push_back('\0');
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery.data(), SQL_NTS);
     if (SQL_SUCCEEDED(retcode)) {
        SQLCHAR db_cnic[21], db_name[256], db_address[256], db_phone[21]; SQLLEN lenCnic=0, lenName=0, lenAddress=0, lenPhone=0;
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            CustomerData cust;
            SQLGetData(hStmt, 1, SQL_C_CHAR, db_cnic, sizeof(db_cnic), &lenCnic); SQLGetData(hStmt, 2, SQL_C_CHAR, db_name, sizeof(db_name), &lenName);
            SQLGetData(hStmt, 3, SQL_C_CHAR, db_address, sizeof(db_address), &lenAddress); SQLGetData(hStmt, 4, SQL_C_CHAR, db_phone, sizeof(db_phone), &lenPhone);
            cust.cnic = (lenCnic != SQL_NULL_DATA && lenCnic > 0) ? std::string(reinterpret_cast<const char*>(db_cnic), lenCnic) : "";
            cust.name = (lenName != SQL_NULL_DATA && lenName > 0) ? std::string(reinterpret_cast<const char*>(db_name), lenName) : "";
            cust.address = (lenAddress != SQL_NULL_DATA && lenAddress > 0) ? std::string(reinterpret_cast<const char*>(db_address), lenAddress) : "";
            cust.phoneNumber = (lenPhone != SQL_NULL_DATA && lenPhone > 0) ? std::string(reinterpret_cast<const char*>(db_phone), lenPhone) : "";
            customers.push_back(cust);
        } SQLCloseCursor(hStmt);
    } else { checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_searchCustomerByCNIC: " + sql_str); SQLCloseCursor(hStmt); }
    return customers;
}
bool ui_updateCustomer(const std::string& cnic_to_update_raw, const CustomerData& data_with_new_values) {
    if (cnic_to_update_raw.empty()) {
        statusMessage = "Error: CNIC to update cannot be empty.";
        return false;
    }
    std::string cnic_to_update = sanitizeSqlString(cnic_to_update_raw);
    std::vector<std::string> set_clauses;
    if (!data_with_new_values.name.empty()) { // Only add to SET if new data is provided
        set_clauses.push_back("Name = '" + sanitizeSqlString(data_with_new_values.name) + "'");
    }
    if (!data_with_new_values.address.empty()) {
        set_clauses.push_back("Address = '" + sanitizeSqlString(data_with_new_values.address) + "'");
    }
    if (!data_with_new_values.phoneNumber.empty()) {
        set_clauses.push_back("PhoneNumber = '" + sanitizeSqlString(data_with_new_values.phoneNumber) + "'");
    }
    if (set_clauses.empty()) {
        statusMessage = "No new data fields provided to update customer " + cnic_to_update;
        return false;
    }
    std::string sql = "UPDATE Customer SET ";
    for (size_t i = 0; i < set_clauses.size(); ++i) {
        sql += set_clauses[i] + (i < set_clauses.size() - 1 ? ", " : "");
    }
    sql += " WHERE CNIC = '" + cnic_to_update + "';";
    return executeNonQuery(sql);
}
bool ui_deleteCustomer(const std::string& cnic_to_delete_raw) {
    if (cnic_to_delete_raw.empty()) { statusMessage = "Error: CNIC to delete cannot be empty."; return false; }
    std::string cnic_to_delete = sanitizeSqlString(cnic_to_delete_raw);
    std::string sql = "DELETE FROM Customer WHERE CNIC = '" + cnic_to_delete + "';";
    return executeNonQuery(sql);
}

// == AllowedAnimal Functions ==
std::vector<AllowedAnimalData> ui_fetchAllAllowedAnimals() {
    std::vector<AllowedAnimalData> types; if (hStmt == SQL_NULL_HSTMT) return types;
    SQLCHAR sqlQuery[] = "SELECT Animal_Type FROM Allowed_Animal;";
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLCHAR db_type[256]; SQLLEN lenType = 0;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            AllowedAnimalData ad; SQLGetData(hStmt, 1, SQL_C_CHAR, db_type, sizeof(db_type), &lenType);
            ad.animal_type = (lenType != SQL_NULL_DATA && lenType > 0) ? std::string(reinterpret_cast<const char*>(db_type), lenType) : "";
            types.push_back(ad);
        } SQLCloseCursor(hStmt);
    } else { checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllAllowedAnimals"); SQLCloseCursor(hStmt); }
    return types;
}
bool ui_insertAllowedAnimal(const AllowedAnimalData& data) {
    if (data.animal_type.empty()) { statusMessage = "Error: Animal type cannot be empty."; return false; }
    std::string sql = "INSERT INTO Allowed_Animal (Animal_Type) VALUES ('" + sanitizeSqlString(toLower(data.animal_type)) + "');";
    return executeNonQuery(sql);
}
bool ui_deleteAllowedAnimal(const std::string& type_to_delete_raw) {
    if (type_to_delete_raw.empty()) {statusMessage = "Error: Type to delete cannot be empty."; return false;}
    std::string type_to_delete = sanitizeSqlString(toLower(type_to_delete_raw));
    std::string sql = "DELETE FROM Allowed_Animal WHERE Animal_Type = '" + type_to_delete + "';";
    return executeNonQuery(sql);
}

// == Employee Functions ==
std::vector<EmployeeData> ui_fetchAllEmployees() {
    std::vector<EmployeeData> employees; if (hStmt == SQL_NULL_HSTMT) return employees;
    SQLCHAR sqlQuery[] = "SELECT Emp_id, CNIC, Name, WorkerOrTrainer, Address FROM Employee;";
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLINTEGER db_emp_id; SQLCHAR db_cnic[21], db_name[256], db_role[256], db_address[256];
        SQLLEN lenEmpId=0, lenCnic=0, lenName=0, lenRole=0, lenAddress=0;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            EmployeeData emp;
            SQLGetData(hStmt, 1, SQL_C_SLONG, &db_emp_id, 0, &lenEmpId); SQLGetData(hStmt, 2, SQL_C_CHAR, db_cnic, sizeof(db_cnic), &lenCnic);
            SQLGetData(hStmt, 3, SQL_C_CHAR, db_name, sizeof(db_name), &lenName); SQLGetData(hStmt, 4, SQL_C_CHAR, db_role, sizeof(db_role), &lenRole);
            SQLGetData(hStmt, 5, SQL_C_CHAR, db_address, sizeof(db_address), &lenAddress);
            emp.emp_id = (lenEmpId != SQL_NULL_DATA) ? db_emp_id : 0;
            emp.cnic = (lenCnic != SQL_NULL_DATA && lenCnic > 0) ? std::string(reinterpret_cast<const char*>(db_cnic), lenCnic) : "";
            emp.name = (lenName != SQL_NULL_DATA && lenName > 0) ? std::string(reinterpret_cast<const char*>(db_name), lenName) : "";
            emp.workerOrTrainer = (lenRole != SQL_NULL_DATA && lenRole > 0) ? std::string(reinterpret_cast<const char*>(db_role), lenRole) : "";
            emp.address = (lenAddress != SQL_NULL_DATA && lenAddress > 0) ? std::string(reinterpret_cast<const char*>(db_address), lenAddress) : "";
            employees.push_back(emp);
        } SQLCloseCursor(hStmt);
    } else { checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllEmployees"); SQLCloseCursor(hStmt); }
    return employees;
}
bool ui_insertEmployee(const EmployeeData& emp) {
    if (emp.cnic.empty() || emp.name.empty() || emp.workerOrTrainer.empty()) { statusMessage = "Error: CNIC, Name, and Role are required for Employee."; return false; }
    std::string role = toLower(emp.workerOrTrainer);
    if (role != "worker" && role != "trainer") {
        statusMessage = "Error: Employee role must be 'Worker' or 'Trainer'.";
        return false;
    }
    std::string sql = "INSERT INTO Employee (CNIC, Name, WorkerOrTrainer, Address) VALUES ('" +
                      sanitizeSqlString(emp.cnic) + "', '" + sanitizeSqlString(emp.name) + "', '" +
                      (role == "worker" ? "Worker" : "Trainer") + "', '" + sanitizeSqlString(emp.address) + "');";
    return executeNonQuery(sql);
}

// CORRECTED ui_updateEmployee function
bool ui_updateEmployee(int emp_id_to_update, const EmployeeData& data_with_new_values) {
    if (emp_id_to_update <= 0) {
        statusMessage = "Error: Employee ID to update is invalid.";
        return false;
    }
    std::vector<std::string> set_clauses;
    if (!data_with_new_values.name.empty()) {
        set_clauses.push_back("Name = '" + sanitizeSqlString(data_with_new_values.name) + "'");
    }
    if (!data_with_new_values.workerOrTrainer.empty()) {
        std::string role_str = toLower(data_with_new_values.workerOrTrainer);
        if (role_str != "worker" && role_str != "trainer") {
            statusMessage = "Error: Updated employee role must be 'Worker' or 'Trainer'.";
            return false;
        }
        // Corrected line using std::string concatenation
        set_clauses.push_back(std::string("WorkerOrTrainer = '") + (role_str == "worker" ? "Worker" : "Trainer") + "'");
    }
    if (!data_with_new_values.address.empty()) {
        set_clauses.push_back("Address = '" + sanitizeSqlString(data_with_new_values.address) + "'");
    }

    if (set_clauses.empty()) {
        statusMessage = "No new data fields provided to update employee ID " + std::to_string(emp_id_to_update);
        return false;
    }
    std::string sql = "UPDATE Employee SET ";
    for (size_t i = 0; i < set_clauses.size(); ++i) {
        sql += set_clauses[i] + (i < set_clauses.size() - 1 ? ", " : "");
    }
    sql += " WHERE Emp_id = " + std::to_string(emp_id_to_update) + ";";
    return executeNonQuery(sql);
}


bool ui_deleteEmployee(int emp_id_to_delete) {
    if (emp_id_to_delete <= 0) { statusMessage = "Error: Invalid Employee ID for deletion."; return false; }

    // SQL queries to check for dependencies
    std::string checkWorkerSql = "SELECT COUNT(*) FROM Worker WHERE Worker_id = " + std::to_string(emp_id_to_delete) + ";";
    std::string checkTrainerSql = "SELECT COUNT(*) FROM Trainer WHERE Trainer_id = " + std::to_string(emp_id_to_delete) + ";";

    SQLINTEGER count = 0;
    SQLLEN lenCount = 0;

    // Check Worker
    std::vector<SQLCHAR> workerQueryBuffer(checkWorkerSql.begin(), checkWorkerSql.end());
    workerQueryBuffer.push_back('\0');

    SQLRETURN retcode_worker_check = SQLExecDirect(hStmt, workerQueryBuffer.data(), SQL_NTS);
    if (SQL_SUCCEEDED(retcode_worker_check)) {
        count = 0; // Initialize before potential SQLGetData
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &count, 0, &lenCount);
        }
        SQLCloseCursor(hStmt); // Close cursor after processing results
        if (count > 0) {
            statusMessage = "Error: Cannot delete employee. They are registered as a Worker. Remove worker specialization first.";
            return false;
        }
    } else {
        // Pass the return code from the *original* SQLExecDirect call
        checkSQLError(hStmt, SQL_HANDLE_STMT, retcode_worker_check, "ui_deleteEmployee - check worker existence");
        SQLCloseCursor(hStmt); // Ensure cursor is closed on error
        return false; // Error during check
    }

    // Check Trainer
    count = 0; // Reset count for the trainer check
    std::vector<SQLCHAR> trainerQueryBuffer(checkTrainerSql.begin(), checkTrainerSql.end());
    trainerQueryBuffer.push_back('\0');

    SQLRETURN retcode_trainer_check = SQLExecDirect(hStmt, trainerQueryBuffer.data(), SQL_NTS);
     if (SQL_SUCCEEDED(retcode_trainer_check)) {
        count = 0; // Initialize before potential SQLGetData
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &count, 0, &lenCount);
        }
        SQLCloseCursor(hStmt); // Close cursor after processing results
        if (count > 0) {
            statusMessage = "Error: Cannot delete employee. They are registered as a Trainer. Remove trainer specialization first.";
            return false;
        }
    } else {
        // Pass the return code from the *original* SQLExecDirect call
        checkSQLError(hStmt, SQL_HANDLE_STMT, retcode_trainer_check, "ui_deleteEmployee - check trainer existence");
        SQLCloseCursor(hStmt); // Ensure cursor is closed on error
        return false; // Error during check
    }

    // If no dependencies, proceed with deletion
    std::string sql = "DELETE FROM Employee WHERE Emp_id = " + std::to_string(emp_id_to_delete) + ";";
    return executeNonQuery(sql);
}


// == Training Functions ==
std::vector<TrainingData> ui_fetchAllTrainings() {
    std::vector<TrainingData> trainings; if (hStmt == SQL_NULL_HSTMT) return trainings;
    SQLCHAR sqlQuery[] = "SELECT Training_id, Name FROM Training;";
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLINTEGER db_id; SQLCHAR db_name[256]; SQLLEN lenId=0, lenName=0;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            TrainingData tr; SQLGetData(hStmt, 1, SQL_C_SLONG, &db_id, 0, &lenId); SQLGetData(hStmt, 2, SQL_C_CHAR, db_name, sizeof(db_name), &lenName);
            tr.training_id = (lenId != SQL_NULL_DATA) ? db_id : 0;
            tr.name = (lenName != SQL_NULL_DATA && lenName > 0) ? std::string(reinterpret_cast<const char*>(db_name), lenName) : "";
            trainings.push_back(tr);
        } SQLCloseCursor(hStmt);
    } else { checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllTrainings"); SQLCloseCursor(hStmt); }
    return trainings;
}
bool ui_insertTraining(const TrainingData& training) {
    if (training.name.empty()) { statusMessage = "Error: Training Name cannot be empty."; return false; }
    std::string sql = "INSERT INTO Training (Name) VALUES ('" + sanitizeSqlString(training.name) + "');";
    return executeNonQuery(sql);
}
bool ui_deleteTraining(int training_id) {
    if (training_id <= 0) {statusMessage = "Error: Invalid Training ID for deletion."; return false; }
    std::string sql = "DELETE FROM Training WHERE Training_id = " + std::to_string(training_id) + ";";
    return executeNonQuery(sql);
}
bool ui_updateTraining(const TrainingData& training) {
    if (training.training_id <= 0 || training.name.empty()) { statusMessage = "Error: Training ID and Name required for update."; return false; }
    std::string sql = "UPDATE Training SET Name = '" + sanitizeSqlString(training.name) + "' WHERE Training_id = " + std::to_string(training.training_id) + ";";
    return executeNonQuery(sql);
}

// == Worker Functions ==
std::vector<WorkerData> ui_fetchAllWorkers() {
    std::vector<WorkerData> workers;
    if (hStmt == SQL_NULL_HSTMT) { std::cerr << "ui_fetchAllWorkers Error: Statement handle is null." << std::endl; return workers; }
    SQLCHAR sqlQuery[] = "SELECT W.Worker_id, E.Name, W.WorkerType, W.Age "
                         "FROM Worker W JOIN Employee E ON W.Worker_id = E.Emp_id;";
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLINTEGER db_worker_id_val, db_age_val;
        SQLCHAR db_emp_name_val[256], db_worker_type_val[256];
        SQLLEN lenId = 0, lenName = 0, lenType = 0, lenAge = 0;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            WorkerData w;
            SQLGetData(hStmt, 1, SQL_C_SLONG, &db_worker_id_val, 0, &lenId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, db_emp_name_val, sizeof(db_emp_name_val), &lenName);
            SQLGetData(hStmt, 3, SQL_C_CHAR, db_worker_type_val, sizeof(db_worker_type_val), &lenType);
            SQLGetData(hStmt, 4, SQL_C_SLONG, &db_age_val, 0, &lenAge);
            w.worker_id = (lenId != SQL_NULL_DATA) ? db_worker_id_val : 0;
            w.employeeName = (lenName != SQL_NULL_DATA && lenName > 0) ? std::string(reinterpret_cast<const char*>(db_emp_name_val), lenName) : "";
            w.workerType = (lenType != SQL_NULL_DATA && lenType > 0) ? std::string(reinterpret_cast<const char*>(db_worker_type_val), lenType) : "";
            w.age = (lenAge != SQL_NULL_DATA) ? db_age_val : 0;
            workers.push_back(w);
        }
        SQLCloseCursor(hStmt);
    } else {
        checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllWorkers");
        SQLCloseCursor(hStmt);
    }
    return workers;
}
bool ui_insertWorker(const WorkerData& worker_data) {
    if (worker_data.worker_id <= 0 || worker_data.workerType.empty()) {
        statusMessage = "Error: Valid Employee ID and Worker Type are required for Worker.";
        return false;
    }   
    std::string sql = "INSERT INTO Worker (Worker_id, WorkerType, Age) VALUES (" +
                      std::to_string(worker_data.worker_id) + ", '" +
                      sanitizeSqlString(worker_data.workerType) + "', " +
                      std::to_string(worker_data.age) + ");";
    return executeNonQuery(sql);
}
bool ui_deleteWorker(int worker_id_to_delete) {
    if (worker_id_to_delete <= 0) {
        statusMessage = "Error: Invalid Worker ID for deletion.";
        return false;
    }
    std::string sql = "DELETE FROM Worker WHERE Worker_id = " + std::to_string(worker_id_to_delete) + ";";
    return executeNonQuery(sql);
}
bool ui_updateWorker(const WorkerData& worker_data) {
    if (worker_data.worker_id <= 0) {
        statusMessage = "Error: Invalid Worker ID for update.";
        return false;
    }
    std::vector<std::string> set_clauses;
    if (!worker_data.workerType.empty()) {
         set_clauses.push_back("WorkerType = '" + sanitizeSqlString(worker_data.workerType) + "'");
    }
    set_clauses.push_back("Age = " + std::to_string(worker_data.age)); // Age is always updated from buffer

    if (set_clauses.empty()) { // Should not happen if age is always included
         statusMessage = "No new data fields provided to update worker " + std::to_string(worker_data.worker_id);
         return false;
    }
    std::string sql = "UPDATE Worker SET ";
    for (size_t i = 0; i < set_clauses.size(); ++i) {
        sql += set_clauses[i] + (i < set_clauses.size() - 1 ? ", " : "");
    }
    sql += " WHERE Worker_id = " + std::to_string(worker_data.worker_id) + ";";
    return executeNonQuery(sql);
}



// == Trainer Functions ==
std::vector<TrainerData> ui_fetchAllTrainers() {
    std::vector<TrainerData> trainers;
    if (hStmt == SQL_NULL_HSTMT) { std::cerr << "ui_fetchAllTrainers Error: Statement handle is null." << std::endl; return trainers; }
    SQLCHAR sqlQuery[] = "SELECT T.Trainer_id, E.Name, T.Age, T.Phone "
                         "FROM Trainer T JOIN Employee E ON T.Trainer_id = E.Emp_id;";
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLINTEGER db_trainer_id_val, db_age_val;
        SQLCHAR db_emp_name_val[256], db_phone_val[20];
        SQLLEN lenId = 0, lenName = 0, lenAge = 0, lenPhone = 0;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            TrainerData t;
            SQLGetData(hStmt, 1, SQL_C_SLONG, &db_trainer_id_val, 0, &lenId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, db_emp_name_val, sizeof(db_emp_name_val), &lenName);
            SQLGetData(hStmt, 3, SQL_C_SLONG, &db_age_val, 0, &lenAge);
            SQLGetData(hStmt, 4, SQL_C_CHAR, db_phone_val, sizeof(db_phone_val), &lenPhone);
            t.trainer_id = (lenId != SQL_NULL_DATA) ? db_trainer_id_val : 0;
            t.employeeName = (lenName != SQL_NULL_DATA && lenName > 0) ? std::string(reinterpret_cast<const char*>(db_emp_name_val), lenName) : "";
            t.age = (lenAge != SQL_NULL_DATA) ? db_age_val : 0;
            t.phone = (lenPhone != SQL_NULL_DATA && lenPhone > 0) ? std::string(reinterpret_cast<const char*>(db_phone_val), lenPhone) : "";
            trainers.push_back(t);
        }
        SQLCloseCursor(hStmt);
    } else {
        checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllTrainers");
        SQLCloseCursor(hStmt);
    }
    return trainers;
}

bool ui_insertTrainer(const TrainerData& trainer_data) { 
    if (trainer_data.trainer_id <= 0) { 
        statusMessage = "Error: Valid Employee ID is required to make them a Trainer."; 
        return false; 
    }
    // Add validation for age and phone if necessary (e.g., age > 0)
    std::string sql = "INSERT INTO Trainer (Trainer_id, Age, Phone) VALUES (" +
                      std::to_string(trainer_data.trainer_id) + ", " +
                      std::to_string(trainer_data.age) + ", '" +
                      sanitizeSqlString(trainer_data.phone) + "');";
    return executeNonQuery(sql);
}

bool ui_deleteTrainer(int trainer_id_to_delete) { // trainer_id is Emp_id
    if (trainer_id_to_delete <= 0) {
        statusMessage = "Error: Invalid Trainer ID for deletion.";
        return false;
    }
    // Check dependencies in Training_Trainer first
    std::string checkTTSql = "SELECT COUNT(*) FROM Training_Trainer WHERE Trainer_id = " + std::to_string(trainer_id_to_delete) + ";";
    SQLINTEGER count = 0; SQLLEN lenCount = 0;
    std::vector<SQLCHAR> ttQueryBuffer(checkTTSql.begin(), checkTTSql.end()); ttQueryBuffer.push_back('\0');
    if (SQLExecDirect(hStmt, ttQueryBuffer.data(), SQL_NTS) == SQL_SUCCESS) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) { SQLGetData(hStmt, 1, SQL_C_SLONG, &count, 0, &lenCount); }
        SQLCloseCursor(hStmt);
        if (count > 0) {
            statusMessage = "Error: Cannot delete Trainer. They are assigned to " + std::to_string(count) + " service(s). Remove assignments first.";
            return false;
        }
    } else {
         checkSQLError(hStmt, SQL_HANDLE_STMT, SQLExecDirect(hStmt, ttQueryBuffer.data(), SQL_NTS), "ui_deleteTrainer - check Training_Trainer");
         SQLCloseCursor(hStmt); return false;
    }

    std::string sql = "DELETE FROM Trainer WHERE Trainer_id = " + std::to_string(trainer_id_to_delete) + ";";
    return executeNonQuery(sql);
}

bool ui_updateTrainer(const TrainerData& trainer_data) {
    if (trainer_data.trainer_id <= 0) {
        statusMessage = "Error: Invalid Trainer ID for update.";
        return false;
    }
    std::vector<std::string> set_clauses;
    bool has_changes = false; 
    // For Trainer, Age and Phone are the updatable fields in the Trainer table itself.
    // Name/CNIC/Address are part of the Employee table.
    
    // Assuming age can be updated. Add validation if age must be > 0 etc.
    set_clauses.push_back("Age = " + std::to_string(trainer_data.age));
    has_changes = true; 

    if (!trainer_data.phone.empty()) { // Only update phone if new value is provided
        set_clauses.push_back("Phone = '" + sanitizeSqlString(trainer_data.phone) + "'");
        has_changes = true;
    }
    
    if (!has_changes && trainer_data.phone.empty() && trainer_data.age == 0 ) { // Check if any actual data to update
         statusMessage = "No new data fields provided to update trainer " + std::to_string(trainer_data.trainer_id);
         return false;
    }

    std::string sql = "UPDATE Trainer SET ";
    for (size_t i = 0; i < set_clauses.size(); ++i) {
        sql += set_clauses[i] + (i < set_clauses.size() - 1 ? ", " : "");
    }
    sql += " WHERE Trainer_id = " + std::to_string(trainer_data.trainer_id) + ";";
    return executeNonQuery(sql);
}


// == Training_Trainer (Service by Trainer) Functions ==
std::vector<TrainingTrainerData> ui_fetchAllServiceByTrainer() {
    std::vector<TrainingTrainerData> assignments; if (hStmt == SQL_NULL_HSTMT) return assignments;
    SQLCHAR sqlQuery[] = "SELECT TT.TT_id, TT.Training_id, T.Name AS TrainingName, TT.Trainer_id, E.Name AS TrainerName FROM Training_Trainer TT JOIN Training T ON TT.Training_id = T.Training_id JOIN Employee E ON TT.Trainer_id = E.Emp_id;"; 
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLINTEGER db_tt_id, db_training_id, db_trainer_id; SQLCHAR db_training_name[256], db_trainer_name[256];
        SQLLEN lenTTId=0, lenTrainId=0, lenTrainName=0, lenTrainerId=0, lenTrainerName=0;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            TrainingTrainerData tta;
            SQLGetData(hStmt, 1, SQL_C_SLONG, &db_tt_id, 0, &lenTTId); SQLGetData(hStmt, 2, SQL_C_SLONG, &db_training_id, 0, &lenTrainId);
            SQLGetData(hStmt, 3, SQL_C_CHAR, db_training_name, sizeof(db_training_name), &lenTrainName); SQLGetData(hStmt, 4, SQL_C_SLONG, &db_trainer_id, 0, &lenTrainerId);
            SQLGetData(hStmt, 5, SQL_C_CHAR, db_trainer_name, sizeof(db_trainer_name), &lenTrainerName);
            tta.tt_id = (lenTTId != SQL_NULL_DATA) ? db_tt_id : 0; tta.training_id = (lenTrainId != SQL_NULL_DATA) ? db_training_id : 0;
            tta.trainingName = (lenTrainName != SQL_NULL_DATA && lenTrainName > 0) ? std::string(reinterpret_cast<const char*>(db_training_name), lenTrainName) : "";
            tta.trainer_id = (lenTrainerId != SQL_NULL_DATA) ? db_trainer_id : 0;
            tta.trainerName = (lenTrainerName != SQL_NULL_DATA && lenTrainerName > 0) ? std::string(reinterpret_cast<const char*>(db_trainer_name), lenTrainerName) : "";
            assignments.push_back(tta);
        } SQLCloseCursor(hStmt);
    } else { checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllServiceByTrainer"); SQLCloseCursor(hStmt); }
    return assignments;
}
bool ui_insertServiceByTrainer(int training_id, int trainer_id) {
    if (training_id <= 0 || trainer_id <= 0) { statusMessage = "Error: Valid Training ID and Trainer ID for assignment."; return false; }
    std::string sql = "INSERT INTO Training_Trainer (Training_id, Trainer_id) VALUES (" + std::to_string(training_id) + ", " + std::to_string(trainer_id) + ");";
    return executeNonQuery(sql);
}
bool ui_deleteServiceByTrainer(int tt_id_to_delete) {
    if (tt_id_to_delete <= 0) { statusMessage = "Error: Invalid Service Assignment ID (TT_id) for deletion."; return false; }
    std::string sql = "DELETE FROM Training_Trainer WHERE TT_id = " + std::to_string(tt_id_to_delete) + ";";
    return executeNonQuery(sql);
}

// ... (after last ui_... function block, e.g., ui_deleteServiceByTrainer)

// == Animal Functions ==
std::vector<AnimalData> ui_fetchAllAnimals() {
    std::vector<AnimalData> animals;
    if (hStmt == SQL_NULL_HSTMT) { statusMessage = "DB Error: Cannot fetch animals (stmt null)."; return animals; }
    SQLCHAR sqlQuery[] = "SELECT A.Animal_id, A.AnimalType, A.CNIC, C.Name AS OwnerName, "
                         "A.Name AS AnimalName, A.Age, "
                         "CONVERT(varchar, A.StartDate, 23) AS StartDate, " 
                         "CONVERT(varchar, A.ReturnDate, 23) AS ReturnDate "
                         "FROM Animal A JOIN Customer C ON A.CNIC = C.CNIC ORDER BY A.Animal_id;";
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLINTEGER db_animal_id_val, db_age_val;
        SQLCHAR db_animal_type_val[256], db_cnic_val[21], db_owner_name_val[256], db_animal_name_val[256];
        SQLCHAR db_start_date_val[11], db_return_date_val[11]; 
        SQLLEN lenId=0, lenType=0, lenCnic=0, lenOwnerName=0, lenAnimalName=0, lenAge=0, lenStart=0, lenReturn=0;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            AnimalData an;
            SQLGetData(hStmt, 1, SQL_C_SLONG, &db_animal_id_val, 0, &lenId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, db_animal_type_val, sizeof(db_animal_type_val), &lenType);
            SQLGetData(hStmt, 3, SQL_C_CHAR, db_cnic_val, sizeof(db_cnic_val), &lenCnic);
            SQLGetData(hStmt, 4, SQL_C_CHAR, db_owner_name_val, sizeof(db_owner_name_val), &lenOwnerName);
            SQLGetData(hStmt, 5, SQL_C_CHAR, db_animal_name_val, sizeof(db_animal_name_val), &lenAnimalName);
            SQLGetData(hStmt, 6, SQL_C_SLONG, &db_age_val, 0, &lenAge);
            SQLGetData(hStmt, 7, SQL_C_CHAR, db_start_date_val, sizeof(db_start_date_val), &lenStart);
            SQLGetData(hStmt, 8, SQL_C_CHAR, db_return_date_val, sizeof(db_return_date_val), &lenReturn);

            an.animal_id = (lenId != SQL_NULL_DATA) ? db_animal_id_val : 0;
            an.animalType = (lenType != SQL_NULL_DATA && lenType > 0) ? std::string(reinterpret_cast<const char*>(db_animal_type_val), lenType) : "";
            an.cnic_owner = (lenCnic != SQL_NULL_DATA && lenCnic > 0) ? std::string(reinterpret_cast<const char*>(db_cnic_val), lenCnic) : "";
            an.ownerNameDisplay = (lenOwnerName != SQL_NULL_DATA && lenOwnerName > 0) ? std::string(reinterpret_cast<const char*>(db_owner_name_val), lenOwnerName) : "";
            an.animalName = (lenAnimalName != SQL_NULL_DATA && lenAnimalName > 0) ? std::string(reinterpret_cast<const char*>(db_animal_name_val), lenAnimalName) : "";
            an.age = (lenAge != SQL_NULL_DATA) ? db_age_val : 0;
            an.startDate = (lenStart != SQL_NULL_DATA && lenStart > 0) ? std::string(reinterpret_cast<const char*>(db_start_date_val), lenStart) : "";
            an.returnDate = (lenReturn != SQL_NULL_DATA && lenReturn > 0) ? std::string(reinterpret_cast<const char*>(db_return_date_val), lenReturn) : "";
            animals.push_back(an);
        }
        SQLCloseCursor(hStmt);
    } else {
        checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllAnimals");
        SQLCloseCursor(hStmt); statusMessage = "Error fetching animals.";
    }
    return animals;
}

bool ui_insertAnimal(const AnimalData& animal) {
    if (animal.animalType.empty() || animal.cnic_owner.empty() || animal.animalName.empty() || animal.startDate.empty()) {
        statusMessage = "Error: Animal Type, Owner CNIC, Animal Name, and Start Date are required for Animal.";
        return false;
    }
    if (animal.startDate.length() != 10 || animal.startDate[4] != '-' || animal.startDate[7] != '-') {
        statusMessage = "Error: Start Date format must be YYYY-MM-DD.";
        return false;
    }
    if (!animal.returnDate.empty() && (animal.returnDate.length() != 10 || animal.returnDate[4] != '-' || animal.returnDate[7] != '-')) {
        statusMessage = "Error: Return Date format must be YYYY-MM-DD or empty.";
        return false;
    }
    std::string returnDateSqlValue = animal.returnDate.empty() ? "NULL" : ("'" + sanitizeSqlString(animal.returnDate) + "'");
    std::string sql="INSERT INTO Animal (AnimalType,CNIC,Name,Age,StartDate,ReturnDate) VALUES ('"+sanitizeSqlString(toLower(animal.animalType))+"','"+sanitizeSqlString(animal.cnic_owner)+"','"+sanitizeSqlString(animal.animalName)+"',"+std::to_string(animal.age)+",'"+sanitizeSqlString(animal.startDate)+"',"+returnDateSqlValue+");";
    if(!executeNonQuery(sql)) { statusMessage = "Database error inserting animal."; return false;} return true;
}

bool ui_deleteAnimal(int animal_id_to_delete) {
    if(animal_id_to_delete<=0){statusMessage="Error: Invalid Animal ID for deletion."; return false;}
    std::string checkATSSql="SELECT COUNT(*) FROM Animal_Training_Trainer WHERE Animal_id = "+std::to_string(animal_id_to_delete)+";";
    SQLINTEGER count=0; SQLLEN lenCount=0;
    std::vector<SQLCHAR> atsQueryBuffer(checkATSSql.begin(), checkATSSql.end()); atsQueryBuffer.push_back('\0');
    SQLRETURN rc_check = SQLExecDirect(hStmt, atsQueryBuffer.data(), SQL_NTS);
    if (SQL_SUCCEEDED(rc_check)) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) { SQLGetData(hStmt, 1, SQL_C_SLONG, &count, 0, &lenCount); }
        SQLCloseCursor(hStmt);
        if (count > 0) {
            statusMessage = "Error: Cannot delete Animal. It is scheduled for " + std::to_string(count) + " training(s). Remove schedules first.";
            return false;
        }
    } else {
         checkSQLError(hStmt, SQL_HANDLE_STMT, rc_check, "ui_deleteAnimal - check Animal_Training_Trainer"); 
         SQLCloseCursor(hStmt); return false;
    }
    std::string sql = "DELETE FROM Animal WHERE Animal_id = " + std::to_string(animal_id_to_delete) + ";";
    if(!executeNonQuery(sql)) { statusMessage = "Database error deleting animal."; return false;} return true;
}

bool ui_updateAnimal(const AnimalData& animal_data_to_update) {
    if (animal_data_to_update.animal_id <= 0) { statusMessage = "Error: Invalid Animal ID for update."; return false; }
    std::vector<std::string> set_clauses;
    bool has_actual_changes = false; 

    if (!animal_data_to_update.animalType.empty()) { set_clauses.push_back("AnimalType = '" + sanitizeSqlString(toLower(animal_data_to_update.animalType)) + "'"); has_actual_changes = true; }
    if (!animal_data_to_update.cnic_owner.empty()) { set_clauses.push_back("CNIC = '" + sanitizeSqlString(animal_data_to_update.cnic_owner) + "'"); has_actual_changes = true; }
    if (!animal_data_to_update.animalName.empty()) { set_clauses.push_back("Name = '" + sanitizeSqlString(animal_data_to_update.animalName) + "'"); has_actual_changes = true; }
    if (animal_data_to_update.age >= 0) { set_clauses.push_back("Age = " + std::to_string(animal_data_to_update.age)); has_actual_changes = true; }
    if (!animal_data_to_update.startDate.empty()) { 
        if (animal_data_to_update.startDate.length() != 10 || animal_data_to_update.startDate[4] != '-' || animal_data_to_update.startDate[7] != '-') { statusMessage = "Error: Update Start Date format YYYY-MM-DD."; return false; }
        set_clauses.push_back("StartDate = '" + sanitizeSqlString(animal_data_to_update.startDate) + "'"); has_actual_changes = true; 
    }
    if (toLower(animal_data_to_update.returnDate) == "null" || animal_data_to_update.returnDate.empty()) {
        set_clauses.push_back("ReturnDate = NULL"); has_actual_changes = true;
    } else if (!animal_data_to_update.returnDate.empty()) {
        if (animal_data_to_update.returnDate.length() != 10 || animal_data_to_update.returnDate[4] != '-' || animal_data_to_update.returnDate[7] != '-') { statusMessage = "Error: Update Return Date format YYYY-MM-DD or empty/NULL."; return false; }
        set_clauses.push_back("ReturnDate = '" + sanitizeSqlString(animal_data_to_update.returnDate) + "'"); has_actual_changes = true;
    }

    if (!has_actual_changes) { statusMessage = "No new data provided to update animal " + std::to_string(animal_data_to_update.animal_id); return false; }
    std::string sql = "UPDATE Animal SET ";
    for (size_t i = 0; i < set_clauses.size(); ++i) { sql += set_clauses[i] + (i < set_clauses.size() - 1 ? ", " : ""); }
    sql += " WHERE Animal_id = " + std::to_string(animal_data_to_update.animal_id) + ";";
    if(!executeNonQuery(sql)) { statusMessage = "Database error updating animal."; return false;} return true;
}

// == Animal Training Schedule (Animal_Training_Trainer) Functions ==
std::vector<AnimalTrainingScheduleData> ui_fetchAllAnimalSchedules() {
    std::vector<AnimalTrainingScheduleData> schedules; if (hStmt == SQL_NULL_HSTMT) { statusMessage = "DB Error: Cannot fetch schedules."; return schedules;}
    SQLCHAR sqlQuery[] = "SELECT ATS.Animal_id, A.Name AS AnimalName, ATS.TT_id, TR.Name AS TrainingName, E.Name AS TrainerName FROM Animal_Training_Trainer ATS JOIN Animal A ON ATS.Animal_id = A.Animal_id JOIN Training_Trainer TT ON ATS.TT_id = TT.TT_id JOIN Training TR ON TT.Training_id = TR.Training_id JOIN Employee E ON TT.Trainer_id = E.Emp_id ORDER BY ATS.Animal_id, ATS.TT_id;";
    SQLRETURN retcode = SQLExecDirect(hStmt, sqlQuery, SQL_NTS);
    if (SQL_SUCCEEDED(retcode)) {
        SQLINTEGER db_animal_id, db_tt_id; SQLCHAR db_animal_name[256], db_training_name[256], db_trainer_name[256];
        SQLLEN lenAnimId=0,lenAnimName=0,lenTTId=0,lenTrainName=0,lenTrainerName=0;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            AnimalTrainingScheduleData ats;
            SQLGetData(hStmt,1,SQL_C_SLONG,&db_animal_id,0,&lenAnimId); SQLGetData(hStmt,2,SQL_C_CHAR,db_animal_name,sizeof(db_animal_name),&lenAnimName);
            SQLGetData(hStmt,3,SQL_C_SLONG,&db_tt_id,0,&lenTTId); SQLGetData(hStmt,4,SQL_C_CHAR,db_training_name,sizeof(db_training_name),&lenTrainName);
            SQLGetData(hStmt,5,SQL_C_CHAR,db_trainer_name,sizeof(db_trainer_name),&lenTrainerName);
            ats.animal_id=(lenAnimId!=SQL_NULL_DATA)?db_animal_id:0;
            ats.animalNameDisplay=(lenAnimName!=SQL_NULL_DATA&&lenAnimName>0)?std::string(reinterpret_cast<const char*>(db_animal_name),lenAnimName):"";
            ats.tt_id=(lenTTId!=SQL_NULL_DATA)?db_tt_id:0;
            ats.trainingNameDisplay=(lenTrainName!=SQL_NULL_DATA&&lenTrainName>0)?std::string(reinterpret_cast<const char*>(db_training_name),lenTrainName):"";
            ats.trainerNameDisplay=(lenTrainerName!=SQL_NULL_DATA&&lenTrainerName>0)?std::string(reinterpret_cast<const char*>(db_trainer_name),lenTrainerName):"";
            schedules.push_back(ats);
        } SQLCloseCursor(hStmt);
    } else { checkSQLError(hStmt, SQL_HANDLE_STMT, retcode, "ui_fetchAllAnimalSchedules"); SQLCloseCursor(hStmt); statusMessage = "Error fetching animal schedules.";}
    return schedules;
}
bool ui_insertAnimalSchedule(int animal_id, int tt_id) {
    if (animal_id <= 0 || tt_id <= 0) { statusMessage = "Error: Valid Animal ID and Service (TT_id) are required for schedule."; return false; }
    std::string sql = "INSERT INTO Animal_Training_Trainer (Animal_id, TT_id) VALUES (" + std::to_string(animal_id) + ", " + std::to_string(tt_id) + ");";
    if(!executeNonQuery(sql)) { statusMessage = "Database error inserting animal schedule."; return false;} return true;
}
bool ui_deleteAnimalSchedule(int animal_id, int tt_id) {
    if (animal_id <= 0 || tt_id <= 0) { statusMessage = "Error: Valid Animal ID and Service (TT_id) for deletion."; return false; }
    std::string sql = "DELETE FROM Animal_Training_Trainer WHERE Animal_id = " + std::to_string(animal_id) + " AND TT_id = " + std::to_string(tt_id) + ";";
    if(!executeNonQuery(sql)) { statusMessage = "Database error deleting animal schedule."; return false;} return true;
}

// TODO: IMPLEMENT UI-SPECIFIC DATABASE FUNCTIONS FOR Trainer, Animal, etc.


static void glfw_error_callback(int error, const char* description) { fprintf(stderr, "Glfw Error %d: %s\n", error, description); }

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 768, "Animal Training DB Management", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window); glfwSwapInterval(1);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { fprintf(stderr, "Failed to initialize GLAD\n"); glfwDestroyWindow(window); glfwTerminate(); return 1; }

    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark(); ImGui_ImplGlfw_InitForOpenGL(window, true); ImGui_ImplOpenGL3_Init(glsl_version);

    if (!connectDB()) {
        statusMessage = "FATAL: DB Connect fail. Cannot start GUI.";
    } else {
        statusMessage = "Welcome! Database connected.";
    }

    ImVec4 clear_color = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);

    // --- Main Loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();

        ImGui::Begin("Animal Training Database Management System", nullptr, ImGuiWindowFlags_MenuBar);
        ImGui::TextUnformatted(statusMessage.c_str()); ImGui::Separator();

        // == CUSTOMER MANAGEMENT SECTION ==
        if (ImGui::CollapsingHeader("Manage Customers", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Load/Refresh All Customers##Cust")) {
                customers_list_display = ui_fetchAllCustomers();
                statusMessage = "Loaded " + std::to_string(customers_list_display.size()) + " customers.";
                cust_selected_cnic_for_ops = ""; cust_selected_idx = -1;
                cust_update_name_buf[0] = '\0'; cust_update_address_buf[0] = '\0'; cust_update_phone_buf[0] = '\0';
            }
            ImGui::InputText("Search CNIC##CustSearch", cust_search_cnic_buf, IM_ARRAYSIZE(cust_search_cnic_buf)); ImGui::SameLine();
            if (ImGui::Button("Search##CustSearchBtn")) {
                customers_list_display = ui_searchCustomerByCNIC(cust_search_cnic_buf);
                if (!customers_list_display.empty()) {
                    statusMessage = "Found " + std::to_string(customers_list_display.size()) + " customer(s).";
                    cust_selected_cnic_for_ops = customers_list_display[0].cnic; cust_selected_idx = 0;
                    strncpy(cust_update_name_buf, customers_list_display[0].name.c_str(), sizeof(cust_update_name_buf) -1); cust_update_name_buf[sizeof(cust_update_name_buf)-1]=0;
                    strncpy(cust_update_address_buf, customers_list_display[0].address.c_str(), sizeof(cust_update_address_buf) -1); cust_update_address_buf[sizeof(cust_update_address_buf)-1]=0;
                    strncpy(cust_update_phone_buf, customers_list_display[0].phoneNumber.c_str(), sizeof(cust_update_phone_buf) -1); cust_update_phone_buf[sizeof(cust_update_phone_buf)-1]=0;
                } else {
                    statusMessage = "Customer with CNIC '" + std::string(cust_search_cnic_buf) + "' not found.";
                    cust_selected_cnic_for_ops = ""; cust_selected_idx = -1;
                    cust_update_name_buf[0] = '\0'; cust_update_address_buf[0] = '\0'; cust_update_phone_buf[0] = '\0';
                }
            } ImGui::Separator();
            ImGui::Text("Customer List:"); ImGui::BeginChild("CustomerListChild", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("CustomersTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY )) {
                ImGui::TableSetupColumn("CNIC", ImGuiTableColumnFlags_WidthFixed, 120.0f); ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch); ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch); ImGui::TableSetupColumn("Phone", ImGuiTableColumnFlags_WidthFixed, 100.0f); ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 60.0f); ImGui::TableHeadersRow();
                for (int i = 0; i < customers_list_display.size(); ++i) {
                    const auto& cust = customers_list_display[i]; ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(cust.cnic.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(cust.name.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(cust.address.c_str());
                    ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(cust.phoneNumber.c_str());
                    ImGui::TableSetColumnIndex(4); ImGui::PushID(i + 1000); // Unique ID
                    if (ImGui::SmallButton("Select")) {
                        cust_selected_cnic_for_ops = cust.cnic; cust_selected_idx = i;
                        strncpy(cust_update_name_buf, cust.name.c_str(), sizeof(cust_update_name_buf) -1); cust_update_name_buf[sizeof(cust_update_name_buf)-1] = 0;
                        strncpy(cust_update_address_buf, cust.address.c_str(), sizeof(cust_update_address_buf) -1); cust_update_address_buf[sizeof(cust_update_address_buf)-1] = 0;
                        strncpy(cust_update_phone_buf, cust.phoneNumber.c_str(), sizeof(cust_update_phone_buf) -1); cust_update_phone_buf[sizeof(cust_update_phone_buf)-1] = 0;
                        statusMessage = "Selected for ops: " + cust.cnic;
                    } ImGui::PopID();
                } ImGui::EndTable();
            } ImGui::EndChild(); ImGui::Separator();
            if(ImGui::TreeNodeEx("Add New Customer", ImGuiTreeNodeFlags_Framed)){
                ImGui::InputText("CNIC##AddCust", cust_add_cnic_buf, IM_ARRAYSIZE(cust_add_cnic_buf)); ImGui::InputText("Name##AddCust", cust_add_name_buf, IM_ARRAYSIZE(cust_add_name_buf)); ImGui::InputText("Address##AddCust", cust_add_address_buf, IM_ARRAYSIZE(cust_add_address_buf)); ImGui::InputText("Phone##AddCust", cust_add_phone_buf, IM_ARRAYSIZE(cust_add_phone_buf));
                if (ImGui::Button("Submit New Customer##AddCustBtn")) {
                    CustomerData newCust = {cust_add_cnic_buf, cust_add_name_buf, cust_add_address_buf, cust_add_phone_buf};
                    if (ui_insertCustomer(newCust)) {
                        statusMessage = "Customer '" + newCust.name + "' added!";
                        cust_add_cnic_buf[0] = '\0'; cust_add_name_buf[0] = '\0'; cust_add_address_buf[0] = '\0'; cust_add_phone_buf[0] = '\0';
                        customers_list_display = ui_fetchAllCustomers();
                    }
                }
                ImGui::TreePop();
            } ImGui::Separator();
            if (!cust_selected_cnic_for_ops.empty()) {
                if(ImGui::TreeNodeEx("Update/Delete Selected Customer", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)){
                    ImGui::Text("Operating on CNIC: %s", cust_selected_cnic_for_ops.c_str());
                    ImGui::InputText("New Name##UpdateCust", cust_update_name_buf, IM_ARRAYSIZE(cust_update_name_buf));
                    ImGui::InputText("New Address##UpdateCust", cust_update_address_buf, IM_ARRAYSIZE(cust_update_address_buf));
                    ImGui::InputText("New Phone##UpdateCust", cust_update_phone_buf, IM_ARRAYSIZE(cust_update_phone_buf));
                    if (ImGui::Button("Update This Customer##UpdateCustBtn")) {
                        CustomerData data_to_update_with = {"", cust_update_name_buf, cust_update_address_buf, cust_update_phone_buf};
                        // data_to_update_with.cnic is not used by ui_updateCustomer, only the first param is key
                        if (strlen(cust_update_name_buf) > 0 || strlen(cust_update_address_buf) > 0 || strlen(cust_update_phone_buf) > 0) {
                            if (ui_updateCustomer(cust_selected_cnic_for_ops, data_to_update_with)) {
                                statusMessage = "Customer " + cust_selected_cnic_for_ops + " updated.";
                                customers_list_display = ui_fetchAllCustomers();
                                cust_selected_cnic_for_ops = ""; cust_selected_idx = -1;
                                cust_update_name_buf[0] = '\0'; cust_update_address_buf[0] = '\0'; cust_update_phone_buf[0] = '\0';
                            }
                        } else {
                            statusMessage = "No new data entered to update for " + cust_selected_cnic_for_ops;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete This Customer##DeleteCustBtn")) {
                        if (ui_deleteCustomer(cust_selected_cnic_for_ops)) {
                            statusMessage = "Customer " + cust_selected_cnic_for_ops + " deleted.";
                            customers_list_display = ui_fetchAllCustomers();
                            cust_selected_cnic_for_ops = ""; cust_selected_idx = -1;
                            cust_update_name_buf[0] = '\0'; cust_update_address_buf[0] = '\0'; cust_update_phone_buf[0] = '\0';
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }
        if (ImGui::CollapsingHeader("Manage Animals")) {
            if (ImGui::Button("Load All Animals##AnimalLoad")) {
                animals_list_display = ui_fetchAllAnimals();
                if(available_allowed_types_for_animal.empty()) available_allowed_types_for_animal = ui_fetchAllAllowedAnimals();
                if(available_customers_for_animal.empty()) available_customers_for_animal = ui_fetchAllCustomers();
                statusMessage = "Loaded " + std::to_string(animals_list_display.size()) + " animals.";
                animal_selected_id_for_ops = 0; animal_selected_name_for_display = "";
                animal_update_name_buf[0] = '\0'; animal_update_age_val = 0; 
                animal_update_start_date_buf[0] = '\0'; animal_update_return_date_buf[0] = '\0';
                animal_update_selected_allowed_type = ""; animal_update_selected_customer_cnic = "";
            }

            ImGui::Text("Animal List:");
            ImGui::BeginChild("AnimalListChild", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("AnimalsTableDisplay", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY )) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Owner (CNIC)", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Age", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Start Date", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Return Date", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableHeadersRow();
                for (int i=0; i < animals_list_display.size(); ++i) {
                    const auto& an = animals_list_display[i];
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%d", an.animal_id);
                    ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(an.animalName.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(an.animalType.c_str());
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%s [%s]", an.ownerNameDisplay.c_str(), an.cnic_owner.c_str());
                    ImGui::TableSetColumnIndex(4); ImGui::Text("%d", an.age);
                    ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(an.startDate.c_str());
                    ImGui::TableSetColumnIndex(6); ImGui::TextUnformatted(an.returnDate.c_str());
                    ImGui::TableSetColumnIndex(7); 
                    ImGui::PushID(an.animal_id + 11000); 
                    if(ImGui::SmallButton("Select")){
                        animal_selected_id_for_ops = an.animal_id;
                        animal_selected_name_for_display = an.animalName;
                        strncpy(animal_update_name_buf, an.animalName.c_str(), sizeof(animal_update_name_buf)-1); animal_update_name_buf[sizeof(animal_update_name_buf)-1]=0;
                        animal_update_age_val = an.age;
                        strncpy(animal_update_start_date_buf, an.startDate.c_str(), sizeof(animal_update_start_date_buf)-1); animal_update_start_date_buf[sizeof(animal_update_start_date_buf)-1]=0;
                        strncpy(animal_update_return_date_buf, an.returnDate.c_str(), sizeof(animal_update_return_date_buf)-1); animal_update_return_date_buf[sizeof(animal_update_return_date_buf)-1]=0;
                        animal_update_selected_allowed_type = an.animalType;
                        animal_update_selected_customer_cnic = an.cnic_owner;
                        statusMessage = "Selected Animal: " + an.animalName;
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::Separator();

            if(ImGui::TreeNodeEx("Add New Animal", ImGuiTreeNodeFlags_Framed)){
                if(available_allowed_types_for_animal.empty() && ImGui::IsWindowAppearing()) available_allowed_types_for_animal = ui_fetchAllAllowedAnimals();
                const char* preview_type_animal_add = animal_add_selected_allowed_type.empty() ? "Select Animal Type" : animal_add_selected_allowed_type.c_str();
                if (ImGui::BeginCombo("Animal Type##AddAnimalType", preview_type_animal_add)) {
                    for (const auto& at : available_allowed_types_for_animal) {
                        bool is_selected = (animal_add_selected_allowed_type == at.animal_type);
                        if (ImGui::Selectable(at.animal_type.c_str(), is_selected)) animal_add_selected_allowed_type = at.animal_type;
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                if(available_customers_for_animal.empty() && ImGui::IsWindowAppearing()) available_customers_for_animal = ui_fetchAllCustomers();
                const char* preview_owner_animal_add = "Select Owner (Customer)";
                if(!animal_add_selected_customer_cnic.empty()){
                     auto it_cust = std::find_if(available_customers_for_animal.begin(), available_customers_for_animal.end(), 
                                           [&](const CustomerData& c){ return c.cnic == animal_add_selected_customer_cnic; });
                    if(it_cust != available_customers_for_animal.end()) preview_owner_animal_add = (it_cust->name + " [" + it_cust->cnic + "]").c_str();
                }
                if (ImGui::BeginCombo("Owner##AddAnimalOwner", preview_owner_animal_add)) {
                    for (const auto& cust : available_customers_for_animal) {
                        bool is_selected = (animal_add_selected_customer_cnic == cust.cnic);
                        if (ImGui::Selectable((cust.name + " [" + cust.cnic + "] ("+ cust.phoneNumber + ")").c_str(), is_selected)) animal_add_selected_customer_cnic = cust.cnic;
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::InputText("Animal Name##AddAnimalName", animal_add_name_buf, IM_ARRAYSIZE(animal_add_name_buf));
                ImGui::InputInt("Age##AddAnimalAge", &animal_add_age_val);
                ImGui::InputText("Start Date (YYYY-MM-DD)##AddAnimalStart", animal_add_start_date_buf, IM_ARRAYSIZE(animal_add_start_date_buf));
                ImGui::InputText("Return Date (YYYY-MM-DD or empty)##AddAnimalReturn", animal_add_return_date_buf, IM_ARRAYSIZE(animal_add_return_date_buf));
                if (ImGui::Button("Submit New Animal##AddAnimalBtn")) {
                    if (!animal_add_selected_allowed_type.empty() && !animal_add_selected_customer_cnic.empty() && strlen(animal_add_name_buf) > 0 && strlen(animal_add_start_date_buf) > 0) {
                        AnimalData new_an = {0, animal_add_selected_allowed_type, animal_add_selected_customer_cnic, "", animal_add_name_buf, animal_add_age_val, animal_add_start_date_buf, animal_add_return_date_buf};
                        if (ui_insertAnimal(new_an)) {
                            statusMessage = "Animal '" + std::string(animal_add_name_buf) + "' added!";
                            animal_add_name_buf[0]='\0'; animal_add_age_val=0; animal_add_start_date_buf[0]='\0'; animal_add_return_date_buf[0]='\0';
                            animal_add_selected_allowed_type = ""; animal_add_selected_customer_cnic = "";
                            animals_list_display = ui_fetchAllAnimals(); 
                        }
                    } else { statusMessage = "Error: Animal Type, Owner, Name, and Start Date are required."; }
                }
                ImGui::TreePop();
            } 
            ImGui::Separator();
            if (animal_selected_id_for_ops > 0) { 
                if(ImGui::TreeNodeEx("Update/Delete Selected Animal", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)){
                    ImGui::Text("Operating on Animal ID: %d (%s)", animal_selected_id_for_ops, animal_selected_name_for_display.c_str());
                    // Update Form (with dropdowns for FKs)
                    const char* preview_type_animal_upd = animal_update_selected_allowed_type.c_str();
                    if (ImGui::BeginCombo("New Animal Type##UpdateAnimalType", preview_type_animal_upd)) {
                        for (const auto& at : available_allowed_types_for_animal) {
                            bool is_selected = (animal_update_selected_allowed_type == at.animal_type);
                            if (ImGui::Selectable(at.animal_type.c_str(), is_selected)) animal_update_selected_allowed_type = at.animal_type;
                            if (is_selected) ImGui::SetItemDefaultFocus();
                        } ImGui::EndCombo();
                    }
                    const char* preview_owner_animal_upd = "Select New Owner";
                    if(!animal_update_selected_customer_cnic.empty()){
                        auto it_cust_upd = std::find_if(available_customers_for_animal.begin(), available_customers_for_animal.end(), 
                                               [&](const CustomerData& c){ return c.cnic == animal_update_selected_customer_cnic; });
                        if(it_cust_upd != available_customers_for_animal.end()) preview_owner_animal_upd = (it_cust_upd->name + " [" + it_cust_upd->cnic + "]").c_str();
                    }
                    if (ImGui::BeginCombo("New Owner##UpdateAnimalOwner", preview_owner_animal_upd)) {
                        for (const auto& cust : available_customers_for_animal) {
                            bool is_selected = (animal_update_selected_customer_cnic == cust.cnic);
                            if (ImGui::Selectable((cust.name + " [" + cust.cnic + "]").c_str(), is_selected)) animal_update_selected_customer_cnic = cust.cnic;
                            if (is_selected) ImGui::SetItemDefaultFocus();
                        } ImGui::EndCombo();
                    }
                    ImGui::InputText("New Name##UpdateAnimalName", animal_update_name_buf, IM_ARRAYSIZE(animal_update_name_buf));
                    ImGui::InputInt("New Age##UpdateAnimalAge", &animal_update_age_val);
                    ImGui::InputText("New Start Date##UpdateAnimalStart", animal_update_start_date_buf, IM_ARRAYSIZE(animal_update_start_date_buf));
                    ImGui::InputText("New Return Date (YYYY-MM-DD or empty/NULL)##UpdateAnimalReturn", animal_update_return_date_buf, IM_ARRAYSIZE(animal_update_return_date_buf));
                    
                    if (ImGui::Button("Update This Animal##UpdateAnimalBtn")) {
                        AnimalData updated_an_data = {animal_selected_id_for_ops, 
                                                     animal_update_selected_allowed_type, animal_update_selected_customer_cnic, 
                                                     "", animal_update_name_buf, animal_update_age_val,
                                                     animal_update_start_date_buf, 
                                                     (strcmp(animal_update_return_date_buf, "NULL") == 0 || strlen(animal_update_return_date_buf) == 0) ? "" : animal_update_return_date_buf};
                        if (ui_updateAnimal(updated_an_data)) { // ui_updateAnimal will check for actual changes
                            statusMessage = "Animal ID " + std::to_string(animal_selected_id_for_ops) + " update submitted.";
                            animals_list_display = ui_fetchAllAnimals(); 
                            animal_selected_id_for_ops = 0; 
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete This Animal##DeleteAnimalBtn")) {
                        if (ui_deleteAnimal(animal_selected_id_for_ops)) {
                            statusMessage = "Animal ID " + std::to_string(animal_selected_id_for_ops) + " deleted.";
                            animals_list_display = ui_fetchAllAnimals();
                            animal_selected_id_for_ops = 0; 
                        }
                    }
                    ImGui::TreePop();
                }
            }
        } // End Manage Animals
        // == ALLOWED ANIMAL MANAGEMENT SECTION ==
        if (ImGui::CollapsingHeader("Manage Allowed Animals")) {
            if (ImGui::Button("Load Allowed Animal Types##AA")) {
                allowed_animals_list_display = ui_fetchAllAllowedAnimals();
                statusMessage = "Loaded " + std::to_string(allowed_animals_list_display.size()) + " types.";
                aa_selected_type_for_ops = "";
            }
            ImGui::BeginChild("AllowedAnimalListChild", ImVec2(0, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("AllowedAnimalsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Animal Type"); ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 60.0f); ImGui::TableHeadersRow();
                for (int i = 0; i < allowed_animals_list_display.size(); ++i) {
                    const auto& aa = allowed_animals_list_display[i];
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(aa.animal_type.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::PushID(i + 2000); // Unique ID
                    if(ImGui::SmallButton("Select")){
                        aa_selected_type_for_ops = aa.animal_type;
                        statusMessage = "Selected for delete: " + aa.animal_type;
                    }
                    ImGui::PopID();
                } ImGui::EndTable();
            } ImGui::EndChild(); ImGui::Separator();
            if(ImGui::TreeNodeEx("Add New Allowed Animal Type", ImGuiTreeNodeFlags_Framed)){
                ImGui::InputText("Type##AddAA", aa_add_type_buf, IM_ARRAYSIZE(aa_add_type_buf));
                if (ImGui::Button("Submit New Type##AddAABtn")) {
                    AllowedAnimalData new_aa = { aa_add_type_buf };
                    if (ui_insertAllowedAnimal(new_aa)) {
                        statusMessage = "Type '" + toLower(std::string(aa_add_type_buf)) + "' added!";
                        aa_add_type_buf[0] = '\0';
                        allowed_animals_list_display = ui_fetchAllAllowedAnimals();
                    }
                }
                ImGui::TreePop();
            } ImGui::Separator();
            if(!aa_selected_type_for_ops.empty()){
                if(ImGui::TreeNodeEx("Delete Selected Allowed Animal Type", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)){
                    ImGui::Text("Selected Type for Deletion: %s", aa_selected_type_for_ops.c_str());
                    if(ImGui::Button("Confirm Delete Type##DelAABtn")){
                        if(ui_deleteAllowedAnimal(aa_selected_type_for_ops)){
                            statusMessage = "Type '" + toLower(aa_selected_type_for_ops) + "' deleted.";
                            allowed_animals_list_display = ui_fetchAllAllowedAnimals();
                            aa_selected_type_for_ops = "";
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }

        // == EMPLOYEE MANAGEMENT SECTION ==
        if (ImGui::CollapsingHeader("Manage Employees")) {
            if (ImGui::Button("Load All Employees##Emp")) {
                employees_list_display = ui_fetchAllEmployees();
                statusMessage = "Loaded " + std::to_string(employees_list_display.size()) + " employees.";
                emp_selected_id_for_ops = 0; emp_selected_cnic_display = "";
                emp_update_name_buf[0]='\0'; emp_update_role_buf[0]='\0'; emp_update_address_buf[0]='\0';
            }
            ImGui::BeginChild("EmployeeListChild", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("EmployeesTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Emp ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("CNIC", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Role", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableHeadersRow();
                for (int i = 0; i < employees_list_display.size(); ++i) {
                    const auto& emp = employees_list_display[i];
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%d", emp.emp_id);
                    ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(emp.cnic.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(emp.name.c_str());
                    ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(emp.workerOrTrainer.c_str());
                    ImGui::TableSetColumnIndex(4); ImGui::TextUnformatted(emp.address.c_str());
                    ImGui::TableSetColumnIndex(5); ImGui::PushID(i + 3000); // Unique ID
                    if(ImGui::SmallButton("Select")){
                        emp_selected_id_for_ops = emp.emp_id;
                        emp_selected_cnic_display = emp.cnic;
                        strncpy(emp_update_name_buf, emp.name.c_str(), sizeof(emp_update_name_buf)-1); emp_update_name_buf[sizeof(emp_update_name_buf)-1]=0;
                        strncpy(emp_update_role_buf, emp.workerOrTrainer.c_str(), sizeof(emp_update_role_buf)-1); emp_update_role_buf[sizeof(emp_update_role_buf)-1]=0;
                        strncpy(emp_update_address_buf, emp.address.c_str(), sizeof(emp_update_address_buf)-1); emp_update_address_buf[sizeof(emp_update_address_buf)-1]=0;
                        statusMessage = "Selected Employee ID: " + std::to_string(emp.emp_id);
                    }
                    ImGui::PopID();
                } ImGui::EndTable();
            } ImGui::EndChild(); ImGui::Separator();
            if(ImGui::TreeNodeEx("Add New Employee", ImGuiTreeNodeFlags_Framed)){
                ImGui::InputText("CNIC##AddEmp", emp_add_cnic_buf, IM_ARRAYSIZE(emp_add_cnic_buf));
                ImGui::InputText("Name##AddEmp", emp_add_name_buf, IM_ARRAYSIZE(emp_add_name_buf));
                ImGui::InputText("Role (Worker/Trainer)##AddEmp", emp_add_role_buf, IM_ARRAYSIZE(emp_add_role_buf));
                ImGui::InputText("Address##AddEmp", emp_add_address_buf, IM_ARRAYSIZE(emp_add_address_buf));
                if (ImGui::Button("Submit New Employee##AddEmpBtn")) {
                    EmployeeData new_emp = {0, emp_add_cnic_buf, emp_add_name_buf, emp_add_role_buf, emp_add_address_buf};
                    if (ui_insertEmployee(new_emp)) {
                        statusMessage = "Employee '" + new_emp.name + "' added!";
                        emp_add_cnic_buf[0]='\0';emp_add_name_buf[0]='\0';emp_add_role_buf[0]='\0';emp_add_address_buf[0]='\0';
                        employees_list_display = ui_fetchAllEmployees();
                        available_employees_for_worker = ui_fetchAllEmployees(); // Refresh for worker dropdown
                    }
                }
                ImGui::TreePop();
            } ImGui::Separator();
            if (emp_selected_id_for_ops > 0) {
                if(ImGui::TreeNodeEx("Update/Delete Selected Employee", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)){
                    ImGui::Text("Operating on Emp ID: %d (CNIC: %s - Not updatable)", emp_selected_id_for_ops, emp_selected_cnic_display.c_str());
                    ImGui::InputText("New Name##UpdateEmp", emp_update_name_buf, IM_ARRAYSIZE(emp_update_name_buf));
                    ImGui::InputText("New Role (Worker/Trainer)##UpdateEmp", emp_update_role_buf, IM_ARRAYSIZE(emp_update_role_buf));
                    ImGui::InputText("New Address##UpdateEmp", emp_update_address_buf, IM_ARRAYSIZE(emp_update_address_buf));
                    if (ImGui::Button("Update This Employee##UpdateEmpBtn")) {
                        EmployeeData data_to_update_with = {0, "", emp_update_name_buf, emp_update_role_buf, emp_update_address_buf};
                        if (strlen(emp_update_name_buf) > 0 || strlen(emp_update_role_buf) > 0 || strlen(emp_update_address_buf) > 0) {
                            if (ui_updateEmployee(emp_selected_id_for_ops, data_to_update_with)) {
                                statusMessage = "Employee ID " + std::to_string(emp_selected_id_for_ops) + " updated.";
                                employees_list_display = ui_fetchAllEmployees();
                                available_employees_for_worker = ui_fetchAllEmployees();
                                emp_selected_id_for_ops = 0; emp_selected_cnic_display = "";
                                emp_update_name_buf[0]='\0'; emp_update_role_buf[0]='\0'; emp_update_address_buf[0]='\0';
                            }
                        } else {
                            statusMessage = "No new data entered to update for Employee ID " + std::to_string(emp_selected_id_for_ops);
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete This Employee##DeleteEmpBtn")) {
                        if (ui_deleteEmployee(emp_selected_id_for_ops)) {
                            statusMessage = "Employee ID " + std::to_string(emp_selected_id_for_ops) + " deleted.";
                            employees_list_display = ui_fetchAllEmployees();
                            available_employees_for_worker = ui_fetchAllEmployees(); // Refresh dropdowns
                            workers_list_display = ui_fetchAllWorkers(); // Refresh worker list as employee might be gone
                            // TODO: Refresh trainer list if that's implemented
                            emp_selected_id_for_ops = 0; emp_selected_cnic_display = "";
                            emp_update_name_buf[0]='\0'; emp_update_role_buf[0]='\0'; emp_update_address_buf[0]='\0';
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }

        // == TRAINING MANAGEMENT SECTION ==
        if (ImGui::CollapsingHeader("Manage Trainings")) {
            if (ImGui::Button("Load All Trainings##Tr")) {
                trainings_list_display = ui_fetchAllTrainings();
                statusMessage = "Loaded " + std::to_string(trainings_list_display.size()) + " trainings.";
                tr_selected_id_for_ops = 0; tr_update_name_buf[0]='\0';
            }
            ImGui::BeginChild("TrainingListChild", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("TrainingsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableHeadersRow();
                for (int i = 0; i < trainings_list_display.size(); ++i) {
                    const auto& tr = trainings_list_display[i];
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%d", tr.training_id);
                    ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(tr.name.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::PushID(i + 4000); // Unique ID
                    if (ImGui::SmallButton("Select")) {
                        tr_selected_id_for_ops = tr.training_id;
                        strncpy(tr_update_name_buf, tr.name.c_str(), sizeof(tr_update_name_buf) -1); tr_update_name_buf[sizeof(tr_update_name_buf)-1] = 0;
                        statusMessage = "Selected Training ID: " + std::to_string(tr.training_id);
                    }
                    ImGui::PopID();
                } ImGui::EndTable();
            } ImGui::EndChild(); ImGui::Separator();
            if(ImGui::TreeNodeEx("Add New Training", ImGuiTreeNodeFlags_Framed)){
                ImGui::InputText("Training Name##AddTr", tr_add_name_buf, IM_ARRAYSIZE(tr_add_name_buf));
                if (ImGui::Button("Submit New Training##AddTrBtn")) {
                    TrainingData new_tr = {0, tr_add_name_buf};
                    if (ui_insertTraining(new_tr)) {
                        statusMessage = "Training '" + std::string(tr_add_name_buf) + "' added!";
                        tr_add_name_buf[0] = '\0';
                        trainings_list_display = ui_fetchAllTrainings();
                    }
                }
                ImGui::TreePop();
            } ImGui::Separator();
            if (tr_selected_id_for_ops > 0) {
                if(ImGui::TreeNodeEx("Update/Delete Selected Training", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)){
                    ImGui::Text("Updating/Deleting Training ID: %d", tr_selected_id_for_ops);
                    ImGui::InputText("New Name##UpdateTr", tr_update_name_buf, IM_ARRAYSIZE(tr_update_name_buf));
                    if (ImGui::Button("Update This Training##UpdateTrBtn")) {
                        if(strlen(tr_update_name_buf) > 0) {
                            TrainingData updated_tr = {tr_selected_id_for_ops, tr_update_name_buf};
                            if (ui_updateTraining(updated_tr)) {
                                statusMessage = "Training ID " + std::to_string(tr_selected_id_for_ops) + " updated.";
                                trainings_list_display = ui_fetchAllTrainings();
                                tr_selected_id_for_ops = 0; tr_update_name_buf[0] = '\0';
                            }
                        } else {
                             statusMessage = "Training name cannot be empty for update.";
                        }
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Delete This Training##DeleteTrBtn")){
                        if(ui_deleteTraining(tr_selected_id_for_ops)){
                            statusMessage = "Training ID " + std::to_string(tr_selected_id_for_ops) + " deleted.";
                            trainings_list_display = ui_fetchAllTrainings();
                            tr_selected_id_for_ops = 0; tr_update_name_buf[0]='\0';
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }

        // == WORKER MANAGEMENT SECTION ==
        if (ImGui::CollapsingHeader("Manage Workers")) {
            if (ImGui::Button("Load All Workers##Wrk")) {
                workers_list_display = ui_fetchAllWorkers();
                available_employees_for_worker = ui_fetchAllEmployees();
                statusMessage = "Loaded " + std::to_string(workers_list_display.size()) + " workers.";
                worker_selected_id_for_ops = 0; worker_selected_employee_name_for_ops = "";
                worker_update_type_buf[0] = '\0'; worker_update_age_val = 0;
            }
            ImGui::BeginChild("WorkerListChild", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("WorkersTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Worker ID (EmpID)", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("Employee Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Worker Type", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Age", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableHeadersRow();
                for (int i = 0; i < workers_list_display.size(); ++i) {
                    const auto& wrk = workers_list_display[i];
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%d", wrk.worker_id);
                    ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(wrk.employeeName.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(wrk.workerType.c_str());
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%d", wrk.age);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::PushID(i + 7000);
                    if (ImGui::SmallButton("Select")) {
                        worker_selected_id_for_ops = wrk.worker_id;
                        worker_selected_employee_name_for_ops = wrk.employeeName;
                        strncpy(worker_update_type_buf, wrk.workerType.c_str(), sizeof(worker_update_type_buf) -1); worker_update_type_buf[sizeof(worker_update_type_buf)-1] = 0;
                        worker_update_age_val = wrk.age;
                        statusMessage = "Selected Worker for ops: ID " + std::to_string(wrk.worker_id) + " (" + wrk.employeeName + ")";
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::Separator();
            if(ImGui::TreeNodeEx("Add New Worker Specialization", ImGuiTreeNodeFlags_Framed)){
                if (available_employees_for_worker.empty() && ImGui::IsWindowAppearing()) {
                    available_employees_for_worker = ui_fetchAllEmployees();
                }
                const char* combo_preview_value = "Select Employee (must be 'Worker' role)";
                if(worker_add_selected_emp_id > 0){
                    auto it = std::find_if(available_employees_for_worker.begin(), available_employees_for_worker.end(),
                                           [&](const EmployeeData& e){ return e.emp_id == worker_add_selected_emp_id; });
                    if(it != available_employees_for_worker.end()){
                        combo_preview_value = (it->name + " (" + it->workerOrTrainer + ")").c_str();
                    }
                }
                if (ImGui::BeginCombo("Employee to make Worker##AddWrkEmp", combo_preview_value)) {
                    for (const auto& emp : available_employees_for_worker) {
                        if (toLower(emp.workerOrTrainer) != "worker") continue;
                        bool already_worker = std::any_of(workers_list_display.begin(), workers_list_display.end(),
                                                          [&](const WorkerData& w){ return w.worker_id == emp.emp_id; });
                        if(already_worker) continue;
                        const bool is_selected = (worker_add_selected_emp_id == emp.emp_id);
                        if (ImGui::Selectable((std::to_string(emp.emp_id) + ": " + emp.name).c_str(), is_selected))
                            worker_add_selected_emp_id = emp.emp_id;
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::InputText("Worker Type##AddWrkType", worker_add_type_buf, IM_ARRAYSIZE(worker_add_type_buf));
                ImGui::InputInt("Age##AddWrkAge", &worker_add_age_val, 1, 5); ImGui::SameLine(); ImGui::Text(" (years)");
                if (ImGui::Button("Submit New Worker##AddWrkBtn")) {
                    if (worker_add_selected_emp_id > 0 && strlen(worker_add_type_buf) > 0) {
                        WorkerData new_wrk = {worker_add_selected_emp_id, "", worker_add_type_buf, worker_add_age_val};
                        if (ui_insertWorker(new_wrk)) {
                            statusMessage = "Worker details added for Emp ID " + std::to_string(worker_add_selected_emp_id);
                            worker_add_selected_emp_id = 0; worker_add_type_buf[0] = '\0'; worker_add_age_val = 0;
                            workers_list_display = ui_fetchAllWorkers();
                        }
                    } else { statusMessage = "Error: Employee ID, and Worker Type are required."; }
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (worker_selected_id_for_ops > 0) {
                if(ImGui::TreeNodeEx("Update/Delete Selected Worker", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)){
                    ImGui::Text("Operating on Worker ID: %d (%s)", worker_selected_id_for_ops, worker_selected_employee_name_for_ops.c_str());
                    ImGui::InputText("New Worker Type##UpdateWrk", worker_update_type_buf, IM_ARRAYSIZE(worker_update_type_buf));
                    ImGui::InputInt("New Age##UpdateWrk", &worker_update_age_val, 1, 5); ImGui::SameLine(); ImGui::Text(" (years)");
                    if (ImGui::Button("Update This Worker##UpdateWrkBtn")) {
                        WorkerData data_to_send_for_update = {worker_selected_id_for_ops, "", worker_update_type_buf, worker_update_age_val};
                        if (strlen(worker_update_type_buf) > 0 || worker_update_age_val > 0) { // Check if there's anything to update
                            if (ui_updateWorker(data_to_send_for_update)) {
                                statusMessage = "Worker ID " + std::to_string(worker_selected_id_for_ops) + " updated.";
                                workers_list_display = ui_fetchAllWorkers();
                                worker_selected_id_for_ops = 0; worker_selected_employee_name_for_ops = "";
                                worker_update_type_buf[0] = '\0'; worker_update_age_val = 0;
                            }
                        } else {
                             statusMessage = "No new data entered to update for worker " + std::to_string(worker_selected_id_for_ops);
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete This Worker##DeleteWrkBtn")) {
                        if (ui_deleteWorker(worker_selected_id_for_ops)) {
                            statusMessage = "Worker ID " + std::to_string(worker_selected_id_for_ops) + " deleted.";
                            workers_list_display = ui_fetchAllWorkers();
                            worker_selected_id_for_ops = 0; worker_selected_employee_name_for_ops = "";
                            worker_update_type_buf[0] = '\0'; worker_update_age_val = 0;
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }
        
        // ... (after "Manage Workers" ImGui::CollapsingHeader block)

        // == TRAINER MANAGEMENT SECTION ==
        if (ImGui::CollapsingHeader("Manage Trainers")) {
            if (ImGui::Button("Load All Trainers##TrnMain")) {
                trainers_list_display = ui_fetchAllTrainers();
                available_employees_for_trainer = ui_fetchAllEmployees(); 
                statusMessage = "Loaded " + std::to_string(trainers_list_display.size()) + " trainers.";
                trainer_selected_id_for_ops = 0; 
                trainer_selected_employee_name_for_ops = "";
                trainer_update_age_val = 0; 
                trainer_update_phone_buf[0]='\0';
            }

            ImGui::BeginChild("TrainerListChild", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("TrainersTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Trainer ID (EmpID)", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("Employee Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Age", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Phone", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableHeadersRow();
                for (int i = 0; i < trainers_list_display.size(); ++i) {
                    const auto& trn = trainers_list_display[i];
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%d", trn.trainer_id);
                    ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(trn.employeeName.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%d", trn.age);
                    ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(trn.phone.c_str());
                    ImGui::TableSetColumnIndex(4);
                    ImGui::PushID(i + 8000); 
                    if (ImGui::SmallButton("Select")) {
                        trainer_selected_id_for_ops = trn.trainer_id;
                        trainer_selected_employee_name_for_ops = trn.employeeName; // Store name for display
                        trainer_update_age_val = trn.age;
                        strncpy(trainer_update_phone_buf, trn.phone.c_str(), sizeof(trainer_update_phone_buf) -1); 
                        trainer_update_phone_buf[sizeof(trainer_update_phone_buf)-1]=0;
                        statusMessage = "Selected Trainer ID: " + std::to_string(trn.trainer_id);
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Delete")) {
                        if (ui_deleteTrainer(trn.trainer_id)) {
                            statusMessage = "Trainer ID " + std::to_string(trn.trainer_id) + " specialization deleted.";
                            trainers_list_display = ui_fetchAllTrainers(); 
                            trainer_selected_id_for_ops = 0;
                        }
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::Separator();

            if(ImGui::TreeNodeEx("Add New Trainer Specialization", ImGuiTreeNodeFlags_Framed)){
                if (available_employees_for_trainer.empty() && ImGui::IsWindowAppearing()) { 
                    available_employees_for_trainer = ui_fetchAllEmployees();
                }
                const char* combo_preview_trn = "Select Employee (must be 'Trainer' role)";
                if(trainer_add_selected_emp_id > 0){
                    auto it = std::find_if(available_employees_for_trainer.begin(), available_employees_for_trainer.end(), 
                                           [&](const EmployeeData& e){ return e.emp_id == trainer_add_selected_emp_id; });
                    if(it != available_employees_for_trainer.end()){ 
                        combo_preview_trn = (it->name + " (" + it->workerOrTrainer + ")").c_str(); 
                    }
                }
                if (ImGui::BeginCombo("Employee to make Trainer##AddTrnEmp", combo_preview_trn)) {
                    for (const auto& emp : available_employees_for_trainer) {
                        if (toLower(emp.workerOrTrainer) != "trainer") continue; 
                        bool already_specialized = std::any_of(trainers_list_display.begin(), trainers_list_display.end(), 
                                                              [&](const TrainerData& t){ return t.trainer_id == emp.emp_id; });
                        if(already_specialized) continue; // Skip if already a trainer
                        // You might also want to check if they are already a Worker and prevent them from being both
                        const bool is_selected = (trainer_add_selected_emp_id == emp.emp_id);
                        if (ImGui::Selectable((std::to_string(emp.emp_id) + ": " + emp.name).c_str(), is_selected))
                            trainer_add_selected_emp_id = emp.emp_id;
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::InputInt("Age##AddTrnAge", &trainer_add_age_val);
                ImGui::InputText("Phone##AddTrnPhone", trainer_add_phone_buf, IM_ARRAYSIZE(trainer_add_phone_buf));
                if (ImGui::Button("Submit New Trainer##AddTrnBtn")) {
                    if(trainer_add_selected_emp_id > 0) {
                        TrainerData new_trn = {trainer_add_selected_emp_id, "", trainer_add_age_val, trainer_add_phone_buf};
                        if (ui_insertTrainer(new_trn)) {
                            statusMessage = "Trainer details added for Emp ID " + std::to_string(trainer_add_selected_emp_id);
                            trainer_add_selected_emp_id = 0; trainer_add_age_val = 0; trainer_add_phone_buf[0] = '\0'; 
                            trainers_list_display = ui_fetchAllTrainers(); 
                        }
                    } else {
                        statusMessage = "Please select an Employee to make a Trainer.";
                    }
                }
                ImGui::TreePop();
            }
            ImGui::Separator();

            if (trainer_selected_id_for_ops > 0) {
                if(ImGui::TreeNodeEx("Update Selected Trainer", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)){
                    ImGui::Text("Updating Trainer ID (Emp ID): %d (%s)", trainer_selected_id_for_ops, trainer_selected_employee_name_for_ops.c_str());
                    ImGui::InputInt("New Age##UpdateTrn", &trainer_update_age_val);
                    ImGui::InputText("New Phone##UpdateTrn", trainer_update_phone_buf, IM_ARRAYSIZE(trainer_update_phone_buf));
                    if (ImGui::Button("Update This Trainer##UpdateTrnBtn")) {
                        // Check if any actual change was made in the buffers
                        bool has_actual_changes = false;
                        if (trainer_update_age_val != 0) { // Assuming 0 is not a valid age to set, or use comparison with original
                             auto original_trainer_it = std::find_if(trainers_list_display.begin(), trainers_list_display.end(),
                                [&](const TrainerData& t){ return t.trainer_id == trainer_selected_id_for_ops; });
                             if(original_trainer_it != trainers_list_display.end()){
                                if(original_trainer_it->age != trainer_update_age_val) has_actual_changes = true;
                                if(original_trainer_it->phone != trainer_update_phone_buf && strlen(trainer_update_phone_buf) > 0) has_actual_changes = true;
                             } else { // Fallback if original not found (should not happen if selected from list)
                                has_actual_changes = true; 
                             }
                        }
                         if(strlen(trainer_update_phone_buf) > 0) has_actual_changes = true;


                        if(has_actual_changes){
                            TrainerData updated_trn = {trainer_selected_id_for_ops, "", trainer_update_age_val, trainer_update_phone_buf};
                            if (ui_updateTrainer(updated_trn)) {
                                statusMessage = "Trainer ID " + std::to_string(trainer_selected_id_for_ops) + " updated.";
                                trainers_list_display = ui_fetchAllTrainers(); 
                                trainer_selected_id_for_ops = 0; 
                                trainer_update_age_val = 0; trainer_update_phone_buf[0] = '\0';
                            }
                        } else {
                             statusMessage = "No changes entered for Trainer ID " + std::to_string(trainer_selected_id_for_ops);
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }

        // == SERVICE BY TRAINER (Training_Trainer) MANAGEMENT SECTION ==
        if (ImGui::CollapsingHeader("Manage Service by Trainer")) {
            if (ImGui::Button("Load All Service Assignments##SBT")) {
                service_by_trainer_list_display = ui_fetchAllServiceByTrainer();
                if(available_trainings_for_sbt.empty()) available_trainings_for_sbt = ui_fetchAllTrainings(); 
                if(available_trainers_for_sbt.empty()) available_trainers_for_sbt = ui_fetchAllTrainers();   
                statusMessage = "Loaded " + std::to_string(service_by_trainer_list_display.size()) + " service assignments.";
                sbt_selected_tt_id_for_ops = 0;
            }
            ImGui::BeginChild("ServiceByTrainerListChild", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("ServiceByTrainerTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Assign. ID (TT_id)", ImGuiTableColumnFlags_WidthFixed, 120.0f); 
                ImGui::TableSetupColumn("Training Name (ID)", ImGuiTableColumnFlags_WidthStretch); 
                ImGui::TableSetupColumn("Trainer Name (ID)", ImGuiTableColumnFlags_WidthStretch); 
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 70.0f); 
                ImGui::TableHeadersRow();
                for (const auto& sbt : service_by_trainer_list_display) { 
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%d", sbt.tt_id); 
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s (%d)", sbt.trainingName.c_str(), sbt.training_id); 
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%s (%d)", sbt.trainerName.c_str(), sbt.trainer_id); 
                    ImGui::TableSetColumnIndex(3); ImGui::PushID(sbt.tt_id + 9000); 
                    if (ImGui::SmallButton("Delete")) { 
                        if (ui_deleteServiceByTrainer(sbt.tt_id)) { 
                            statusMessage = "Assignment ID " + std::to_string(sbt.tt_id) + " deleted."; 
                            service_by_trainer_list_display = ui_fetchAllServiceByTrainer(); 
                        } 
                    } ImGui::PopID(); 
                } ImGui::EndTable();
            } ImGui::EndChild(); ImGui::Separator();
            if(ImGui::TreeNodeEx("Assign Trainer to Training Service", ImGuiTreeNodeFlags_Framed)){
                if (available_trainings_for_sbt.empty() && ImGui::IsWindowAppearing()) available_trainings_for_sbt = ui_fetchAllTrainings();
                const char* combo_training_preview_sbt = "Select Training"; 
                if(sbt_add_selected_training_id > 0){ 
                    auto it = std::find_if(available_trainings_for_sbt.begin(), available_trainings_for_sbt.end(), 
                                           [&](const TrainingData& t){ return t.training_id == sbt_add_selected_training_id; }); 
                    if(it != available_trainings_for_sbt.end()){ combo_training_preview_sbt = it->name.c_str(); }
                }
                if (ImGui::BeginCombo("Select Training##SBTAddTraining", combo_training_preview_sbt)) { 
                    for (const auto& tr : available_trainings_for_sbt) { 
                        const bool is_selected = (sbt_add_selected_training_id == tr.training_id); 
                        if (ImGui::Selectable((std::to_string(tr.training_id) + ": " + tr.name).c_str(), is_selected)) 
                            sbt_add_selected_training_id = tr.training_id; 
                        if (is_selected) ImGui::SetItemDefaultFocus(); 
                    } ImGui::EndCombo(); 
                }
                
                if (available_trainers_for_sbt.empty() && ImGui::IsWindowAppearing()) available_trainers_for_sbt = ui_fetchAllTrainers(); 
                const char* combo_trainer_preview_sbt = "Select Trainer"; 
                if(sbt_add_selected_trainer_id > 0){ 
                    auto it = std::find_if(available_trainers_for_sbt.begin(), available_trainers_for_sbt.end(), 
                                           [&](const TrainerData& t){ return t.trainer_id == sbt_add_selected_trainer_id; }); 
                    if(it != available_trainers_for_sbt.end()){ combo_trainer_preview_sbt = it->employeeName.c_str(); }
                }
                if (ImGui::BeginCombo("Select Trainer##SBTAddTrainer", combo_trainer_preview_sbt)) { 
                    for (const auto& trn : available_trainers_for_sbt) { 
                        const bool is_selected = (sbt_add_selected_trainer_id == trn.trainer_id); 
                        if (ImGui::Selectable((std::to_string(trn.trainer_id) + ": " + trn.employeeName).c_str(), is_selected)) 
                            sbt_add_selected_trainer_id = trn.trainer_id; 
                        if (is_selected) ImGui::SetItemDefaultFocus(); 
                    } ImGui::EndCombo(); 
                }

                if (ImGui::Button("Assign Service##SBTAddBtn")) { 
                    if (sbt_add_selected_training_id > 0 && sbt_add_selected_trainer_id > 0) { 
                        if (ui_insertServiceByTrainer(sbt_add_selected_training_id, sbt_add_selected_trainer_id)) { 
                            statusMessage = "Trainer assigned to training successfully!"; 
                            sbt_add_selected_training_id = 0; sbt_add_selected_trainer_id = 0;
                            service_by_trainer_list_display = ui_fetchAllServiceByTrainer(); 
                        } 
                    } else { statusMessage = "Please select both a training and a trainer."; } 
                }
                ImGui::TreePop();
            }
        }

        // == ANIMAL TRAINING SCHEDULE (Animal_Training_Trainer) SECTION ==
        if (ImGui::CollapsingHeader("Manage Animal Training Schedule")) {
            if (ImGui::Button("Load All Schedules##ATS")) {
                animal_schedule_list_display = ui_fetchAllAnimalSchedules();
                if(available_animals_for_ats.empty()) available_animals_for_ats = ui_fetchAllAnimals();
                if(available_tt_assignments_for_ats.empty()) available_tt_assignments_for_ats = ui_fetchAllServiceByTrainer();
                statusMessage = "Loaded " + std::to_string(animal_schedule_list_display.size()) + " animal training schedules.";
                ats_delete_selected_animal_id_key = 0; 
                ats_delete_selected_tt_id_key = 0;
            }

            ImGui::BeginChild("AnimalScheduleListChild", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginTable("AnimalSchedulesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Animal (ID)", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Training (Service TT_ID)", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Trainer", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableHeadersRow();
                for (const auto& ats : animal_schedule_list_display) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s (%d)", ats.animalNameDisplay.c_str(), ats.animal_id);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s (TT_ID:%d)", ats.trainingNameDisplay.c_str(), ats.tt_id);
                    ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(ats.trainerNameDisplay.c_str());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::PushID(ats.animal_id * 1000 + ats.tt_id + 12000); 
                    if (ImGui::SmallButton("Select to Delete")) { 
                        ats_delete_selected_animal_id_key = ats.animal_id;
                        ats_delete_selected_tt_id_key = ats.tt_id;
                        statusMessage = "Selected schedule for Animal " + ats.animalNameDisplay + " with Service TT_ID " + std::to_string(ats.tt_id);
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            ImGui::Separator();

            if(ImGui::TreeNodeEx("Assign Animal to Training Service", ImGuiTreeNodeFlags_Framed)){
                // Dropdown for Animals
                if(available_animals_for_ats.empty() && ImGui::IsWindowAppearing()) available_animals_for_ats = ui_fetchAllAnimals();
                const char* combo_animal_preview_ats = "Select Animal";
                if(ats_add_selected_animal_id > 0){
                    auto it = std::find_if(available_animals_for_ats.begin(), available_animals_for_ats.end(), 
                                           [&](const AnimalData& a){ return a.animal_id == ats_add_selected_animal_id; });
                    if(it != available_animals_for_ats.end()){ combo_animal_preview_ats = it->animalName.c_str(); }
                }
                if (ImGui::BeginCombo("Select Animal##ATSAddAnimal", combo_animal_preview_ats)) {
                    for (const auto& an : available_animals_for_ats) {
                        const bool is_selected = (ats_add_selected_animal_id == an.animal_id);
                        if (ImGui::Selectable((std::to_string(an.animal_id) + ": " + an.animalName).c_str(), is_selected))
                            ats_add_selected_animal_id = an.animal_id;
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                // Dropdown for Training_Trainer (Service by Trainer) assignments
                if(available_tt_assignments_for_ats.empty() && ImGui::IsWindowAppearing()) available_tt_assignments_for_ats = ui_fetchAllServiceByTrainer();
                const char* combo_tt_preview_ats = "Select Service (Training by Trainer)";
                if(ats_add_selected_tt_id > 0){
                    auto it = std::find_if(available_tt_assignments_for_ats.begin(), available_tt_assignments_for_ats.end(), 
                                           [&](const TrainingTrainerData& tt){ return tt.tt_id == ats_add_selected_tt_id; });
                    if(it != available_tt_assignments_for_ats.end()){ 
                        combo_tt_preview_ats = (it->trainingName + " by " + it->trainerName + " [TTID:" + std::to_string(it->tt_id) + "]").c_str(); 
                    }
                }
                if (ImGui::BeginCombo("Select Service Assignment (TT_ID)##ATSAddTT", combo_tt_preview_ats)) {
                    for (const auto& tt : available_tt_assignments_for_ats) {
                        const bool is_selected = (ats_add_selected_tt_id == tt.tt_id);
                        std::string item_label = std::to_string(tt.tt_id) + ": " + tt.trainingName + " (Trainer: " + tt.trainerName + ")";
                        if (ImGui::Selectable(item_label.c_str(), is_selected))
                            ats_add_selected_tt_id = tt.tt_id;
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Assign Animal to Service##ATSAddBtn")) {
                    if (ats_add_selected_animal_id > 0 && ats_add_selected_tt_id > 0) {
                        if (ui_insertAnimalSchedule(ats_add_selected_animal_id, ats_add_selected_tt_id)) {
                            statusMessage = "Animal assigned to service successfully!";
                            ats_add_selected_animal_id = 0; ats_add_selected_tt_id = 0; // Clear selections
                            animal_schedule_list_display = ui_fetchAllAnimalSchedules(); // Refresh
                        } 
                    } else {
                        statusMessage = "Please select both an animal and a service assignment.";
                    }
                }
                ImGui::TreePop();
            }
             ImGui::Separator();
            if(ats_delete_selected_animal_id_key > 0 && ats_delete_selected_tt_id_key > 0){
                 if(ImGui::TreeNodeEx("Delete Selected Animal Schedule", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)){
                    ImGui::Text("Delete schedule for Animal ID: %d with Service TT_ID: %d?", ats_delete_selected_animal_id_key, ats_delete_selected_tt_id_key);
                    if(ImGui::Button("Confirm Delete Schedule##DelATSBtn")){
                        if(ui_deleteAnimalSchedule(ats_delete_selected_animal_id_key, ats_delete_selected_tt_id_key)){
                            statusMessage = "Schedule deleted.";
                            animal_schedule_list_display = ui_fetchAllAnimalSchedules(); // Refresh
                            ats_delete_selected_animal_id_key = 0; ats_delete_selected_tt_id_key = 0; // Clear selection
                        }
                    }
                    ImGui::TreePop();
                 }
            }
        } // End Manage Animal Training Schedule

        ImGui::End(); 
        ImGui::Render();
        int display_w, display_h; glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x*clear_color.w, clear_color.y*clear_color.w, clear_color.z*clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    } 

    disconnectDB();
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
    glfwDestroyWindow(window); glfwTerminate();
    return 0;
}
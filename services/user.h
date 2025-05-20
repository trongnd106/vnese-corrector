#include <ulfius.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#include <jansson.h>
#include <regex.h>
#include <crypt.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define MIN_PASSWORD_LENGTH 8
#define SALT_LENGTH 16

int is_valid_email(const char *email);
int is_valid_password(const char *password);
int email_exists(MYSQL *conn, const char *email);
char* hash_user_password(const char *password);
int verify_password(const char *password, const char *hashed_password);

// POST "/users"
int callback_create_user(const struct _u_request *request, struct _u_response *response, void *user_data);
// POST "/login"
int callback_login_user(const struct _u_request *request, struct _u_response *response, void *user_data);

int is_valid_email(const char *email) {
    regex_t regex;
    int reti;
    const char *pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
    
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        return 0;
    }
    
    reti = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return reti == 0;
}

int is_valid_password(const char *password) {
    if (strlen(password) < MIN_PASSWORD_LENGTH) {
        return 0;
    }
    
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (isupper(password[i])) has_upper = 1;
        else if (islower(password[i])) has_lower = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_special = 1;
    }
    
    return has_upper && has_lower && has_digit && has_special;
}

int email_exists(MYSQL *conn, const char *email) {
    char query[256];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM users WHERE email = '%s'", email);
    
    if (mysql_query(conn, query) != 0) {
        return -1;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        return -1;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    int count = atoi(row[0]);
    
    mysql_free_result(result);
    return count > 0;
}

char* hash_user_password(const char *password) {
    // Generate random salt
    char salt[32];
    const char *salt_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    srand(time(NULL));
    for (int i = 0; i < SALT_LENGTH; i++) {
        salt[i] = salt_chars[rand() % 64];
    }
    salt[SALT_LENGTH] = '\0';
    
    // Generate hash using crypt
    char *hash = crypt(password, salt);
    if (hash == NULL) {
        return NULL;
    }
    
    return strdup(hash);
}

int verify_password(const char *password, const char *hashed_password) {
    char *new_hash = crypt(password, hashed_password);
    if (new_hash == NULL) {
        return 0;
    }
    return strcmp(new_hash, hashed_password) == 0;
}

// POST "/users"
int callback_create_user(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_error_t error;
    json_t *parsed_json = json_loads(request->binary_body, 0, &error);
    if (parsed_json == NULL) {
        ulfius_set_string_body_response(response, 400, "Invalid JSON format");
        return U_ERROR;
    }

    json_t *email_obj = json_object_get(parsed_json, "email");
    json_t *password_obj = json_object_get(parsed_json, "password");

    if (!json_is_string(email_obj) || !json_is_string(password_obj)) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 400, "Missing email or password");
        return U_ERROR;
    }

    const char *email = json_string_value(email_obj);
    const char *password = json_string_value(password_obj);

    // Validate email format
    if (!is_valid_email(email)) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 400, "Invalid email format");
        return U_ERROR;
    }

    // Validate password strength
    if (!is_valid_password(password)) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 400, 
            "Password must be at least 8 characters long and contain uppercase, lowercase, number and special character");
        return U_ERROR;
    }

    MYSQL *conn = (MYSQL *)user_data;

    // Check if email already exists
    if (email_exists(conn, email)) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 400, "Email already registered");
        return U_ERROR;
    }

    // Hash password
    char *hashed_password = hash_user_password(password);
    if (hashed_password == NULL) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 500, "Error hashing password");
        return U_ERROR;
    }

    // Insert user into database
    char query[512];
    snprintf(query, sizeof(query), 
             "INSERT INTO users (email, password) VALUES ('%s', '%s')",
             email, hashed_password);

    if (mysql_query(conn, query) != 0) {
        free(hashed_password);
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 500, mysql_error(conn));
        return U_ERROR;
    }

    free(hashed_password);

    json_t *response_json = json_object();
    json_object_set_new(response_json, "status", json_string("success"));
    json_object_set_new(response_json, "message", json_string("User created successfully"));

    char *response_str = json_dumps(response_json, 0);
    ulfius_set_string_body_response(response, 200, response_str);

    free(response_str);
    json_decref(parsed_json);
    json_decref(response_json);

    return U_OK;
}

// POST "/login"
int callback_login_user(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_error_t error;
    json_t *parsed_json = json_loads(request->binary_body, 0, &error);
    if (parsed_json == NULL) {
        ulfius_set_string_body_response(response, 400, "Invalid JSON format");
        return U_ERROR;
    }

    json_t *email_obj = json_object_get(parsed_json, "email");
    json_t *password_obj = json_object_get(parsed_json, "password");

    if (!json_is_string(email_obj) || !json_is_string(password_obj)) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 400, "Missing email or password");
        return U_ERROR;
    }

    const char *email = json_string_value(email_obj);
    const char *password = json_string_value(password_obj);

    MYSQL *conn = (MYSQL *)user_data;

    char query[256];
    snprintf(query, sizeof(query), "SELECT password FROM users WHERE email = '%s'", email);
    
    if (mysql_query(conn, query) != 0) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 500, mysql_error(conn));
        return U_ERROR;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 500, "Database error");
        return U_ERROR;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL) {
        mysql_free_result(result);
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 401, "Invalid email or password");
        return U_ERROR;
    }

    const char *hashed_password = row[0];
    
    // Verify password
    if (!verify_password(password, hashed_password)) {
        mysql_free_result(result);
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 401, "Invalid email or password");
        return U_ERROR;
    }

    mysql_free_result(result);

    json_t *response_json = json_object();
    json_object_set_new(response_json, "status", json_string("success"));
    json_object_set_new(response_json, "message", json_string("Login successful"));

    char *response_str = json_dumps(response_json, 0);
    ulfius_set_string_body_response(response, 200, response_str);

    free(response_str);
    json_decref(parsed_json);
    json_decref(response_json);

    return U_OK;
}
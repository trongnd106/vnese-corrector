#include <ulfius.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <jansson.h>

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

    MYSQL *conn = (MYSQL *)user_data;

    char query[256];
    snprintf(query, sizeof(query), 
             "INSERT INTO users (email, password) VALUES ('%s', '%s')",
             email, password);

    if (mysql_query(conn, query) != 0) {
        json_decref(parsed_json);
        ulfius_set_string_body_response(response, 500, mysql_error(conn));
        return U_ERROR;
    }

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
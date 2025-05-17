#include <ulfius.h>
#include <stdio.h>
// #include <mysql/mysql.h>
// #include <sqlite3.h>
// #include "model/user_model.h"

#define PORT 8080

// Connect DB
// #define DB_HOST "localhost"
// #define DB_USER "root"
// #define DB_PASS "root"
// #define DB_NAME "corrector"
// #define DB_PORT 3306

// MYSQL *conn;

// Handle request GET "/hello"
int callback_hello(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_t *json_body = json_object();
    json_object_set_new(json_body, "message", json_string("Hello from Ulfius!"));

    ulfius_set_json_body_response(response, 200, json_body);
    json_decref(json_body);
    return U_OK;
}

// POST /hello
int callback_post_hello(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_error_t error;
    const json_t *json_req = ulfius_get_json_body_request(request, &error);

    json_t *json_body = json_object();

    if (json_req && json_is_object(json_req)) {
        const char *name = json_string_value(json_object_get(json_req, "name"));

        if (name) {
            char message[256];
            snprintf(message, sizeof(message), "POST: Hello, %s!", name);
            json_object_set_new(json_body, "message", json_string(message));
        } else {
            json_object_set_new(json_body, "error", json_string("Required field 'name'"));
        }
    } else {
        json_object_set_new(json_body, "error", json_string("Invalid JSON"));
    }

    ulfius_set_json_body_response(response, 200, json_body);
    json_decref(json_body);
    return U_OK;
}

// PUT /hello
int callback_put_hello(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_t *json_body = json_object();
    json_object_set_new(json_body, "message", json_string("PUT: Update data successfully!"));

    ulfius_set_json_body_response(response, 200, json_body);
    json_decref(json_body);
    return U_OK;
}

// DELETE /hello
int callback_delete_hello(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_t *json_body = json_object();
    json_object_set_new(json_body, "message", json_string("DELETE: Delete successfully!"));

    ulfius_set_json_body_response(response, 200, json_body);
    json_decref(json_body);
    return U_OK;
}

// int callback_post_user(const struct _u_request *req, struct _u_response *res, void *db_ptr) {
//     json_error_t error;
//     const json_t *json_req = ulfius_get_json_body_request(req, &error);
//     const char *name = json_string_value(json_object_get(json_req, "name"));

//     json_t *body = json_object();
//     if (!name) {
//         json_object_set_new(body, "error", json_string("Require field 'name'"));
//         ulfius_set_json_body_response(res, 400, body);
//     } else if (create_user((sqlite3 *)db_ptr, name) != 0) {
//         json_object_set_new(body, "error", json_string("Create user failed"));
//         ulfius_set_json_body_response(res, 500, body);
//     } else {
//         json_object_set_new(body, "message", json_string("OK"));
//         ulfius_set_json_body_response(res, 200, body);
//     }
//     json_decref(body);
//     return U_OK;
// }

// int callback_get_users(const struct _u_request *req, struct _u_response *res, void *db_ptr) {
//     json_t *users = get_all_users((sqlite3 *)db_ptr);
//     if (!users) {
//         json_t *error = json_object();
//         json_object_set_new(error, "error", json_string("Lỗi DB"));
//         ulfius_set_json_body_response(res, 500, error);
//         json_decref(error);
//     } else {
//         ulfius_set_json_body_response(res, 200, users);
//         json_decref(users);
//     }
//     return U_OK;
// }


int main(void) {
    struct _u_instance instance;

    // Connect MySQL
    // conn = mysql_init(NULL);
    // if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)) {
    //     fprintf(stderr, "Error MySQL connection: %s\n", mysql_error(conn));
    //     return 1;
    // }

    // sqlite3 *db;
    // if (sqlite3_open("db.sqlite", &db) != SQLITE_OK) {
    //     fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    //     return 1;
    // }

    // Initialize Ulfius
    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error initializing Ulfius framework\n");
        return 1;
    }

    // Define endpoint
    if (ulfius_add_endpoint_by_val(&instance, "GET", "/hello", NULL, 0, &callback_hello, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /hello endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    
    if (ulfius_add_endpoint_by_val(&instance, "POST", "/hello", NULL, 0, &callback_post_hello, NULL) != U_OK) {
        fprintf(stderr, "Error adding POST /hello endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    
    if (ulfius_add_endpoint_by_val(&instance, "PUT", "/hello", NULL, 0, &callback_put_hello, NULL) != U_OK) {
        fprintf(stderr, "Error adding PUT /hello endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }
    
    if (ulfius_add_endpoint_by_val(&instance, "DELETE", "/hello", NULL, 0, &callback_delete_hello, NULL) != U_OK) {
        fprintf(stderr, "Error adding DELETE /hello endpoint\n");
        ulfius_clean_instance(&instance);
        return 1;
    }

    // Run server
    printf("Starting server on port %d...\n", PORT);
    int ret = ulfius_start_framework(&instance);
    if (ret == U_OK) {
        printf("Server running on http://localhost:%d\n", PORT);
        printf("Press Enter to stop the server...\n");
        getchar();
    } else {
        fprintf(stderr, "Error starting server: %d\n", ret);
        fprintf(stderr, "Possible reasons: Port %d might be in use or requires root privileges\n", PORT);
    }

    // Down server
    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);
    // sqlite3_close(db);

    return 0;
}

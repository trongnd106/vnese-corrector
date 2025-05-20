#include <ulfius.h>
#include "../services/view.h"
#include "../services/static.h"
#include "../services/user.h"

int init_api_endpoints(struct _u_instance *instance, MYSQL *conn) {
    if (ulfius_add_endpoint_by_val(instance, "GET", "/statics/*", NULL, 0, &callback_static, NULL) != U_OK) {
        fprintf(stderr, "Error adding static files endpoint\n");
        return 0;
    }

    if (ulfius_add_endpoint_by_val(instance, "GET", "/", NULL, 0, &callback_view_guest, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET / endpoint\n");
        return 0;
    }
    if (ulfius_add_endpoint_by_val(instance, "GET", "/index", NULL, 0, &callback_view, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /index endpoint\n");
        return 0;
    }
    if (ulfius_add_endpoint_by_val(instance, "GET", "/signup", NULL, 0, &callback_view_signup, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /signup endpoint\n");
        return 0;
    }
    if (ulfius_add_endpoint_by_val(instance, "GET", "/login", NULL, 0, &callback_view_login, NULL) != U_OK) {
        fprintf(stderr, "Error adding GET /login endpoint\n");
        return 0;
    }

    if (ulfius_add_endpoint_by_val(instance, "POST", "/users", NULL, 0, &callback_create_user, conn) != U_OK) {
        fprintf(stderr, "Error adding POST /users endpoint\n");
        return 0;
    }
    if (ulfius_add_endpoint_by_val(instance, "POST", "/login", NULL, 0, &callback_login_user, conn) != U_OK) {
        fprintf(stderr, "Error adding POST /login endpoint\n");
        return 0;
    }

    return 1;
} 
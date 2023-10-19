struct BlogOperation {
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};

// Action Types:
# define NEW_CONNECION 1
# define NEW_POST 2
# define LIST_TOPICS 3
# define SUBSCRIBE 4
# define DISCONNECT 5


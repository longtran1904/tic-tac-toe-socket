

typedef struct sock_buf_node {
    int sock;
    char *buf;
    int bytes;
    int buf_offset;
    struct sock_buf_node *next;
} sock_buf_node;

void sock_buf_init();

sock_buf_node *create_new_node( int sock );

int rm_sock_buf_node( int sock );

sock_buf_node *get_buf_info( int sock );

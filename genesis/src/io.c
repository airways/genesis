/*
// ColdMUD was created and is copyright 1993, 1994 by Greg Hudson
//
// Genesis is a derivitive work, and is copyright 1995 by Brandon Gillespie.
// Full details and copyright information can be found in the file doc/CREDITS
//
// File: io.c
// ---
// Network routines.
*/

#define _io_

#include "config.h"
#include "defs.h"

#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "y.tab.h"
#include "io.h"
#include "net.h"
#include "execute.h"
#include "memory.h"
#include "grammar.h"
#include "cdc_types.h"
#include "data.h"
#include "util.h"
#include "cache.h"

INTERNAL void connection_read(connection_t *conn);
INTERNAL void connection_write(connection_t *conn);
INTERNAL connection_t *connection_add(int fd, long objnum);
INTERNAL void connection_discard(connection_t *conn);
INTERNAL void pend_discard(pending_t *pend);
INTERNAL void server_discard(server_t *serv);

INTERNAL connection_t * connections;  /* List of client connections. */
INTERNAL server_t     * servers;      /* List of server sockets. */
INTERNAL pending_t    * pendings;     /* List of pending connections. */

/*
// --------------------------------------------------------------------
// Flush defunct connections and files.
//
// Notify the connection object of any dead connections and delete them.
*/

void flush_defunct(void) {
    connection_t **connp, *conn;
    server_t     **servp, *serv;
    pending_t    **pendp, *pend;
    /* filec_t       **filep, *file; */

    connp = &connections;
    while (*connp) {
        conn = *connp;
        if (conn->flags.dead && conn->write_buf->len == 0) {
            *connp = conn->next;
            connection_discard(conn);
        } else {
            connp = &conn->next;
        }
    }

    servp = &servers;
    while (*servp) {
        serv = *servp;
        if (serv->dead) {
            *servp = serv->next;
            server_discard(serv);
        } else {
            servp = &serv->next;
        }
    }

    pendp = &pendings;
    while (*pendp) {
        pend = *pendp;
        if (pend->finished) {
            *pendp = pend->next;
            pend_discard(pend);
        } else {
            pendp = &pend->next;
        }
    }

#if DISABLED
    filep = &files;
    while (*filep) {
        file = *filep;
        if (file->flags.closed) {
            *filep = file->next;
            file_discard(file);
        } else {
            filep = &file->next;
        }
    }
#endif
}

/*
// --------------------------------------------------------------------
// Call io_event_wait() to wait for something to happen.  The return
// value is nonzero if an I/O event occurred.  If there is a new
// connection, then *fd will be set to the descriptor of the new
// connection; otherwise, it is set to -1.
*/

void handle_io_event_wait(int seconds) {
    io_event_wait(seconds, connections, servers, pendings);
}

/*
// --------------------------------------------------------------------
*/

void handle_connection_input(void) {
    connection_t * conn;

    for (conn = connections; conn; conn = conn->next) {
        if (conn->flags.readable && !conn->flags.dead)
            connection_read(conn);
    }
}

/*
// --------------------------------------------------------------------
*/
void handle_connection_output(void) {
    connection_t * conn;
    /*filec_t       * file;*/

    for (conn = connections; conn; conn = conn->next) {
        if (conn->flags.writable)
            connection_write(conn);
    }

#if DISABLED
    for (file = files; file; file = file->next) {
        if (file->flags.writable && file->wbuf)
            file_write(file);
    }
#endif
}

/*
// --------------------------------------------------------------------
*/
void handle_new_and_pending_connections(void) {
    connection_t *conn;
    server_t *serv;
    pending_t *pend;
    string_t *str;
    data_t d1, d2;

    /* Look for new connections on the server sockets. */
    for (serv = servers; serv; serv = serv->next) {
        if (serv->client_socket == -1)
            continue;
        conn = connection_add(serv->client_socket, serv->objnum);
        serv->client_socket = -1;
        str = string_from_chars(serv->client_addr, strlen(serv->client_addr));
        d1.type = STRING;
        d1.u.str = str;
        d2.type = INTEGER;
        d2.u.val = serv->client_port;
        task(conn->objnum, connect_id, 2, &d1, &d2);
        string_discard(str);
    }

    /* Look for pending connections succeeding or failing. */
    for (pend = pendings; pend; pend = pend->next) {
        if (pend->finished) {
            if (pend->error == NOT_AN_IDENT) {
                conn = connection_add(pend->fd, pend->objnum);
                d1.type = INTEGER;
                d1.u.val = pend->task_id;
                task(conn->objnum, connect_id, 1, &d1);
            } else {
                close(pend->fd);
                d1.type = INTEGER;
                d1.u.val = pend->task_id;
                d2.type = ERROR;
                d2.u.error = pend->error;
                task(pend->objnum, failed_id, 2, &d1, &d2);
            }
        }
    }
}

/*
// --------------------------------------------------------------------
// This will attempt to find a connection associated with an object.
// For faster hunting we will check obj->conn, which may be set to NULL
// even though a connection may exist (the pointer is only valid while
// the object is in the cache, and is reset to NULL when it is read from
// disk).  If obj->conn is NULL and a connection exists, we set
// obj->conn to the connection, so we will know it next time.
//
// Note: if more than one connection is associated with an object, this
// will only return the most recent connection.  Hopefully more than one
// connection will not get associated, we need to hack the server to
// blast old connections when new ones are associated, or to deny new
// ones.  Either way the db should be paying close attention to what
// is occuring.
//
// Once new connections bump old connections, this problem will go
// away.
*/

connection_t * find_connection(object_t * obj) {

    /* obj->conn is only for faster lookups */
    if (obj->conn == NULL) {
        connection_t * conn;

        /* lets try and find the connection */
        for (conn = connections; conn; conn = conn->next) {
            if (conn->objnum == obj->objnum && !conn->flags.dead) {
                obj->conn = conn;
                break;
            }
        }
    }

    /* it could still be NULL */
    return obj->conn;
}

/*
// --------------------------------------------------------------------
// returning the connection is what we are using as a status report, if
// there is no connection, it will be NULL, and we will know.
*/

connection_t * tell(object_t * obj, Buffer * buf) {
    connection_t * conn = find_connection(obj);

    if (conn != NULL)
        conn->write_buf = buffer_append(conn->write_buf, buf);

    return conn;
}

/*
// --------------------------------------------------------------------
*/

int boot(object_t * obj) {
    connection_t * conn = find_connection(obj);

    if (conn != NULL) {
        conn->flags.dead = 1;
        return 1;
    }

    return 0;
}

/*
// --------------------------------------------------------------------
*/

int add_server(int port, long objnum) {
    server_t *cnew;
    int server_socket;

    /* Check if a server already exists for this port. */
    for (cnew = servers; cnew; cnew = cnew->next) {
        if (cnew->port == port) {
            cnew->objnum = objnum;
            cnew->dead = 0;
            return 1;
        }
    }

    /* Get a server socket for the port. */
    server_socket = get_server_socket(port);
    if (server_socket < 0)
    return 0;

    cnew = EMALLOC(server_t, 1);
    cnew->server_socket = server_socket;
    cnew->client_socket = -1;
    cnew->port = port;
    cnew->objnum = objnum;
    cnew->dead = 0;
    cnew->next = servers;
    servers = cnew;

    return 1;
}

/*
// --------------------------------------------------------------------
*/
int remove_server(int port) {
    server_t **servp;

    for (servp = &servers; *servp; servp = &((*servp)->next)) {
        if ((*servp)->port == port) {
            (*servp)->dead = 1;
            return 1;
        }
    }

    return 0;
}

/*
// --------------------------------------------------------------------
*/
INTERNAL void connection_read(connection_t *conn) {
    unsigned char temp[BIGBUF];
    int len;
    Buffer *buf;
    data_t d;

    len = read(conn->fd, (char *) temp, BIGBUF);
    if (len < 0 && errno == EINTR) {
        /* We were interrupted; deal with this next time around. */
        return;
    }
    conn->flags.readable = 0;

    if (len <= 0) {
        /* The connection closed. */
        conn->flags.dead = 1;
        return;
    }

    /* We successfully read some data.  Handle it. */
    buf = buffer_new(len);
    MEMCPY(buf->s, temp, len);
    d.type = BUFFER;
    d.u.buffer = buf;
    task(conn->objnum, parse_id, 1, &d);
    buffer_discard(buf);
}

/*
// --------------------------------------------------------------------
*/
INTERNAL void connection_write(connection_t *conn) {
    Buffer *buf = conn->write_buf;
    int r;

    r = write(conn->fd, buf->s, buf->len);
    conn->flags.writable = 0;

    if (r <= 0) {
       /* We lost the connection. */
       conn->flags.dead = 1;
       buf = buffer_resize(buf, 0);
    } else {
       MEMMOVE(buf->s, buf->s + r, buf->len - r);
       buf = buffer_resize(buf, buf->len - r);
    }

    conn->write_buf = buf;
}

/*
// --------------------------------------------------------------------
*/
INTERNAL connection_t * connection_add(int fd, long objnum) {
    connection_t * conn;

    /* clear old connections to this objnum */
    for (conn = connections; conn; conn = conn->next) {
        if (conn->objnum == objnum && !conn->flags.dead)
            conn->flags.dead = 1;
    }

    /* initialize new connection */
    conn = EMALLOC(connection_t, 1);
    conn->fd = fd;
    conn->write_buf = buffer_new(0);
    conn->objnum = objnum;
    conn->flags.readable = 0;
    conn->flags.writable = 0;
    conn->flags.dead = 0;
    conn->next = connections;
    connections = conn;

    return conn;
}

/*
// --------------------------------------------------------------------
*/
INTERNAL void connection_discard(connection_t *conn) {
    object_t * obj;

    /* Notify connection object that the connection is gone. */
    task(conn->objnum, disconnect_id, 0);

    /* reset the conn variable on the object */
    obj = cache_retrieve(conn->objnum);
    if (obj != NULL) {
        obj->conn = NULL;
        cache_discard(obj);
    }

    /* Free the data associated with the connection. */
    close(conn->fd);
    buffer_discard(conn->write_buf);
    efree(conn);
}

/*
// --------------------------------------------------------------------
*/
INTERNAL void pend_discard(pending_t *pend) {
    efree(pend);
}

/*
// --------------------------------------------------------------------
*/
INTERNAL void server_discard(server_t *serv) {
    close(serv->server_socket);
}

/*
// --------------------------------------------------------------------
*/
long make_connection(char *addr, int port, objnum_t receiver) {
    pending_t *cnew;
    int socket;
    long result;

    result = non_blocking_connect(addr, port, &socket);
    if (result == address_id || result == socket_id)
    return result;
    cnew = TMALLOC(pending_t, 1);
    cnew->fd = socket;
    cnew->task_id = task_id;
    cnew->objnum = receiver;
    cnew->finished = 0;
    cnew->error = result;
    cnew->next = pendings;
    pendings = cnew;
    return NOT_AN_IDENT;
}

/*
// --------------------------------------------------------------------
// Write out everything in connections' write buffers.  Called by main()
// before exiting; does not modify the write buffers to reflect writing.
*/

void flush_output(void) {
    connection_t  * conn;
/*    filec_t        * file; */
    unsigned char * s;
    int len, r;

    /* do connections */
    for (conn = connections; conn; conn = conn->next) {
        s = conn->write_buf->s;
        len = conn->write_buf->len;
        while (len) {
            r = write(conn->fd, s, len);
            if (r <= 0)
            break;
            len -= r;
            s += r;
        }
    }

#if DISABLED
    /* do files */
    for (file = files; file; file = file->next)
        close_file(file);
#endif
}

#undef _io_

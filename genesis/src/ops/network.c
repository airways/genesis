/*
// Full copyright information is available in the file ../doc/CREDITS
*/

#include "defs.h"

#include <string.h>
#include "execute.h"
#include "net.h"
#include "util.h"
#include "cache.h"

/*
// -----------------------------------------------------------------
//
// If the current object has a connection, it will reassign that
// connection too the specified object.
//
*/

void func_reassign_connection(void) {
    cData       * args;
    Conn * c;
    Obj     * obj;

    /* Accept a objnum. */
    if (!func_init_1(&args, OBJNUM))
        return;

    c = find_connection(cur_frame->object);
    if (c) {
        obj = cache_retrieve(args[0].u.objnum);
        if (!obj) {
            cthrow(objnf_id, "Object #%l does not exist.", args[0].u.objnum);
            return;
        } else if (find_connection(obj)) {
            cthrow(perm_id, "Object %O already has a connection.", obj->objnum);
            return;
        }
        c->objnum = obj->objnum;
        cache_discard(obj);
        cur_frame->object->conn = NULL;
        pop(1);
        push_int(1);
    } else {
        pop(1);
        push_int(0);
    }
}

/*
// -----------------------------------------------------------------
*/
void func_bind_port(void) {
    cData * args;

    /* Accept a port to bind to, and a objnum to handle connections. */
    if (!func_init_1(&args, INTEGER))
        return;

    if (add_server(args[0].u.val, cur_frame->object->objnum))
        push_int(1);
    else if (server_failure_reason == socket_id)
        cthrow(socket_id, "Couldn't create server socket.");
    else /* (server_failure_reason == bind_id) */
        cthrow(bind_id, "Couldn't bind to port %d.", args[0].u.val);
}

/*
// -----------------------------------------------------------------
*/
void func_unbind_port(void) {
    cData * args;

    /* Accept a port number. */
    if (!func_init_1(&args, INTEGER))
        return;

    if (!remove_server(args[0].u.val))
        cthrow(servnf_id, "No server socket on port %d.", args[0].u.val);
    else
        push_int(1);
}

/*
// -----------------------------------------------------------------
*/
void func_open_connection(void) {
    cData *args;
    char *address;
    Int port;
    Long r;

    if (!func_init_2(&args, STRING, INTEGER))
        return;

    address = string_chars(args[0].u.str);
    port = args[1].u.val;

    r = make_connection(address, port, cur_frame->object->objnum);
    if (r == address_id)
        cthrow(address_id, "Invalid address");
    else if (r == socket_id)
        cthrow(socket_id, "Couldn't create socket for connection");
    pop(3);
    push_int(1);
}

/*
// -----------------------------------------------------------------
*/
void func_close_connection(void) {
    /* Accept no arguments. */
    if (!func_init_0())
        return;

    /* Kick off anyone assigned to the current object. */
    push_int(boot(cur_frame->object));
}

/*
// -----------------------------------------------------------------
// Echo a buffer to the connection
*/
void func_cwrite(void) {
    cData *args;

    /* Accept a buffer to write. */
    if (!func_init_1(&args, BUFFER))
        return;

    /* Write the string to any connection associated with this object.  */
    tell(cur_frame->object, args[0].u.buffer);

    pop(1);
    push_int(1);
}

/*
// -----------------------------------------------------------------
// write a file to the connection
*/
void func_cwritef(void) {
    size_t        block, r;
    cData      * args;
    FILE        * fp;
    cBuf    * buf;
    cStr    * str;
    struct stat   statbuf;
    Int           nargs;

    /* Accept the name of a file to echo */
    if (!func_init_1_or_2(&args, &nargs, STRING, INTEGER))
        return;

    /* Initialize the file */
    str = build_path(args[0].u.str->s, &statbuf, DISALLOW_DIR);
    if (str == NULL)
        return;

    /* Open the file for reading. */
    fp = open_scratch_file(str->s, "rb");
    if (!fp) {
        cthrow(file_id, "Cannot open file \"%s\" for reading.", str->s);
        return;
    }

    /* how big of a chunk do we read at a time? */
    if (nargs == 2) {
        if (args[1].u.val == -1)
            block = statbuf.st_size;
        else
            block = (size_t) args[1].u.val;
    } else
        block = (size_t) DEF_BLOCKSIZE;

    /* Allocate a buffer to hold the block */
    buf = buffer_new(block);

    while (!feof(fp)) {
        r = fread(buf->s, sizeof(unsigned char), block, fp);
        if (r != block && !feof(fp)) {
            buffer_discard(buf);
            close_scratch_file(fp);
            cthrow(file_id, "Trouble reading file \"%s\": %s",
                   str->s, strerror(GETERR()));
            return;
        }
        tell(cur_frame->object, buf);
    }

    /* Discard the buffer and close the file. */
    buffer_discard(buf);
    close_scratch_file(fp);

    pop(nargs);
    push_int((cNum) statbuf.st_size);
}

/*
// -----------------------------------------------------------------
// return random info on the connection
*/
void func_connection(void) {
    cList       * info;
    cData       * list;
    Conn * c;

    if (!func_init_0())
        return;

    c = find_connection(cur_frame->object);
    if (!c) {
        cthrow(net_id, "No connection established.");
        return;
    }

    info = list_new(4);
    list = list_empty_spaces(info, 4);

    list[0].type = INTEGER;
    list[0].u.val = (cNum) (c->flags.readable ? 1 : 0);
    list[1].type = INTEGER;
    list[1].u.val = (cNum) (c->flags.writable ? 1 : 0);
    list[2].type = INTEGER;
    list[2].u.val = (cNum) (c->flags.dead ? 1 : 0);
    list[3].type = INTEGER;
    list[3].u.val = (cNum) (c->fd);

    push_list(info);
    list_discard(info);
}

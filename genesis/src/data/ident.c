/*
// Full copyright information is available in the file ../doc/CREDITS
*/

#include "defs.h"

#include <string.h>
#include "util.h"
#include "string_tab.h"

StringTab *idents;

Ident perm_id, type_id, div_id, integer_id, float_id, string_id, objnum_id,
      list_id, symbol_id, error_id, frob_id, methodnf_id, methoderr_id,
      parent_id, maxdepth_id, objnf_id, numargs_id, range_id, varnf_id,
      file_id, ticks_id, connect_id, disconnect_id, startup_id, parse_id,
      socket_id, bind_id, servnf_id, varexists_id, dictionary_id, keynf_id,
      address_id, refused_id, net_id, timeout_id, other_id, failed_id,
      heartbeat_id, regexp_id, buffer_id, object_id, namenf_id, salt_id,
      function_id, opcode_id, method_id, interpreter_id, signal_id,
      directory_id, eof_id, backup_done_id;

Ident public_id, protected_id, private_id, root_id, driver_id, fpe_id, inf_id,
      noover_id, sync_id, locked_id, native_id, forked_id, atomic_id;

Ident SEEK_SET_id, SEEK_CUR_id, SEEK_END_id, preaddr_id, pretype_id;
Ident breadth_id, depth_id, full_id, partial_id;

/* limits */
Ident datasize_id, forkdepth_id, calldepth_id, recursion_id, objswap_id;
Ident left_id, right_id, both_id;

/* config options */
Ident cachelog_id, cachewatch_id, cachewatchcount_id, cleanerwait_id, cleanerignore_id;
Ident log_malloc_size_id, log_method_cache_id, cache_history_size_id;

/* cache stats options */
Ident ancestor_cache_id, method_cache_id, name_cache_id, object_cache_id;

void init_ident(void)
{
    idents = string_tab_new();

    perm_id = ident_get("perm");
    type_id = ident_get("type");
    div_id = ident_get("div");
    integer_id = ident_get("integer");
    float_id = ident_get("float");
    fpe_id = ident_get("fpe");
    inf_id = ident_get("inf");
    string_id = ident_get("string");
    objnum_id = ident_get("objnum");
    list_id = ident_get("list");
    symbol_id = ident_get("symbol");
    error_id = ident_get("error");
    frob_id = ident_get("frob");
    methodnf_id = ident_get("methodnf");
    methoderr_id = ident_get("methoderr");
    parent_id = ident_get("parent");
    maxdepth_id = ident_get("maxdepth");
    objnf_id = ident_get("objnf");
    numargs_id = ident_get("numargs");
    range_id = ident_get("range");
    varnf_id = ident_get("varnf");
    file_id = ident_get("file");
    ticks_id = ident_get("ticks");
    connect_id = ident_get("connect");
    disconnect_id = ident_get("disconnect");
    parse_id = ident_get("parse");
    startup_id = ident_get("startup");
    socket_id = ident_get("socket");
    bind_id = ident_get("bind");
    servnf_id = ident_get("servnf");
    varexists_id = ident_get("varexists");
    dictionary_id = ident_get("dictionary");
    keynf_id = ident_get("keynf");
    address_id = ident_get("address");
    refused_id = ident_get("refused");
    net_id = ident_get("net");
    timeout_id = ident_get("timeout");
    other_id = ident_get("other");
    failed_id = ident_get("failed");
    heartbeat_id = ident_get("heartbeat");
    regexp_id = ident_get("regexp");
    buffer_id = ident_get("buffer");
    object_id = ident_get("object");
    namenf_id = ident_get("namenf");
    salt_id = ident_get("salt");
    function_id = ident_get("function");
    opcode_id = ident_get("opcode");
    method_id = ident_get("method");
    interpreter_id = ident_get("interpreter");
    signal_id = ident_get("signal");
    directory_id = ident_get("directory");
    eof_id = ident_get("eof");
    public_id = ident_get("public");
    protected_id = ident_get("protected");
    private_id = ident_get("private");
    root_id = ident_get("root");
    driver_id = ident_get("driver");
    noover_id = ident_get("nooverride");
    forked_id = ident_get("forked");
    sync_id = ident_get("synchronized");
    locked_id = ident_get("locked");
    native_id = ident_get("native");
    atomic_id = ident_get("atomic");
    backup_done_id = ident_get("backup_done");
    SEEK_SET_id = ident_get("SEEK_SET");
    SEEK_CUR_id = ident_get("SEEK_CUR");
    SEEK_END_id = ident_get("SEEK_END");
    preaddr_id = ident_get("preaddr");
    pretype_id = ident_get("pretype");

    datasize_id = ident_get("datasize");
    forkdepth_id = ident_get("forkdepth");
    recursion_id = ident_get("recursion");
    objswap_id = ident_get("objswap");
    calldepth_id = ident_get("calldepth");

    cachelog_id = ident_get("cachelog");
    cachewatch_id = ident_get("cachewatch");
    cachewatchcount_id = ident_get("cachewatchcount");
    cleanerwait_id = ident_get("cleanerwait");
    cleanerignore_id = ident_get("cleanerignore");

    log_malloc_size_id = ident_get("log_malloc_size");
    log_method_cache_id = ident_get("log_method_cache");
    cache_history_size_id = ident_get("cache_history_size");

    ancestor_cache_id = ident_get("ancestor_cache");
    method_cache_id = ident_get("method_cache");
    name_cache_id = ident_get("name_cache");
    object_cache_id = ident_get("object_cache");

    left_id = ident_get("left");
    right_id = ident_get("right");
    both_id = ident_get("both");
    breadth_id = ident_get("breadth");
    depth_id = ident_get("depth");
    full_id = ident_get("full");
    partial_id = ident_get("partial");
}

void uninit_ident(void)
{
    string_tab_free(idents);
}


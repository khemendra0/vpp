/*
 * Copyright (c) 2016 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 *------------------------------------------------------------------
 * ioam_cache.c - ioam ip6 API / debug CLI handling
 *------------------------------------------------------------------
 */

#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include <ioam/ip6/ioam_cache.h>

#include <vlibapi/api.h>
#include <vlibmemory/api.h>
#include <vlibsocket/api.h>
#include <vnet/ip/ip6_hop_by_hop.h>

#include "ioam_cache.h"

/* define message IDs */
#include <ioam/ip6/ioam_cache_msg_enum.h>

/* define message structures */
#define vl_typedefs
#include <ioam/ip6/ioam_cache_all_api_h.h>
#undef vl_typedefs

/* define generated endian-swappers */
#define vl_endianfun
#include <ioam/ip6/ioam_cache_all_api_h.h>
#undef vl_endianfun

/* instantiate all the print functions we know about */
#define vl_print(handle, ...) vlib_cli_output (handle, __VA_ARGS__)
#define vl_printfun
#include <ioam/ip6/ioam_cache_all_api_h.h>
#undef vl_printfun

/* Get the API version number */
#define vl_api_version(n,v) static u32 api_version=(v);
#include <ioam/ip6/ioam_cache_all_api_h.h>
#undef vl_api_version

/*
 * A handy macro to set up a message reply.
 * Assumes that the following variables are available:
 * mp - pointer to request message
 * rmp - pointer to reply message type
 * rv - return value
 */

#define REPLY_MACRO(t)                                          \
do {                                                            \
    unix_shared_memory_queue_t * q =                            \
    vl_api_client_index_to_input_queue (mp->client_index);      \
    if (!q)                                                     \
        return;                                                 \
                                                                \
    rmp = vl_msg_api_alloc (sizeof (*rmp));                     \
    rmp->_vl_msg_id = ntohs((t)+cm->msg_id_base);               \
    rmp->context = mp->context;                                 \
    rmp->retval = ntohl(rv);                                    \
                                                                \
    vl_msg_api_send_shmem (q, (u8 *)&rmp);                      \
} while(0);


/* List of message types that this plugin understands */

#define foreach_ioam_cache_plugin_api_msg                        \
_(IOAM_CACHE_IP6_ENABLE_DISABLE, ioam_cache_ip6_enable_disable)

/*
 * This routine exists to convince the vlib plugin framework that
 * we haven't accidentally copied a random .dll into the plugin directory.
 *
 * Also collects global variable pointers passed from the vpp engine
 */

clib_error_t *
vlib_plugin_register (vlib_main_t * vm, vnet_plugin_handoff_t * h,
		      int from_early_init)
{
  ioam_cache_main_t *em = &ioam_cache_main;
  clib_error_t *error = 0;

  em->vlib_main = vm;
  em->vnet_main = h->vnet_main;

  return error;
}


static u8 * ioam_e2e_id_trace_handler (u8 * s,
                                    ip6_hop_by_hop_option_t *opt)
{
  ioam_e2e_id_option_t * e2e = (ioam_e2e_id_option_t *)opt;

  if (e2e)
    {
      s = format (s, "IP6_HOP_BY_HOP E2E ID = %U", format_ip6_address, &(e2e->id));
    }


  return s;
}

/* Action function shared between message handler and debug CLI */
int
ioam_cache_ip6_enable_disable (ioam_cache_main_t * em,
			       u8 is_disable)
{
  vlib_main_t *vm = em->vlib_main;

  if (is_disable == 0)
    {
        ioam_cache_table_init(vm);
	ip6_hbh_set_next_override (em->cache_hbh_slot);
	ip6_hbh_register_option(HBH_OPTION_TYPE_IOAM_EDGE_TO_EDGE_ID,
                          0,
                          ioam_e2e_id_trace_handler);

    }
  else
    {
      ip6_hbh_set_next_override (IP6_LOOKUP_NEXT_POP_HOP_BY_HOP); 
      ioam_cache_table_destroy(vm);
      ip6_hbh_unregister_option(HBH_OPTION_TYPE_IOAM_EDGE_TO_EDGE_ID);
    }

  return 0;
}
/* Action function shared between message handler and debug CLI */
int
ioam_tunnel_select_ip6_enable_disable (ioam_cache_main_t * em,
			       u8 is_disable)
{
  vlib_main_t *vm = em->vlib_main;

  if (is_disable == 0)
    {
        ioam_cache_ts_table_init(vm);
	ip6_hbh_set_next_override (em->ts_hbh_slot);
	ip6_hbh_register_option(HBH_OPTION_TYPE_IOAM_EDGE_TO_EDGE_ID,
                          0,
                          ioam_e2e_id_trace_handler);

    }
  else
    {
      ip6_hbh_set_next_override (IP6_LOOKUP_NEXT_POP_HOP_BY_HOP); 
      ioam_cache_ts_table_destroy(vm);
      ip6_hbh_unregister_option(HBH_OPTION_TYPE_IOAM_EDGE_TO_EDGE_ID);
    }

  return 0;
}

/* API message handler */
static void vl_api_ioam_cache_ip6_enable_disable_t_handler
  (vl_api_ioam_cache_ip6_enable_disable_t * mp)
{
  vl_api_ioam_cache_ip6_enable_disable_reply_t *rmp;
  ioam_cache_main_t *cm = &ioam_cache_main;
  int rv;

  rv = ioam_cache_ip6_enable_disable (cm, (int) (mp->is_disable));
  REPLY_MACRO (VL_API_IOAM_CACHE_IP6_ENABLE_DISABLE_REPLY);
}

/* Set up the API message handling tables */
static clib_error_t *
ioam_cache_plugin_api_hookup (vlib_main_t * vm)
{
  ioam_cache_main_t *sm = &ioam_cache_main;
#define _(N,n)                                                  \
    vl_msg_api_set_handlers((VL_API_##N + sm->msg_id_base),     \
                           #n,					\
                           vl_api_##n##_t_handler,              \
                           vl_noop_handler,                     \
                           vl_api_##n##_t_endian,               \
                           vl_api_##n##_t_print,                \
                           sizeof(vl_api_##n##_t), 1);
  foreach_ioam_cache_plugin_api_msg;
#undef _

  return 0;
}

static clib_error_t *
set_ioam_cache_command_fn (vlib_main_t * vm,
				  unformat_input_t * input,
				  vlib_cli_command_t * cmd)
{
  ioam_cache_main_t *em = &ioam_cache_main;
  u8 is_disable = 0;

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "disable"))
	is_disable = 1;
      else
	break;
    }

  /* Turn on the ip6 timer process */
  // vlib_process_signal_event (vm, flow_report_process_node.index,
  //1, 0);
  ioam_cache_ip6_enable_disable (em, is_disable);

  return 0;
}

/* *INDENT_OFF* */
VLIB_CLI_COMMAND (set_ioam_cache_command, static) =
{
  .path = "set ioam ip6 cache",
  .short_help =
    "set ioam ip6 cache [disable]",
  .function = set_ioam_cache_command_fn
};
/* *INDENT_ON* */

static clib_error_t *
set_ioam_tunnel_select_command_fn (vlib_main_t * vm,
				  unformat_input_t * input,
				  vlib_cli_command_t * cmd)
{
  ioam_cache_main_t *em = &ioam_cache_main;
  u8 is_disable = 0;

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "disable"))
	is_disable = 1;
      else
	break;
    }

  /* Turn on the ip6 timer process */
  // vlib_process_signal_event (vm, flow_report_process_node.index,
  //1, 0);
  ioam_tunnel_select_ip6_enable_disable (em, is_disable);

  return 0;
}

/* *INDENT_OFF* */
VLIB_CLI_COMMAND (set_ioam_cache_ts_command, static) =
{
  .path = "set ioam ip6 sr-tunnel-select",
  .short_help =
    "set ioam ip6 sr-tunnel-select [disable]",
  .function = set_ioam_tunnel_select_command_fn
};
/* *INDENT_ON* */

static void ioam_cache_table_print (vlib_main_t *vm)
{
  ioam_cache_main_t *cm = &ioam_cache_main;
  ioam_cache_entry_t *entry = 0;  
  ioam_cache_ts_entry_t *ts_entry = 0;  

  pool_foreach (entry, cm->ioam_rewrite_pool,
		({
		    vlib_cli_output (vm, "%U",
				format_ioam_cache_entry, entry);
		}));
  pool_foreach (ts_entry, cm->ioam_ts_pool,
		({
		    vlib_cli_output (vm, "%U",
				format_ioam_cache_ts_entry, ts_entry);
		}));
  
}

static clib_error_t *
show_ioam_cache_command_fn (vlib_main_t * vm,
				  unformat_input_t * input,
				  vlib_cli_command_t * cmd)
{
  ioam_cache_table_print(vm);
  return 0;
}

/* *INDENT_OFF* */
VLIB_CLI_COMMAND (show_ioam_cache_command, static) =
{
  .path = "show ioam ip6 cache",
  .short_help =
    "show ioam ip6 cache",
  .function = show_ioam_cache_command_fn
};
/* *INDENT_ON* */

static clib_error_t *
ioam_cache_init (vlib_main_t * vm)
{
  ioam_cache_main_t *em = &ioam_cache_main;
  clib_error_t *error = 0;
  u8 *name;
  u32 cache_node_index = ioam_cache_node.index;
  u32 ts_node_index = ioam_cache_ts_node.index;
  vlib_node_t *ip6_hbyh_node = NULL, *ip6_hbh_pop_node = NULL, *error_node = NULL;

  name = format (0, "ioam_cache_%08x%c", api_version, 0);

  /* Ask for a correctly-sized block of API message decode slots */
  em->msg_id_base = vl_msg_api_get_msg_ids
    ((char *) name, VL_MSG_FIRST_AVAILABLE);

  error = ioam_cache_plugin_api_hookup (vm);

  /* Hook this node to ip6-hop-by-hop */
  ip6_hbyh_node = vlib_get_node_by_name (vm, (u8 *) "ip6-hop-by-hop");
  em->cache_hbh_slot = vlib_node_add_next (vm, ip6_hbyh_node->index, cache_node_index);
  em->ts_hbh_slot = vlib_node_add_next (vm, ip6_hbyh_node->index, ts_node_index);

  ip6_hbh_pop_node = vlib_get_node_by_name (vm, (u8 *) "ip6-pop-hop-by-hop");
  em->ip6_hbh_pop_node_index = ip6_hbh_pop_node->index;

  error_node = vlib_get_node_by_name (vm, (u8 *) "error-drop");
  em->error_node_index = error_node->index;

  vec_free (name);

  return error;
}

VLIB_INIT_FUNCTION (ioam_cache_init);
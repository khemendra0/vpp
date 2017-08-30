/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <typeinfo>
#include <cassert>
#include <iostream>

#include "vom/interface.hpp"
#include "vom/cmd.hpp"

DEFINE_VAPI_MSG_IDS_VPE_API_JSON;
DEFINE_VAPI_MSG_IDS_INTERFACE_API_JSON;
DEFINE_VAPI_MSG_IDS_AF_PACKET_API_JSON;
DEFINE_VAPI_MSG_IDS_TAP_API_JSON;

using namespace VOM;

interface::loopback_create_cmd::loopback_create_cmd(HW::item<handle_t> &item,
                                                    const std::string &name):
    create_cmd(item, name)
{
}

rc_t interface::loopback_create_cmd::issue(connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    VAPI_CALL(req.execute());

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        interface::add(m_name, m_hw_item);
    }

    return rc_t::OK;
}
std::string interface::loopback_create_cmd::to_string() const
{
    std::ostringstream s;
    s << "loopback-itf-create: " << m_hw_item.to_string()
      << " name:" << m_name;

    return (s.str());
}

interface::af_packet_create_cmd::af_packet_create_cmd(HW::item<handle_t> &item,
                                                      const std::string &name):
    create_cmd(item, name)
{
}

rc_t interface::af_packet_create_cmd::issue(connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();

    payload.use_random_hw_addr = 1;
    memset(payload.host_if_name, 0,
           sizeof(payload.host_if_name));
    memcpy(payload.host_if_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(payload.host_if_name)));

    VAPI_CALL(req.execute());

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        interface::add(m_name, m_hw_item);
    }

    return rc_t::OK;
}
std::string interface::af_packet_create_cmd::to_string() const
{
    std::ostringstream s;
    s << "af-packet-itf-create: " << m_hw_item.to_string()
      << " name:" << m_name;

    return (s.str());
}

interface::tap_create_cmd::tap_create_cmd(HW::item<handle_t> &item,
                                          const std::string &name):
    create_cmd(item, name)
{
}

rc_t interface::tap_create_cmd::issue(connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();

    memset(payload.tap_name, 0,
           sizeof(payload.tap_name));
    memcpy(payload.tap_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(payload.tap_name)));
    payload.use_random_mac = 1;

    VAPI_CALL(req.execute());

    m_hw_item = wait();

    if (m_hw_item.rc() == rc_t::OK)
    {
        interface::add(m_name, m_hw_item);
    }

    return rc_t::OK;
}

std::string interface::tap_create_cmd::to_string() const
{
    std::ostringstream s;
    s << "tap-intf-create: " << m_hw_item.to_string()
      << " name:" << m_name;

    return (s.str());
}

interface::loopback_delete_cmd::loopback_delete_cmd(HW::item<handle_t> &item):
    delete_cmd(item)
{
}

rc_t interface::loopback_delete_cmd::issue(connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_hw_item.data().value();

    VAPI_CALL(req.execute());

    wait();
    m_hw_item.set(rc_t::NOOP);

    interface::remove(m_hw_item);
    return rc_t::OK;
}

std::string interface::loopback_delete_cmd::to_string() const
{
    std::ostringstream s;
    s << "loopback-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

interface::af_packet_delete_cmd::af_packet_delete_cmd(HW::item<handle_t> &item,
                                                      const std::string &name):
    delete_cmd(item, name)
{
}

rc_t interface::af_packet_delete_cmd::issue(connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    memset(payload.host_if_name, 0,
           sizeof(payload.host_if_name));
    memcpy(payload.host_if_name, m_name.c_str(),
           std::min(m_name.length(),
                    sizeof(payload.host_if_name)));

    VAPI_CALL(req.execute());

    wait();
    m_hw_item.set(rc_t::NOOP);

    interface::remove(m_hw_item);
    return rc_t::OK;
}
std::string interface::af_packet_delete_cmd::to_string() const
{
    std::ostringstream s;
    s << "af_packet-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

interface::tap_delete_cmd::tap_delete_cmd(HW::item<handle_t> &item):
    delete_cmd(item)
{
}

rc_t interface::tap_delete_cmd::issue(connection &con)
{
    // finally... call VPP

    interface::remove(m_hw_item);
    return rc_t::OK;
}
std::string interface::tap_delete_cmd::to_string() const
{
    std::ostringstream s;
    s << "tap-itf-delete: " << m_hw_item.to_string();

    return (s.str());
}

interface::state_change_cmd::state_change_cmd(HW::item<interface::admin_state_t> &state,
                                              const HW::item<handle_t> &hdl):
    rpc_cmd(state),
    m_hdl(hdl)
{
}

bool interface::state_change_cmd::operator==(const state_change_cmd& other) const
{
    return ((m_hdl == other.m_hdl) &&
            (m_hw_item == other.m_hw_item));
}

rc_t interface::state_change_cmd::issue(connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_hdl.data().value();
    payload.admin_up_down = m_hw_item.data().value();

    VAPI_CALL(req.execute());

    m_hw_item.set(wait());

    return rc_t::OK;
}

std::string interface::state_change_cmd::to_string() const
{
    std::ostringstream s;
    s << "itf-state-change: " << m_hw_item.to_string()
      << " hdl:" << m_hdl.to_string();
    return (s.str());
}

interface::set_table_cmd::set_table_cmd(HW::item<route::table_id_t> &table,
                                        const HW::item<handle_t> &hdl):
    rpc_cmd(table),
    m_hdl(hdl)
{
}

bool interface::set_table_cmd::operator==(const set_table_cmd& other) const
{
    return ((m_hdl == other.m_hdl) &&
            (m_hw_item == other.m_hw_item));
}

rc_t interface::set_table_cmd::issue(connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.sw_if_index = m_hdl.data().value();
    payload.is_ipv6 = 0;
    payload.vrf_id = m_hw_item.data();

    VAPI_CALL(req.execute());

    m_hw_item.set(wait());

    return (rc_t::OK);
}

std::string interface::set_table_cmd::to_string() const
{
    std::ostringstream s;
    s << "itf-state-change: " << m_hw_item.to_string()
      << " hdl:" << m_hdl.to_string();
    return (s.str());
}


interface::events_cmd::events_cmd(event_listener &el):
    rpc_cmd(el.status()),
    event_cmd(),
    m_listener(el)
{
}

bool interface::events_cmd::operator==(const events_cmd& other) const
{
    return (true);
}

rc_t interface::events_cmd::issue(connection &con)
{
    /*
     * First set the clal back to handle the interface events
     */
    m_reg.reset(new reg_t(con.ctx(), std::ref(*(static_cast<event_cmd*>(this)))));
    // m_reg->execute();

    /*
     * then send the request to enable them
     */
    msg_t req(con.ctx(), std::ref(*(static_cast<rpc_cmd*>(this))));

    auto &payload = req.get_request().get_payload();
    payload.enable_disable = 1;
    payload.pid = getpid();

    VAPI_CALL(req.execute());

    wait();

    return (rc_t::INPROGRESS);
}

void interface::events_cmd::retire()
{
}

void interface::events_cmd::notify()
{
    m_listener.handle_interface_event(this);
}

std::string interface::events_cmd::to_string() const
{
    return ("itf-events");
}

interface::dump_cmd::dump_cmd()
{
}

bool interface::dump_cmd::operator==(const dump_cmd& other) const
{
    return (true);
}

rc_t interface::dump_cmd::issue(connection &con)
{
    m_dump.reset(new msg_t(con.ctx(), std::ref(*this)));

    auto &payload = m_dump->get_request().get_payload();
    payload.name_filter_valid = 0;

    VAPI_CALL(m_dump->execute());

    wait();

    return rc_t::OK;
}

std::string interface::dump_cmd::to_string() const
{
    return ("itf-dump");
}

interface::set_tag::set_tag(HW::item<handle_t> &item,
                            const std::string &name):
    rpc_cmd(item),
    m_name(name)
{
}

rc_t interface::set_tag::issue(connection &con)
{
    msg_t req(con.ctx(), std::ref(*this));

    auto &payload = req.get_request().get_payload();
    payload.is_add = 1;
    payload.sw_if_index = m_hw_item.data().value();
    memcpy(payload.tag, m_name.c_str(), m_name.length());

    VAPI_CALL(req.execute());

    wait();

    return rc_t::OK;
}
std::string interface::set_tag::to_string() const
{
    std::ostringstream s;
    s << "itf-set-tag: " << m_hw_item.to_string()
      << " name:" << m_name;

    return (s.str());
}

bool interface::set_tag::operator==(const set_tag& o) const
{
    return ((m_name == o.m_name) &&
            (m_hw_item.data() == o.m_hw_item.data()));
}
/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VOM_BRIDGE_DOMAIN_ARP_ENTRY_H__
#define __VOM_BRIDGE_DOMAIN_ARP_ENTRY_H__

#include "vom/types.hpp"
#include "vom/bridge_domain.hpp"
#include "vom/interface.hpp"
#include "vom/singular_db.hpp"

#include <vapi/l2.api.vapi.hpp>

namespace VOM
{
    /**
     * A entry in the ARP termination table of a Bridge Domain
     */
    class bridge_domain_arp_entry: public object_base
    {
    public:
        /**
         * The key for a bridge_domain ARP entry;
         *  the BD, IP address and MAC address
         */
        typedef std::tuple<uint32_t, mac_address_t, boost::asio::ip::address> key_t;

        /**
         * Construct a bridge_domain in the given bridge domain
         */
        bridge_domain_arp_entry(const bridge_domain &bd,
                                const mac_address_t &mac,
                                const boost::asio::ip::address &ip_addr);

        /**
         * Construct a bridge_domain in the default table
         */
        bridge_domain_arp_entry(const mac_address_t &mac,
                                const boost::asio::ip::address &ip_addr);

        /**
         * Copy Construct
         */
        bridge_domain_arp_entry(const bridge_domain_arp_entry &r);

        /**
         * Destructor
         */
        ~bridge_domain_arp_entry();

        /**
         * Return the matching 'singular instance'
         */
        std::shared_ptr<bridge_domain_arp_entry> singular() const;

        /**
         * Find the instnace of the bridge_domain domain in the OM
         */
        static std::shared_ptr<bridge_domain_arp_entry> find(const bridge_domain_arp_entry &temp);

        /**
         * Dump all bridge_domain-doamin into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

        /**
         * Convert to string for debugging
         */
        std::string to_string() const;

        /**
         * A command class that creates or updates the bridge domain ARP Entry
         */
        class create_cmd: public rpc_cmd<HW::item<bool>, rc_t,
                                         vapi::Bd_ip_mac_add_del>
        {
        public:
            /**
             * Constructor
             */
            create_cmd(HW::item<bool> &item,
                       uint32_t id,
                       const mac_address_t &mac,
                       const boost::asio::ip::address &ip_addr);

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(connection &con);

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const create_cmd&i) const;

        private:
            uint32_t m_bd;
            mac_address_t m_mac;
            boost::asio::ip::address m_ip_addr;
        };

        /**
         * A cmd class that deletes a bridge domain ARP entry
         */
        class delete_cmd: public rpc_cmd<HW::item<bool>, rc_t,
                                         vapi::Bd_ip_mac_add_del>
        {
        public:
            /**
             * Constructor
             */
            delete_cmd(HW::item<bool> &item,
                       uint32_t id,
                       const mac_address_t &mac,
                       const boost::asio::ip::address &ip_addr);

            /**
             * Issue the command to VPP/HW
             */
            rc_t issue(connection &con);

            /**
             * convert to string format for debug purposes
             */
            std::string to_string() const;

            /**
             * Comparison operator - only used for UT
             */
            bool operator==(const delete_cmd&i) const;

        private:
            uint32_t m_bd;
            mac_address_t m_mac;
            boost::asio::ip::address m_ip_addr;
        };

    private:
        /**
         * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
         */
        void update(const bridge_domain_arp_entry &obj);

        /**
         * Find or add the instnace of the bridge_domain domain in the OM
         */
        static std::shared_ptr<bridge_domain_arp_entry> find_or_add(const bridge_domain_arp_entry &temp);

        /*
         * It's the VPPHW class that updates the objects in HW
         */
        friend class VOM::OM;

        /**
         * It's the VOM::singular_db class that calls replay()
         */
        friend class VOM::singular_db<key_t, bridge_domain_arp_entry>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * HW configuration for the result of creating the bridge_domain
         */
        HW::item<bool> m_hw;

        /**
         * The bridge_domain domain the bridge_domain is in.
         */
        std::shared_ptr<bridge_domain> m_bd;

        /**
         * The mac to match
         */
        mac_address_t m_mac;

        /**
         * The IP address
         */
        boost::asio::ip::address m_ip_addr;

        /**
         * A map of all bridge_domains
         */
        static singular_db<key_t, bridge_domain_arp_entry> m_db;
    };

    std::ostream & operator<<(std::ostream &os,
                              const bridge_domain_arp_entry::key_t &key);
};

#endif

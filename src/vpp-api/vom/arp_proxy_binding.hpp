/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __VOM_ARP_PROXY_BINDING_H__
#define __VOM_ARP_PROXY_BINDING_H__

#include "vom/object_base.hpp"
#include "vom/om.hpp"
#include "vom/hw.hpp"
#include "vom/rpc_cmd.hpp"
#include "vom/dump_cmd.hpp"
#include "vom/singular_db.hpp"
#include "vom/interface.hpp"
#include "vom/arp_proxy_config.hpp"
#include "vom/inspect.hpp"

#include <vapi/vpe.api.vapi.hpp>

namespace VOM
{
    /**
     * A representation of LLDP client configuration on an interface
     */
    class arp_proxy_binding: public object_base
    {
    public:
        /**
         * Construct a new object matching the desried state
         */
        arp_proxy_binding(const interface &itf,
                          const arp_proxy_config &proxy_cfg);

        /**
         * Copy Constructor
         */
        arp_proxy_binding(const arp_proxy_binding& o);

        /**
         * Destructor
         */
        ~arp_proxy_binding();


        /**
         * Return the 'singular' of the LLDP binding that matches this object
         */
        std::shared_ptr<arp_proxy_binding> singular() const;

        /**
         * convert to string format for debug purposes
         */
        std::string to_string() const;

        /**
         * Dump all LLDP bindings into the stream provided
         */
        static void dump(std::ostream &os);

        /**
         * A command class that binds the LLDP config to the interface
         */
        class bind_cmd: public rpc_cmd<HW::item<bool>, rc_t,
                                       vapi::Proxy_arp_intfc_enable_disable>
        {
        public:
            /**
             * Constructor
             */
            bind_cmd(HW::item<bool> &item,
                     const handle_t &itf);

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
            bool operator==(const bind_cmd&i) const;
        private:
            /**
             * Reference to the HW::item of the interface to bind
             */
            const handle_t &m_itf;
        };

        /**
         * A cmd class that Unbinds ArpProxy Config from an interface
         */
        class unbind_cmd: public rpc_cmd<HW::item<bool>, rc_t,
                                         vapi::Proxy_arp_intfc_enable_disable>
        {
        public:
            /**
             * Constructor
             */
            unbind_cmd(HW::item<bool> &item,
                       const handle_t &itf);

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
            bool operator==(const unbind_cmd&i) const;
        private:
            /**
             * Reference to the HW::item of the interface to unbind
             */
            const handle_t &m_itf;
        };

    private:
        /**
         * Class definition for listeners to OM events
         */
        class event_handler: public OM::listener, public inspect::command_handler
        {
        public:
            event_handler();
            virtual ~event_handler() = default;

            /**
             * Handle a populate event
             */
            void handle_populate(const client_db::key_t & key);

            /**
             * Handle a replay event
             */
            void handle_replay();

            /**
             * Show the object in the Singular DB
             */
            void show(std::ostream &os);

            /**
             * Get the sortable Id of the listener
             */
            dependency_t order() const;
        };

        /**
         * event_handler to register with OM
         */
        static event_handler m_evh;

        /**
         * Enquue commonds to the VPP command Q for the update
         */
        void update(const arp_proxy_binding &obj);

        /**
         * Find or add LLDP binding to the OM
         */
        static std::shared_ptr<arp_proxy_binding> find_or_add(const arp_proxy_binding &temp);

        /*
         * It's the VOM::OM class that calls singular()
         */
        friend class VOM::OM;

        /**
         * It's the VOM::singular_db class that calls replay()
         */
        friend class VOM::singular_db<interface::key_type, arp_proxy_binding>;

        /**
         * Sweep/reap the object if still stale
         */
        void sweep(void);

        /**
         * replay the object to create it in hardware
         */
        void replay(void);

        /**
         * A reference counting pointer to the interface on which LLDP config
         * resides. By holding the reference here, we can guarantee that
         * this object will outlive the interface
         */
        const std::shared_ptr<interface> m_itf;

        /**
         * A reference counting pointer to the prxy config.
         */
        const std::shared_ptr<arp_proxy_config> m_arp_proxy_cfg;

        /**
         * HW configuration for the binding. The bool representing the
         * do/don't bind.
         */
        HW::item<bool> m_binding;

        /**
         * A map of all ArpProxy bindings keyed against the interface.
         */
        static singular_db<interface::key_type, arp_proxy_binding> m_db;
    };
};

#endif
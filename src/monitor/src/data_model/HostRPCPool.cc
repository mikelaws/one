/* -------------------------------------------------------------------------- */
/* Copyright 2002-2019, OpenNebula Project, OpenNebula Systems                */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#include "HostRPCPool.h"
#include "NebulaLog.h"

const char * monit_table = "host_monitoring_new";
const char * monit_db_names = "hid, timestamp, body";

int HostRPCPool::load_info(xmlrpc_c::value &result)
{
    try
    {
        client->call("one.hostpool.info", "", &result);

        return 0;
    }
    catch (exception const& e)
    {
        ostringstream   oss;
        oss << "Exception raised: " << e.what();

        NebulaLog::log("HOST", Log::ERROR, oss);

        return -1;
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HostRPCPool::update_monitoring(HostBase* h)
{
    const auto& monitoring = h->monitoring();
    auto sql_xml = db->escape_str(monitoring.to_xml());

    if (sql_xml == 0)
    {
        NebulaLog::log("HPL", Log::WARNING, "Could not transform Host monitoring to XML");
        return -1;
    }

    if (ObjectXML::validate_xml(sql_xml) != 0)
    {
        NebulaLog::log("HPL", Log::WARNING, "Could not transform Host monitoring to XML" + string(sql_xml));
        db->free_str(sql_xml);
        return -1;
    }

    ostringstream oss;
    oss << "REPLACE INTO " << monit_table << " ("<< monit_db_names <<") VALUES ("
        <<          h->oid()             << ","
        <<          monitoring.timestamp() << ","
        << "'" <<   sql_xml                 << "')";

    db->free_str(sql_xml);

    auto rc = db->exec_local_wr(oss);

    return rc;
}
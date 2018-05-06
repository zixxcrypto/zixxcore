#include "netbase.h"
#include "triggerorderconfig.h"
#include "util.h"
#include "chainparams.h"

#include <iomanip>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

TriggerOrderConfig triggerOrderConfig;

void TriggerOrderConfig::add(std::string alias, std::string from_address, double percentage, std::string to_address)
{
    TriggerOrderEntry cme(alias, from_address, percentage, to_address);
    _entries.push_back(cme);

    setDirty(true);
}

bool TriggerOrderConfig::remove(std::string alias)
{
    for (std::vector<TriggerOrderEntry>::iterator it = _entries.begin() ; it != _entries.end(); ++it) {
        if (it->getAlias() == alias) {
            _entries.erase(it);
            setDirty(true);
            return true;
        }
    }

    return false;
}

TriggerOrderConfig::TriggerOrderEntry TriggerOrderConfig::get(std::string alias) const
{
    BOOST_FOREACH(const TriggerOrderConfig::TriggerOrderEntry entry, _entries) {
        if (entry.getAlias() == alias) {
            return entry;
        }
    }

    return TriggerOrderEntry();
}

bool TriggerOrderConfig::write()
{
    // Write the new cached files to the disk
    boost::filesystem::path pathTriggerOrderEntry = GetTriggerOrderConfigFile();
    boost::filesystem::ofstream outstream(pathTriggerOrderEntry);

    BOOST_FOREACH(TriggerOrderConfig::TriggerOrderEntry entry, _entries){
        if (!(outstream
              << std::setprecision(4)
              << entry.getAlias().c_str()
              << " "
              << entry.getFromAddress().c_str()
              << " "
              << entry.getPercentage()
              << " "
              << entry.getToAddress().c_str()
              << "\n")) {
            outstream.close();
            return false;
        }
    }

    outstream.close();
    setDirty(false);
    return true;
}

void TriggerOrderConfig::clear()
{
    _entries.clear();
}

bool TriggerOrderConfig::read(std::string& strErr) {
    int linenumber = 1;
    boost::filesystem::path pathTriggerOrderEntry = GetTriggerOrderConfigFile();
    boost::filesystem::ifstream streamConfig(pathTriggerOrderEntry);

    if (!streamConfig.good()) {
        // Do nothing if the file is not there.
        return true;
    }

    for(std::string line; std::getline(streamConfig, line); linenumber++)
    {
        if(line.empty()) continue;

        std::istringstream iss(line);
        std::string comment, alias, from_address, to_address;
        double      percentage = 0;

        if (iss >> comment) {
            if(comment.at(0) == '#') continue;
            iss.str(line);
            iss.clear();
        }

        if (!(iss >> alias >> from_address >> percentage >> to_address)) {
            iss.str(line);
            iss.clear();
            strErr = _("Could not parse triggerorder.conf") + "\n" + strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"";
            streamConfig.close();
            return false;
        }

        add(alias, from_address, percentage, to_address);
    }

    streamConfig.close();
    return true;
}

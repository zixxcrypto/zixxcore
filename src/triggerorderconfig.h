#ifndef TRIGGERORDERCONFIG_H
#define TRIGGERORDERCONFIG_H

// Copyright (c) 2018-2018 The ZIXX Team developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

class TriggerOrderConfig;
extern TriggerOrderConfig triggerOrderConfig;

class TriggerOrderConfig
{

public:

    class TriggerOrderEntry {

    private:
        std::string _alias;
        std::string _from_address;
        double      _percentage;
        std::string _to_address;
    public:
        TriggerOrderEntry() {}
        TriggerOrderEntry(std::string alias, std::string from_address, double percentage, std::string to_address)
            : _alias(alias)
            , _from_address(from_address)
            , _percentage(percentage)
            , _to_address(to_address)
        {
        }

        const std::string& getAlias() const {
            return _alias;
        }

        void setAlias(const std::string& alias) {
            _alias = alias;
        }

        const std::string& getFromAddress() const {
            return _from_address;
        }

        void setFromAddress(const std::string& from_address) {
            _from_address = from_address;
        }

        double getPercentage() const {
            return _percentage;
        }

        void setPercentage(double percentage) {
            _percentage = percentage;
        }

        const std::string& getToAddress() const {
            return _to_address;
        }

        void setToAddress(const std::string& to_address) {
            _to_address = to_address;
        }

    };

    TriggerOrderConfig()
        : _dirty(false)
    {
        _entries = std::vector<TriggerOrderEntry>();
    }

    void clear();
    bool read(std::string& strErr);
    bool write();
    void add(std::string alias, std::string from_address, double percentage, std::string to_address);
    bool remove(std::string alias);
    TriggerOrderEntry get(std::string alias) const;


    bool isDirty() const {
        return _dirty;
    }

    std::vector<TriggerOrderEntry>& getEntries() {
        return _entries;
    }

    int getCount() {
        return (int)_entries.size();
    }

    void setDirty(bool not_clean) {
        _dirty = not_clean;
    }

private:
    std::vector<TriggerOrderEntry> _entries;

    // To indicate whether it has been flushed.
    bool                           _dirty;

};

#endif // TRIGGERORDERCONFIG_H

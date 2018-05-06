#pragma once
// Copyright (c) 2018-2018 The ZIXX Team developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "consensus/consensus.h"
#include "validation.h"
#include "timedata.h"
#include "util.h"
#include "wallet/db.h"
#include "wallet/wallet.h"
#include "masternode-sync.h"


class CWallet;

/// The responsibilities of this class are to manage creation and dispatching of the
/// pre-defined scheduled transactions
class TriggerTransactionManager
{
public:
    class TriggerTransactionEntry {

    private:
        uint256         _input_hash;
        std::string     _alias;
        CBitcoinAddress _from_address;
        CAmount         _value;
        CBitcoinAddress _to_address;
        int             _target_block;
        bool            _committed;
        bool            _has_error;
    public:
        TriggerTransactionEntry() {}
        TriggerTransactionEntry(uint256 input_hash, std::string alias, CBitcoinAddress from_address, CAmount value, CBitcoinAddress to_address, int target_block, bool committed)
            : _input_hash(input_hash)
            , _alias(alias)
            , _from_address(from_address)
            , _value(value)
            , _to_address(to_address)
            , _target_block(target_block)
            , _committed(committed)
            , _has_error(false)
        {
        }

        uint256 getInputHash () const {
            return _input_hash;
        }

        std::string getAlias () const {
            return _alias;
        }

        CBitcoinAddress getFromAddress () const {
            return _from_address;
        }

        CAmount getValue () const {
            return _value;
        }

        CBitcoinAddress getToAddress () const {
            return _to_address;
        }

        int getTargetBlock () const {
            return _target_block;
        }

        bool getCommitted() const {
            return _committed;
        }

        bool hasError() const {
            return _has_error;
        }

        void setInputHash (uint256 input_hash) {
            _input_hash = input_hash;
        }

        void setAlias (std::string alias) {
            _alias = alias;
        }

        void setFromAddress (CBitcoinAddress from_address) {
            _from_address = from_address;
        }

        void setValue (CAmount value) {
            _value = value;
        }

        void setToAddress (CBitcoinAddress to_address) {
            _to_address = to_address;
        }

        void setTargetBlock (int target_block) {
            _target_block = target_block;
        }

        void setCommitted(bool committed) {
            _committed = committed;
        }

        void setError(bool error) {
            _has_error = error;
        }

        friend bool operator==(const TriggerTransactionEntry& a, const TriggerTransactionEntry& b);
    };

    TriggerTransactionManager();
    TriggerTransactionManager(CWallet *wallet);
    virtual ~TriggerTransactionManager() {    }

    void setup(CWallet *wallet);
    void clear();
    bool read(std::string& strErr);
    bool write();
    void add(uint256 input_hash, std::string alias, CBitcoinAddress from_address, CAmount value, CBitcoinAddress to_address, int target_block, bool committed);
    bool remove(std::string alias);

    bool process_cache(int current_block);
    bool send(TriggerTransactionEntry &entry);

    TriggerTransactionEntry get(std::string alias) const;
    std::vector<TriggerTransactionEntry> get_block(int target_block, bool committed) const;

    bool isDirty() const {
        return _dirty;
    }

    std::vector<TriggerTransactionEntry>& getEntries() {
        return _entries;
    }

    void setEntries(const std::vector<TriggerTransactionEntry> &entries);

    int getCount() {
        return (int)_entries.size();
    }

    void setDirty(bool not_clean) {
        _dirty = not_clean;
    }

    void setWallet(CWallet* wallet) {
        _wallet = wallet;
    }

    bool isReady() const {
        return masternodeSync.IsSynced();
    }

    // Protecting the data structures.
public:
    mutable CCriticalSection _cs;

private:
    void subscribe_to_core_signals();
    void unsubscribe_to_core_signals();

private:
    CWallet *_wallet;
    std::vector<TriggerTransactionEntry> _entries;

    int      _block_num;
    // To indicate whether it has been flushed.
    bool     _dirty;
};

class TriggerTransactionManager;
extern TriggerTransactionManager ttManager;

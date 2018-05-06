#include "triggertransactionmanager.h"
#include "wallet/wallet.h"
#include "triggerorderconfig.h"
#include "net.h"

TriggerTransactionManager ttManager;

static void BlockChecked(TriggerTransactionManager* ttm, const CBlock &block, const CValidationState &state);

TriggerTransactionManager::TriggerTransactionManager()
    : _wallet(nullptr)
    , _block_num(0)
    , _dirty(false)
{
}


TriggerTransactionManager::TriggerTransactionManager(CWallet *wallet)
    : _wallet(wallet)
    , _block_num(0)
    , _dirty(false)
{
}

void TriggerTransactionManager::setup(CWallet *wallet)
{
    _wallet = wallet;
    subscribe_to_core_signals();
}

void TriggerTransactionManager::clear()
{
    _entries.clear();
}

bool TriggerTransactionManager::read(std::string &strErr)
{
    return true;
}

bool TriggerTransactionManager::write()
{
    return false;
}

void TriggerTransactionManager::add(uint256 input_hash, std::string alias, CBitcoinAddress from_address, CAmount value, CBitcoinAddress to_address, int target_block, bool committed)
{
    TriggerTransactionEntry tte(input_hash, alias, from_address, value, to_address, target_block, committed);
    _entries.push_back(tte);

    setDirty(true);
}

bool TriggerTransactionManager::remove(std::string alias)
{
    for (std::vector<TriggerTransactionEntry>::iterator it = _entries.begin() ; it != _entries.end(); ++it) {
        if (it->getAlias() == alias) {
            _entries.erase(it);
            setDirty(true);
            return true;
        }
    }

    return false;
}

bool TriggerTransactionManager::process_cache(int current_block)
{
    {
        LOCK(_cs);
        for(TriggerTransactionManager::TriggerTransactionEntry &entry : _entries) {
            if (entry.getTargetBlock() < current_block && entry.getCommitted() == false) {
                bool error = false;
                if (!send(entry)) {
                    error = true;
                }

                entry.setCommitted(true);
                entry.setError(error);
            }
        }        
    }

    return true;
}

bool TriggerTransactionManager::send(TriggerTransactionManager::TriggerTransactionEntry &entry)
{
    LOCK2(cs_main, _wallet->cs_wallet);

    // Target address
    CBitcoinAddress address = entry.getToAddress();
    if (!address.IsValid()) {
        return false;
    }

    // Amount - determined previously based on the %
    CAmount amount = entry.getValue();
    if (amount <= 0) {
        return false;
    }

    // Wallet comments - to indicate that the transaction came from the alias.
    CWalletTx wtx;
    wtx.mapValue["comment"] = entry.getAlias();

    // Sending the actual cash.
    CAmount balance = _wallet->GetBalance();

    // Check amount
    if (amount <= 0 || amount > balance) {
        // Funds issue. Meh.
        return false;
    }

    // Parse Zixx address
    CScript scriptPubKey = GetScriptForDestination(address.Get());

    // Create and send the transaction
    CReserveKey reservekey(_wallet);
    CAmount fee;
    std::string error;
    std::vector<CRecipient> vec_send;
    int nChangePosRet = -1;
    CRecipient recipient = {scriptPubKey, amount, false};
    vec_send.push_back(recipient);

    if (!_wallet->CreateTransaction(vec_send, wtx, reservekey, fee, nChangePosRet, error, NULL, true, ALL_COINS, false)) {
        if (amount + fee > _wallet->GetBalance()) {
            // No funds!
        }

        return false;
    }

    if (!_wallet->CommitTransaction(wtx, reservekey, g_connman.get(), NetMsgType::TX)) {
        return false;
    }

    return true;
}

TriggerTransactionManager::TriggerTransactionEntry TriggerTransactionManager::get(std::string alias) const
{
    BOOST_FOREACH(const TriggerTransactionManager::TriggerTransactionEntry entry, _entries) {
        if (entry.getAlias() == alias) {
            return entry;
        }
    }

    return TriggerTransactionEntry();
}

std::vector<TriggerTransactionManager::TriggerTransactionEntry> TriggerTransactionManager::get_block(int target_block, bool committed = false) const
{
    std::vector<TriggerTransactionManager::TriggerTransactionEntry> count;
    BOOST_FOREACH(const TriggerTransactionManager::TriggerTransactionEntry& entry, _entries) {
        if (entry.getTargetBlock() < target_block && entry.getCommitted() == committed) {
            count.push_back(entry);
        }
    }

    return count;
}

void TriggerTransactionManager::setEntries(const std::vector<TriggerTransactionManager::TriggerTransactionEntry>& entries)
{
    for(auto potential_entry : entries) {
        auto it = std::find( _entries.begin(), _entries.end(), potential_entry );
        if (it == _entries.end() ) {
            _entries.push_back(potential_entry);
        }
    }
}

static void NotifyTransactionChanged(TriggerTransactionManager* ttm, CWallet *wallet, const uint256 &hash, ChangeType status)
{
    // Check to see if this transaction is present in the wallet.
    std::map<uint256, CWalletTx>::iterator mi = wallet->mapWallet.find(hash);
    bool inWallet = mi != wallet->mapWallet.end();

    // Can only process this if it's in the wallet.
    if (!inWallet) {
        return;
    }

    // Only process if the wallet is entirely synchronized.
    if (!ttm->isReady()) {
        return;
    }

    {
        LOCK(ttm->_cs);
        // Store the wallet tx and get the outputs to see if we have to do anything about the incoming transactions.
        CWalletTx wtx = mi->second;
        std::vector<TriggerOrderConfig::TriggerOrderEntry> possible_outputs = triggerOrderConfig.getEntries();
        std::vector<TriggerTransactionManager::TriggerTransactionEntry> entries;
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            BOOST_FOREACH(const TriggerOrderConfig::TriggerOrderEntry& entry, possible_outputs)
            {
                CScript entry_script = GetScriptForDestination(CBitcoinAddress(entry.getFromAddress()).Get());
                if (txout.scriptPubKey == entry_script)
                {
                    //TriggerTransactionEntry(uint256 input_hash, std::string alias, CBitcoinAddress from_address, CAmount value, CBitcoinAddress to_address, int target_block, bool committed)
                    TriggerTransactionManager::TriggerTransactionEntry trigger(
                                hash,
                                entry.getAlias(),
                                CBitcoinAddress(entry.getFromAddress()),
                                txout.nValue * (entry.getPercentage()/100.),
                                CBitcoinAddress(entry.getToAddress()),
                                wtx.GetBlocksToMaturity() + chainActive.Height(),
                                false);

                    // Found an entry, push back if it's unique
                    entries.push_back(trigger);
                }
            }
        }

        // Adding the entries to the DB if there are any.
        if (entries.size() > 0)
        {
            ttm->setEntries(entries);
        }
    }
}

void TriggerTransactionManager::subscribe_to_core_signals()
{
    boost::signals2::connection conn = _wallet->NotifyTransactionChanged.connect(boost::bind(NotifyTransactionChanged, this, _1, _2, _3));
    GetMainSignals().BlockChecked.connect(boost::bind(BlockChecked, this, _1, _2));
}

void TriggerTransactionManager::unsubscribe_to_core_signals()
{
    _wallet->NotifyTransactionChanged.disconnect(boost::bind(NotifyTransactionChanged, this, _1, _2, _3));
    GetMainSignals().BlockChecked.disconnect(boost::bind(BlockChecked, this, _1, _2));
}

static void BlockChecked(TriggerTransactionManager* ttm, const CBlock &block, const CValidationState &state)
{
    // This behaves as a tick device, which indicates when this entity should behave..
    int current_block = chainActive.Height();
    std::vector<TriggerTransactionManager::TriggerTransactionEntry> entries = ttm->get_block(current_block, false);
    if (entries.size() > 0)
    {
        // There is data to be processed!
        ttm->process_cache(current_block);
    }
}

bool operator==(const TriggerTransactionManager::TriggerTransactionEntry &a, const TriggerTransactionManager::TriggerTransactionEntry &b)
{
    bool fResult = (a.getInputHash() == b.getInputHash()
                    && a.getValue() == b.getValue()
                    && a.getToAddress() == b.getToAddress()
                    && a.getValue() == b.getValue()
                    );
    return fResult;
}

// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2017 The DashCore developers
// Copyright (c) 2017-2018 ZIXX Team
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

// For the numeric max.
#include <limits>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The collapse of the western fiat monetary system may have begun. -GlobalResearch. 04.2016";
    const CScript genesisOutputScript = CScript() << ParseHex("04fcd5af4a65c7cfd24ef3785f5cbf4ca855736f462bdda425e03655e9f2385721c566d1adcad84daef367b8a255deac4cfbde206a546a272d800ced5d4ffea5bc") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */


class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 262800;             // one year
        consensus.nMasternodePaymentsStartBlock = 1500;         // one week: (BLK_PER_DAY * DAYS)

        // Those two are not quite used.
        consensus.nMasternodePaymentsIncreaseBlock  = 0;   // not used
        consensus.nMasternodePaymentsIncreasePeriod = 0;   // not used
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = std::numeric_limits<int>::max(); // actual historical value
        consensus.nBudgetPaymentsCycleBlocks = 16616; // ~(60*24*30)/2.6, actual number of blocks per month is 200700 / 12 = 16725
        consensus.nBudgetPaymentsWindowBlocks = 100;
        consensus.nBudgetProposalEstablishingTime = 60 * 60 * 24;
        consensus.nSuperblockStartBlock = std::numeric_limits<int>::max(); // The block at which 12.1 goes live (end of final 12.0 budget cycle)
        consensus.nSuperblockCycle      = std::numeric_limits<int>::max(); // ~(60*24*30)/2.6, actual number of blocks per month is 200700 / 12 = 16725
        consensus.nGovernanceMinQuorum  = 50;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height       = 150000;           // 6 months. => zixxteam to look at this
        consensus.BIP34Hash = uint256S("0x0000047d24635e347be3aaaeb66c26be94901a2f962feccd4f95090191f208c1");
        consensus.powLimit = uint256S("00000fffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 1 * 60 * 60; // Zixx: 1 hour
        consensus.nPowTargetSpacing = 2 * 60; // Zixx: 2 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        // KGW is off, move to DGW straightaway
        consensus.nPowKGWHeight = 1;
        consensus.nPowDGWHeight = 1;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1486252800; // Feb 5th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517788800; // Feb 5th, 2018

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 4070908800ULL;   // Yeap, a looooooooong time,
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 4070908800ULL;     // A long time from now.
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 3226; // 80% of 4032

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0"); // 0

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x000000000000001c172f518793c3b9e83f202284615592f87fe3506ce964dcd4"); // 782700

        // The development subsidy should start and end at the following blocks - 0 based.
        consensus.dev_subsidy_start_block = 15000;
        consensus.dev_subsidy_end_block = std::numeric_limits<uint32_t>::max();

        dev_subsidy.push_back(DevSubsidyEntry("zWYfGpcMMemnjyhCFJGszoqnVDzaZm1c8y", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zFC5GQzp6veFH2JFY6c1NYNpWS1yoXvZ4p", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zBCyYHKR4uSgajH3WbCHwXLi5Mpk5jRkeS", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zJNThSenZT3tZ41QurPLJSdpXPfFmffaP1", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zUMLFhS5LBpc5HJF5CHPffV5DB6i4pPgJK", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zJ45mWHCh1PMSLC5PkJnCLjNpQkNoG2Dbc", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zMBwsjbd63QY5qNSSPSG3cceMwMAnqGV7S", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zDHKLRf2JZDxvXEuHbM9qevFVZeSkSk2gW", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zMxmHG2mvo42UudZ5V7xqjQQcg2vowez8k", 100));
        dev_subsidy.push_back(DevSubsidyEntry("zET3frUzt2UDRN7LsPTk2isXR2kX4DpnHP", 100));

        // In %;
        consensus.dev_subsidy_percentage = 10;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xab;
        pchMessageStart[1] = 0x76;
        pchMessageStart[2] = 0x98;
        pchMessageStart[3] = 0xe1;
        vAlertPubKey = ParseHex("0476a1ada6f2c9b5ad0a61b1abfc58ed684cf67466d4e519f0a27161dd25f85560dccb309e39a6fdd2e91fb8f6e808b59f3c4044bff4df4d41b35d441c75938f4f");
        nDefaultPort = 44845;
        nMaxTipAge = 6 * 60 * 60;                               // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1525392000UL, 457084UL, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000c0d3f3582cb23f04e1458fc6c41caf609ee3729f02b881559860cecc1cc"));
        assert(genesis.hashMerkleRoot == uint256S("0x8649d6d3f502ae1c0a1d6b5a2a82a612d1247cb9904e852e4ca57b5025bdb397"));


        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("node01.zixx.org",  "node01.zixx.org"));
        vSeeds.push_back(CDNSSeedData("node02.zixx.org",  "node02.zixx.org"));
        vSeeds.push_back(CDNSSeedData("node03.zixx.org",  "node03.zixx.org"));


        // Zixx addresses start with 'z'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,142);
        // Zixx script addresses start with 'S'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,63);
        // Zixx private keys start with '8' or 'N' (?)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,198);
        // Zixx BIP32 pubkeys start with 'xpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        // Zixx BIP32 prvkeys start with 'xprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // Zixx BIP44 coin type is '5'
        nExtCoinType = 5;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60 * 60; // fulfilled requests expire in 1 hour
        strSporkPubKey = "03e46b8935f497d7656331b4a7effdc24b9a89f3119b792209ada14fbebfad7eaf";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (  0,       uint256S("0x000006e84d14c5c0d0025253a5198a0fc4b02a9208a46a2e89b768f7b6967f16")),
            1503127892, // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            5000        // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 262800; // Should be one year.

        // not true, but it's ok as long as it's less then nMasternodePaymentsIncreaseBlock
        consensus.nMasternodePaymentsStartBlock = 50;

        // Those two are not quite used.
        consensus.nMasternodePaymentsIncreaseBlock  = 0;   // not used
        consensus.nMasternodePaymentsIncreasePeriod = 0;   // not used
        consensus.nInstantSendKeepLock              = 6;
        consensus.nBudgetPaymentsStartBlock         = std::numeric_limits<int>::max();
        consensus.nBudgetPaymentsCycleBlocks        = 50;
        consensus.nBudgetPaymentsWindowBlocks       = 10;
        consensus.nBudgetProposalEstablishingTime   = 60 * 20;
        consensus.nSuperblockStartBlock             = std::numeric_limits<int>::max();     // NOTE: Should satisfy nSuperblockStartBlock > nBudgetPeymentsStartBlock
        consensus.nSuperblockCycle                  = std::numeric_limits<int>::max();     // Superblocks can be issued hourly on testnet
        consensus.nGovernanceMinQuorum              = 1;
        consensus.nGovernanceFilterElements         = 500;
        consensus.nMasternodeMinimumConfirmations   = 1;
        consensus.nMajorityEnforceBlockUpgrade      = 51;
        consensus.nMajorityRejectBlockOutdated      = 75;
        consensus.nMajorityWindow                   = 100;
        consensus.BIP34Height                       = 150000;           // 6 months.
        consensus.BIP34Hash                         = uint256S("0x0000047d24635e347be3aaaeb66c26be94901a2f962feccd4f95090191f208c1");
        consensus.powLimit                          = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan                = 1 * 60 * 60; // Zixx: 1 hour
        consensus.nPowTargetSpacing                 = 2 * 60; // Zixx: 2 minutes
        consensus.fPowAllowMinDifficultyBlocks      = true;
        consensus.fPowNoRetargeting                 = true;
        consensus.nPowKGWHeight                     = 1; // nPowKGWHeight >= nPowDGWHeight means "no KGW"
        consensus.nPowDGWHeight                     = 1;
        consensus.nRuleChangeActivationThreshold    = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow          = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1538092800; // September 28th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538092800; // September 28th, 2018

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime    = 4070908800ULL; // A long long time from now
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout      = 4070908800ULL; // A long time from now
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize   = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold    = 50; // 50% of 100

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00"); // 0

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000004f5aef732d572ff514af99a995702c92e4452c7af10858231668b1f"); // 37900

        // The development subsidy should start and end at the following blocks - 0 based.
        consensus.dev_subsidy_start_block = 5000;
        consensus.dev_subsidy_end_block = std::numeric_limits<uint32_t>::max();

        // In %;
        consensus.dev_subsidy_percentage = 10;

        dev_subsidy.push_back(DevSubsidyEntry("yc8skk65Vf7YSttjtCfxotw69VPki8kSwt", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yTpTYqeUKhMHw6mDi8HY49DTUqA67HqnwE", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yR3kDjct4Q4f4J5hwtCTaF9vkBe1tdretW", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yj7mdY7AfBmuagdKBZLdzqze6XnF48Fvt2", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yNHHVGDC7sFVixTFzqBMEa5vHrEJZvJfwm", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yaYXR1ihSXDaiarytuo8nZhFDMSR7MdgFB", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yLd9Whf2L9Q2A2WwdkLXC4xt5kqu26h2Gf", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yc2Tps3UfWzB6YBbH1m7FLh7BsYGC4VA6t", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yYJu3m9kJSCCCKKQBnMHvzzRv34JL5cvEW", 100));
        dev_subsidy.push_back(DevSubsidyEntry("yip7SaPP5xdkCvFNnrC8eSanZepe57FtAv", 100));

        pchMessageStart[0] = 0xab;
        pchMessageStart[1] = 0x76;
        pchMessageStart[2] = 0x98;
        pchMessageStart[3] = 0xe1;
        vAlertPubKey = ParseHex("04517d8a699cb43d3938d7b24faaff7cda448ca4ea267723ba614784de661949bf632d6304316b244646dea079735b9a6fc4af804efb4752075b9fe2245e14e412");
        nDefaultPort = 44846;
        nMaxTipAge = 0x7fffffff; // allow mining on top of old blocks for testnet
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1511622981UL, 197171UL, 0x1e7ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00005d272d44b92a0a8f6664224174ec06bed97fb402bc28788d3b3adc37a219"));
        assert(genesis.hashMerkleRoot == uint256S("0x8649d6d3f502ae1c0a1d6b5a2a82a612d1247cb9904e852e4ca57b5025bdb397"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("node01.zixx.org",  "node01.zixx.org"));
        vSeeds.push_back(CDNSSeedData("node02.zixx.org",  "node02.zixx.org"));
        vSeeds.push_back(CDNSSeedData("node03.zixx.org",  "node03.zixx.org"));

        // Testnet Zixx addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Testnet Zixx script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Testnet Zixx BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Testnet Zixx BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Testnet Zixx BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60 * 60; // fulfilled requests expire in 1 hour

        strSporkPubKey = "02be779ad31f5915d41eea8443262b25bc73acf8461273ef2eca4104cd2ff0c562";
        strMasternodePaymentsPubKey = "035130dd524b89392b56a89ff4324446f047a9b56f5fd32f207ed7178595c65c35";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (  0,       uint256S("0x00005d272d44b92a0a8f6664224174ec06bed97fb402bc28788d3b3adc37a219")),
               1503127892, // * UNIX timestamp of last checkpoint block
               0,          // * total number of transactions between genesis and last checkpoint
                           //   (the tx=... number in the SetBestChain debug.log lines)
               5000        // * estimated number of transactions per day after checkpoint
        };

    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";

        consensus.nSubsidyHalvingInterval = 150;

        consensus.nMasternodePaymentsStartBlock = 25;

        // Those two are not quite used.
        consensus.nMasternodePaymentsIncreaseBlock  = 0;   // not used
        consensus.nMasternodePaymentsIncreasePeriod = 0;   // not used
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 1000;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nBudgetProposalEstablishingTime = 60*20;
        consensus.nSuperblockStartBlock = 1500;
        consensus.nSuperblockCycle = 10;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 1 * 60 * 60; // Zixx: 1 hour
        consensus.nPowTargetSpacing = 2 * 60; // Zixx: 2 minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nPowKGWHeight = 15200; // same as mainnet
        consensus.nPowDGWHeight = 34140; // same as mainnet
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        // The development subsidy should start and end at the following blocks - 0 based.
        // First block is not necessary, of course.
        consensus.dev_subsidy_start_block = 2;
        consensus.dev_subsidy_end_block = std::numeric_limits<uint32_t>::max();

        // In %;
        consensus.dev_subsidy_percentage = 10;

        // Adding each dev account.
        dev_subsidy.push_back(DevSubsidyEntry("n7RfC2Yi2AJp5bxUU9FVMKa3zWJ6kTQdpx", 100));
        dev_subsidy.push_back(DevSubsidyEntry("nRFCwFYug1o5Ambm9dr7UCiYA5c8JSz3tK", 100));
        dev_subsidy.push_back(DevSubsidyEntry("nAYsHM8ZysLuUL7fMFNK8iwBubxYsVpRzd", 100));
        dev_subsidy.push_back(DevSubsidyEntry("nSgYjnbEemG7vpxc6Y7uGZWcajbHS95XxW", 100));

        pchMessageStart[0] = 0xab;
        pchMessageStart[1] = 0x76;
        pchMessageStart[2] = 0x98;
        pchMessageStart[3] = 0xe1;
        nMaxTipAge = 6 * 60 * 60;   // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 0;   // never delay GETHEADERS in regtests
        nDefaultPort = 44847;
        nPruneAfterHeight = 10000;

        genesis = CreateGenesisBlock(1511622981UL, 197171UL, 0x1e7ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00005d272d44b92a0a8f6664224174ec06bed97fb402bc28788d3b3adc37a219"));
        assert(genesis.hashMerkleRoot == uint256S("0x8649d6d3f502ae1c0a1d6b5a2a82a612d1247cb9904e852e4ca57b5025bdb397"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers           = false;
        fDefaultConsistencyChecks      = true;
        fRequireStandard               = false;
        fMineBlocksOnDemand            = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        nFulfilledRequestExpireTime = 5 * 60; // fulfilled requests expire in 5 minutes
        strSporkPubKey = "02be779ad31f5915d41eea8443262b25bc73acf8461273ef2eca4104cd2ff0c562";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (  0,       uint256S("0x00005d272d44b92a0a8f6664224174ec06bed97fb402bc28788d3b3adc37a219")),
            0,          // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0           // * estimated number of transactions per day after checkpoint
        };

        // Regtest Zixx addresses start with 'n'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,112);
        // Regtest Zixx script addresses start with '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,20);
        // Regtest private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,240);
        // Regtest Zixx BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Regtest Zixx BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Regtest Zixx BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;
   }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

const DevSubsidyEntry &CChainParams::GetDevSubsidyAtHeight(int height) const
{
    // Quit and don't do anything if there's no subsidy.
    if (height <= 0 || dev_subsidy.size() == 0)
    {
        throw std::runtime_error(strprintf("Invalid height (%d) or dev. subsidy entry. Giving up!", height));
    }

    // But for now we're grand with a rotation scheme.
    int entry = height % dev_subsidy.size();
    return dev_subsidy.at(entry);
}

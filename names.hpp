#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

#include "./include/eosio/native.hpp"

using namespace eosio;
using namespace std;

class [[eosio::contract("names")]] names : public contract {
public:
    using contract::contract;

    /**
     * ## TABLE `prices`
     *
     * - `{extended_asset} base` - base price for 12 character names
     * - `{map<uint8_t, asset} premium` - prices for premium names
     * - `{uint64_t} incremental` - incremental price increase (bips 1/100 of 1%)
     * - `{uint64_t} [min_multiplier=2500]` - minimum price multiplier (bips 1/100 of 1%)
     * - `{uint64_t} [max_multiplier=40000]` - maximum price multiplier (bips 1/100 of 1%)
     *
     * ### example
     *
     * ```json
     * {
     *     "base": {"quantity": "1.0000 EOS", "contract": "eosio.token"},
     *     "premium": [
     *         {"key": 12, "value": "3.0000 EOS"},
     *         {"key": 11, "value": "5.0000 EOS"},
     *         ...
     *     ],
     *     "incremental": 13500,
     *     "min_multiplier": 2500,
     *     "max_multiplier": 40000
     * }
     * ```
     */
    struct [[eosio::table("prices")]] prices_row {
        extended_asset          base;
        map<uint8_t, asset>     premium;
        uint64_t                incremental;
        uint64_t                min_multiplier = 2500;
        uint64_t                max_multiplier = 40000;
    };
    typedef eosio::singleton< "prices"_n, prices_row> prices;

    /**
     * ## TABLE `suffixes`
     *
     * - `{name} suffix` - premium suffix
     * - `{uint16_t} [price_multiplier=10000]` - price multiplier (bips 1/100 of 1%)
     * - `{name} comission_account` - account recipient of comission sales
     * - `{asset} comissions` - total amount of comissions
     * - `{permission_level} permission` - permission level authorized to execute `newaccount` action
     * - `{uint64_t} transactions` - total amount of buyaccount transactions
     * - `{time_point_sec} created_at` - timestamp when suffix was created
     * - `{time_point_sec} updated_at` - timestamp when suffix was updated
     * - `{time_point_sec} buyaccount_at` - timestamp for last buyaccount event
     *
     * ### example
     *
     * ```json
     * {
     *     "suffix": "xy",
     *     "price_multiplier": 10000,
     *     "commission_account": "sale.xy",
     *     "commissions": "20.0000 EOS",
     *     "permission": { "actor": "xy", "permission": "active" },
     *     "transactions": 5,
     *     "created_at": "2020-11-15T15:33:10",
     *     "udpated_at": "2020-11-15T15:33:10",
     *     "buyaccount_at": "2020-11-15T15:33:10"
     * }
     * ```
     */
    struct [[eosio::table("suffixes")]] suffixes_row {
        name                suffix;
        uint16_t            price_multiplier = 10000;
        name                commission_account;
        asset               commissions;
        uint64_t            transactions = 0;
        permission_level    permission;
        time_point_sec      created_at;
        time_point_sec      updated_at;
        time_point_sec      buyaccount_at;

        uint64_t primary_key() const { return suffix.value; }
    };
    typedef eosio::multi_index< "suffixes"_n, suffixes_row> suffixes;

    /**
     * ## TABLE `settings`
     *
     * - `{asset} cpu` - CPU resources
     * - `{asset} net` - NET resources
     * - `{asset} ram` - RAM resources
     * - `{bool} rex` - [true/false] rent resources using REX or `delegatebw`
     *
     * ### example
     *
     * ```json
     * {
     *   "cpu": "0.0950 EOS",
     *   "net": "0.0050 EOS",
     *   "ram": "0.1500 EOS",
     *   "rex": true
     * }
     * ```
     */
    struct [[eosio::table("settings")]] settings_row {
        asset           cpu = asset{950, symbol{"EOS", 4}};
        asset           net = asset{50, symbol{"EOS", 4}};
        asset           ram = asset{1500, symbol{"EOS", 4}};
        bool            rex = true;
    };
    typedef eosio::singleton< "settings"_n, settings_row> settings;

    /**
     * ## TABLE `accounts`
     *
     * Track account balances for deposits & withdraws
     *
     * **scope:** `owner`
     *
     * - `{asset} balance` - quantity balance
     * - `{name} contract` - quantity token contract
     *
     * ### Example - cleos
     *
     * ```bash
     * $ cleos get currency balance names myaccount EOS
     * "1.0000 EOS"
     * ```
     *
     * ### Example - json
     *
     * ```json
     * {
     *     "balance": "1.0000 EOS",
     *     "contract": "eosio.token"
     * }
     * ```
     */
    struct [[eosio::table("accounts")]] account {
        asset       balance;
        name        contract;

        uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };
    typedef eosio::multi_index< "accounts"_n, account > accounts;

    /**
     * ## ACTION `buyaccount`
     *
     * Buy account
     *
     * - **authority**: `creator`
     *
     * ### params
     *
     * - `{name} creator` - creator/payer for name
     * - `{name} name` - name to buy
     * - `{authority} owner` - owner account permission
     * - `{authority} [active=null]` - (optional) active account permission (if null defaults to `owner` permission)
     *
     * ### Example
     *
     * ```bash
     * cleos push action names buyaccount '["myaccount", "foo.xy",
     *     {"accounts": [],"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"threshold": 1,"waits": []},
     *     {"accounts": [],"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"threshold": 1,"waits": []}
     * ]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void buyaccount( const name creator, const name name, const eosiosystem::authority owner, const optional<eosiosystem::authority> active );

    /**
     * ## ACTION `regsuffix`
     *
     * Register suffix name
     *
     * - **authority**: `suffix`
     *
     * ### params
     *
     * - `{name} suffix` - suffix owner account
     * - `{name} [commission_account=null]` - (optional) account to receive sale commissions (default will be `<suffix>`)
     * - `{permission_level} [permission=null]` - (optional) permission level authorized to execute `newaccount` action (default will be `<suffix>@active`)
     *
     * ### Example
     *
     * ```bash
     * cleos push action names regsuffix '["xy", "sale.xy", {"actor": "xy", "permission": "active"}]' -p xy
     * ```
     */
    [[eosio::action]]
    void regsuffix( const name suffix, const optional<name> commission_account, const optional<permission_level> permission );

    /**
     * ## ACTION `discount`
     *
     * Discount allow to reduce/increase the `premium` price curve per suffix, price will not fall below `base` price.
     *
     * - **authority**: `suffix`
     *
     * ### params
     *
     * - `{name} suffix` - suffix owner account
     * - `{uint16_t} [price_multiplier=10000]` - account to receive sale commissions (10000 => 100%)
     *
     * ### Example
     *
     * ```bash
     * cleos push action names discount '["xy", 10000]' -p xy
     * ```
     */
    [[eosio::action]]
    void discount( const name suffix, const uint16_t price_multiplier = 10000 );

    /**
     * ## ACTION `withdraw`
     *
     * Request to withdraw quantity
     *
     * - **authority**: `owner` or `get_self()`
     *
     * ### params
     *
     * - `{name} owner` - owner account to withdraw
     * - `{extended_asset} amount` - withdraw quantity amount (ex: "1.0000 EOS@eosio.token")
     * - `{string} [memo=null]` - (optional) memo when sending transfer (default = "withdraw")
     *
     * ### Example - cleos
     *
     * ```bash
     * cleos push action names withdraw '["myaccount", ["1.0000 EOS", "eosio.token", null]]' -p myaccount
     * ```
     *
     * ### Example - smart contract
     *
     * ```c++
     * // input variables
     * const name owner = "myaccount"_n;
     * const extended_asset amount = {{ 10000, {"EOS", 4}}, "eosio.token"};
     *
     * // send transaction
     * names::withdraw_action withdraw( "names"_n, { owner, "active"_n });
     * withdraw.send( owner, amount );
     * ```
     */
    [[eosio::action]]
    void withdraw( const name owner, const extended_asset amount, const optional<string> memo );

    /**
     * ## ACTION `delsuffix`
     *
     * Delete suffix
     *
     * - **authority**: `suffix` or `get_self()`
     *
     * ### params
     *
     * - `{name} suffix` - suffix to be deleted
     *
     * ### Example - cleos
     *
     * ```bash
     * cleos push action names delsuffix '["xy"]' -p xy
     * ```
     */
    [[eosio::action]]
    void delsuffix( const name suffix );

    [[eosio::action]]
    void deposit( const name owner, const asset amount );

    [[eosio::action]]
    void log( const eosio::name creator, const eosio::name name, const eosio::name suffix, const asset price, const asset commission, const asset fee, const string website );

    [[eosio::action]]
    void setsettings( const optional<names::settings_row> settings );

    [[eosio::action]]
    void setprices( const extended_asset base, const uint64_t incremental, const optional<uint64_t> min_multiplier, const optional<uint64_t> max_multiplier );

    /**
     * Notify contract when any token transfer notifiers relay contract
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );

    /**
     * ## STATIC `get_balance`
     *
     * Get balance of account
     *
     * ### params
     *
     * - `{name} code` - names contract account
     * - `{name} account` - account name
     * - `{symbol_code} symcode` - symbol code
     *
     * ### example
     *
     * ```c++
     * const asset balance = names::get_balance( "names"_n, "myaccount", {"EOS"} );
     * //=> "1.0000 EOS"
     * ```
     */
    static asset get_balance( const name code, const name owner, const symbol_code symcode )
    {
        names::accounts _accounts( code, owner.value );
        return _accounts.get( symcode.raw(), "no balance with specified symbol" ).balance;
    }

    /**
     * ## STATIC `get_price`
     *
     * Get price for account name
     *
     * ### params
     *
     * - `{name} code` - `names` contract account
     * - `{name} name` - price for account name
     *
     * ### example
     *
     * ```c++
     * const asset price = names::get_price( "names"_n, "foo.xy" );
     * //=> "6.0000 EOS";
     * ```
     */
    static asset get_price( const name code, const eosio::name name )
    {
        // tables
        names::prices _prices( code, code.value );
        names::settings _settings( code, code.value );
        names::suffixes _suffixes( code, code.value );

        // settings
        check( _prices.exists() || _settings.exists(), "contract is under going maintenance");
        auto prices = _prices.get();

        // basic name
        const eosio::name suffix = name.suffix();
        if ( name == suffix ) return prices.base.quantity;

        // premium name
        const uint64_t price_multiplier = _suffixes.get( suffix.value, "suffix is not available" ).price_multiplier;
        const asset premium = prices.premium.at(name.length()) * price_multiplier / 10000;

        // premium name cannot be lower than `base` price (due to lower price multiplier)
        if ( premium < prices.base.quantity ) return prices.base.quantity;
        return premium;
    }

    // action wrappers
    using withdraw_action = eosio::action_wrapper<"withdraw"_n, &names::withdraw>;
    using deposit_action = eosio::action_wrapper<"deposit"_n, &names::deposit>;
    using regsuffix_action = eosio::action_wrapper<"regsuffix"_n, &names::regsuffix>;
    using discount_action = eosio::action_wrapper<"discount"_n, &names::discount>;
    using log_action = eosio::action_wrapper<"log"_n, &names::log>;
    using setsettings_action = eosio::action_wrapper<"setsettings"_n, &names::setsettings>;

private:
    void add_balance( const name owner, const extended_asset value, const name ram_payer );
    void sub_balance( const name owner, const extended_asset value );
    asset buy_resources( const name account );
};
# `names`

[EOS Name Service](https://eosnameservice.io) is the most comprehensive platform to register premium/base EOS/WAX account names.

## Chains

- [EOS Mainnet](https://bloks.io/account/names)
- [WAX Mainnet](https://wax.bloks.io/account/names)

## Quickstart - Buy Name

```bash
# deposit amount to `names`
cleos transfer myaccount names "1.0000 EOS"

# buy account using EOSIO permissions
cleos push action names buyaccount '["<creator>", "<name>", {owner}, {active}]' -p myaccount

# example - @active public key
cleos push action names buyaccount '["myaccount", "foo.xy", {
    "accounts": [],
    "keys": [{"key": "EOS6MRyAj...GDW5CV","weight": 1}],
    "threshold": 1,
    "waits": []
}, null]' -p myaccount
```

## Quickstart - Register Suffix

```bash
# add `@eosio.code` to your suffix's active permission
cleos set account permission <suffix> active names --add-code

# register suffix
cleos push action names regsuffix '["<suffix>", "<commission>", {permission}]' -p <suffix>

# example - simple
cleos push action names regsuffix '["xy", null, null]' -p xy

# example - comission & permission level
cleos push action names regsuffix '["xy", "sale.xy", {"actor": "xy", "permission": "names"}]' -p xy
```

## Quickstart - Discount Suffix

```bash
# discount allow to reduce/increase the `premium` price curve per suffix, price will not fall below `base` price.
cleos push action names discount '["<suffix>", <price multiplier>]' -p <suffix>

# example - offer 50% discount
cleos push action names discount '["xy", 5000]' -p xy
```

## Table of Content

- [TABLE `prices`](#table-prices)
- [TABLE `suffixes`](#table-suffixes)
- [TABLE `settings`](#table-settings)
- [ACTION `buyaccount`](#action-buyaccount)
- [ACTION `regsuffix`](#action-regsuffix)
- [ACTION `discount`](#action-discount)
- [ACTION `withdraw`](#action-withdraw)
- [ACTION `delsuffix`](#action-delsuffix)
- [STATIC `get_balance`](#static-get_balance)
- [STATIC `get_price`](#static-get_price)

## TABLE `prices`

- `{extended_asset} base` - base price for 12 character names
- `{map<uint8_t, asset} premium` - prices for premium names
- `{uint64_t} incremental` - incremental price increase (bips 1/100 of 1%)
- `{uint64_t} [min_multiplier=2500]` - minimum price multiplier (bips 1/100 of 1%)
- `{uint64_t} [max_multiplier=40000]` - maximum price multiplier (bips 1/100 of 1%)

### example

```json
{
    "base": {"quantity": "1.0000 EOS", "contract": "eosio.token"},
    "premium": [
        {"key": 12, "value": "3.0000 EOS"},
        {"key": 11, "value": "5.0000 EOS"},
        ...
    ],
    "incremental": 13500,
    "min_multiplier": 2500,
    "max_multiplier": 40000
}
```

## TABLE `suffixes`

- `{name} suffix` - premium suffix
- `{uint16_t} [price_multiplier=10000]` - price multiplier (bips 1/100 of 1%)
- `{name} comission_account` - account recipient of comission sales
- `{asset} comissions` - total amount of comissions
- `{permission_level} permission` - permission level authorized to execute `newaccount` action
- `{uint64_t} transactions` - total amount of buyaccount transactions
- `{time_point_sec} created_at` - timestamp when suffix was created
- `{time_point_sec} updated_at` - timestamp when suffix was updated
- `{time_point_sec} buyaccount_at` - timestamp for last buyaccount event

### example

```json
{
    "suffix": "xy",
    "price_multiplier": 10000,
    "commission_account": "sale.xy",
    "commissions": "20.0000 EOS",
    "permission": { "actor": "xy", "permission": "active" },
    "transactions": 5,
    "created_at": "2020-11-15T15:33:10",
    "udpated_at": "2020-11-15T15:33:10",
    "buyaccount_at": "2020-11-15T15:33:10"
}
```

## TABLE `settings`

- `{asset} cpu` - CPU resources
- `{asset} net` - NET resources
- `{asset} ram` - RAM resources
- `{bool} rex` - [true/false] rent resources using REX or `delegatebw`

### example

```json
{
    "cpu": "0.0950 EOS",
    "net": "0.0050 EOS",
    "ram": "0.1500 EOS",
    "rex": true
}
```

## ACTION `buyaccount`

Buy account

- **authority**: `creator`

### params

- `{name} creator` - creator/payer for name
- `{name} name` - name to buy
- `{authority} owner` - owner account permission
- `{authority} [active=null]` - (optional) active account permission (if null defaults to `owner` permission)

### Example

```bash
cleos push action names buyaccount '["myaccount", "foo.xy",
    {"accounts": [],"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"threshold": 1,"waits": []},
    {"accounts": [],"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"threshold": 1,"waits": []}
]' -p myaccount
```

## ACTION `regsuffix`

Register suffix name

- **authority**: `suffix`

### params

- `{name} suffix` - suffix owner account
- `{name} [commission_account=null]` - (optional) account to receive sale commissions (default will be `<suffix>`)
- `{permission_level} [permission=null]` - (optional) permission level authorized to execute `newaccount` action (default will be `<suffix>@active`)

### Example

```bash
cleos push action names regsuffix '["xy", "sale.xy", {"actor": "xy", "permission": "active"}]' -p xy
```

## ACTION `discount`

Discount allow to reduce/increase the `premium` price curve per suffix, price will not fall below `base` price.

- **authority**: `suffix`

### params

- `{name} suffix` - suffix owner account
- `{uint16_t} [price_multiplier=10000]` - account to receive sale commissions (10000 => 100%)

### Example

```bash
cleos push action names discount '["xy", 10000]' -p xy
```

## ACTION `withdraw`

Request to withdraw quantity

- **authority**: `owner` or `get_self()`

### params

- `{name} owner` - owner account to withdraw
- `{extended_asset} amount` - withdraw quantity amount (ex: "1.0000 EOS@eosio.token")
- `{string} [memo=null]` - (optional) memo when sending transfer (default = "withdraw")

### Example - cleos

```bash
cleos push action names withdraw '["myaccount", ["1.0000 EOS", "eosio.token"], null]' -p myaccount
```

### Example - smart contract

```c++
// input variables
const name owner = "myaccount"_n;
const extended_asset amount = {{ 10000, {"EOS", 4}}, "eosio.token"};

// send transaction
names::withdraw_action withdraw( "names"_n, { owner, "active"_n });
withdraw.send( owner, amount );
```

## ACTION `delsuffix`

Delete suffix

- **authority**: `suffix` or `get_self()`

### params

- `{name} suffix` - suffix to be deleted

### Example - cleos

```bash
cleos push action names delsuffix '["xy"]' -p xy
```

## STATIC `get_balance`

Get balance of account

### params

- `{name} code` - names contract account
- `{name} account` - account name
- `{symbol_code} symcode` - symbol code

### example

```c++
const asset balance = names::get_balance( "names"_n, "myaccount", {"EOS"} );
//=> "1.0000 EOS"
```

## STATIC `get_price`

Get price for account name

### params

- `{name} code` - `names` contract account
- `{name} name` - price for account name

### example

```c++
const asset price = names::get_price( "names"_n, "foo.xy" );
//=> "6.0000 EOS"
```
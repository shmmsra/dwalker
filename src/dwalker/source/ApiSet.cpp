#include <algorithm>
#include <utility>

#include <ApiSet.hpp>

using namespace std;

class V2V4ApiSetSchema final : public ApiSetSchemaBase
{
public:
    vector<pair<wstring, ApiSetTarget>> All;

    vector<pair<wstring, ApiSetTarget>> GetAll() override { return All; }
    ApiSetTarget Lookup(wstring name) override
    {
        auto lower_name = name;
        transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

        // TODO : check if ext- is not present on win7 and 8.1
        if (lower_name.rfind(L"api-", 0) != 0)
            return ApiSetTarget();

        // Force lowercase name
        name = move(lower_name);

        // remove "api-" or "ext-" prefix
        name = name.substr(4);

        // Note: The list is initially alphabetically sorted!!!
        auto min = 0;
        auto max = All.size() - 1;
        while (min <= max)
        {
            auto const cur = (min + max) / 2;
            auto pair = All[cur];

            if (lower_name.rfind(pair.first, 0) == 0)
                return pair.second;

            // TODO: The expected behaviour is to compare ordinal,
            // need to verify that it is working as expected
            if (name.compare(pair.first) < 0)
                max = cur - 1;
            else
                min = cur + 1;
        }
        return ApiSetTarget();
    }
};

unique_ptr<ApiSetSchemaBase> ApiSetSchemaImpl::ParseApiSetSchema(API_SET_NAMESPACE_UNION const* const apiSetMap)
{
    // Check the returned api namespace is correct
    if (!apiSetMap)
        return unique_ptr<ApiSetSchemaBase>(new EmptyApiSetSchema());

    switch (apiSetMap->Version)
    {
    case 2: // Win7
        return GetApiSetSchemaV2(&(apiSetMap->ApiSetNameSpaceV2));

    case 4: // Win8.1
        return GetApiSetSchemaV4(&(apiSetMap->ApiSetNameSpaceV4));

    case 6: // Win10
        return GetApiSetSchemaV6(&(apiSetMap->ApiSetNameSpaceV6));

    default: // unsupported
        return unique_ptr<ApiSetSchemaBase>(new EmptyApiSetSchema());
    }

    return unique_ptr<ApiSetSchemaBase>(new EmptyApiSetSchema());
}

unique_ptr<ApiSetSchemaBase> ApiSetSchemaImpl::GetApiSetSchemaV2(API_SET_NAMESPACE_V2 const* const map)
{
    auto const base = reinterpret_cast<ULONG_PTR>(map);
    auto schema = unique_ptr<V2V4ApiSetSchema>(new V2V4ApiSetSchema());
    for (auto it = map->Array, eit = it + map->Count; it < eit; ++it)
    {
        // Retrieve DLLs names implementing the contract
        ApiSetTarget targets;
        auto const value_entry = reinterpret_cast<PAPI_SET_VALUE_ENTRY_V2>(base + it->DataOffset);
        for (auto it2 = value_entry->Redirections, eit2 = it2 + value_entry->NumberOfRedirections; it2 < eit2; ++it2)
        {
            auto const value_buffer = reinterpret_cast<PWCHAR>(base + it2->ValueOffset);
            auto const value = wstring(value_buffer, 0, it2->ValueLength / sizeof(WCHAR));
            targets.push_back(value);
        }

        // Retrieve api min-win contract name
        auto const name_buffer = reinterpret_cast<PWCHAR>(base + it->NameOffset);
        auto name = wstring(name_buffer, 0, it->NameLength / sizeof(WCHAR));

        // force storing lowercase variant for comparison
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        wstring lower_name = move(name);

        schema->All.push_back(make_pair(lower_name, targets));
    }
    return schema;
}

unique_ptr<ApiSetSchemaBase> ApiSetSchemaImpl::GetApiSetSchemaV4(API_SET_NAMESPACE_V4 const* const map)
{
    auto const base = reinterpret_cast<ULONG_PTR>(map);
    auto schema = unique_ptr<V2V4ApiSetSchema>(new V2V4ApiSetSchema());
    for (auto it = map->Array, eit = it + map->Count; it < eit; ++it)
    {
        // Retrieve DLLs names implementing the contract
        auto targets = ApiSetTarget();
        auto const value_entry = reinterpret_cast<PAPI_SET_VALUE_ENTRY_V4>(base + it->DataOffset);
        for (auto it2 = value_entry->Redirections, eit2 = it2 + value_entry->NumberOfRedirections; it2 < eit2; ++it2)
        {
            auto const value_buffer = reinterpret_cast<PWCHAR>(base + it2->ValueOffset);
            auto const value = wstring(value_buffer, 0, it2->ValueLength / sizeof(WCHAR));
            targets.push_back(value);
        }

        // Retrieve api min-win contract name
        auto const name_buffer = reinterpret_cast<PWCHAR>(base + it->NameOffset);
        auto name = wstring(name_buffer, 0, it->NameLength / sizeof(WCHAR));

        // force storing lowercase variant for comparison
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        auto const lower_name = move(name);

        schema->All.push_back(make_pair(lower_name, targets));
    }
    return schema;
}

class V6ApiSetSchema sealed : public ApiSetSchemaBase
{
public:
    vector<pair<wstring, ApiSetTarget>> All = vector<pair<wstring, ApiSetTarget>>();
    vector<pair<wstring, ApiSetTarget>> HashedAll = vector<pair<wstring, ApiSetTarget>>();

    vector<pair<wstring, ApiSetTarget>> GetAll() override { return All; }
    ApiSetTarget Lookup(wstring name) override
    {
        // Force lowercase name
        transform(name.begin(), name.end(), name.begin(), ::tolower);

        // Note: The list is initially alphabetically sorted!!!
        auto min = 0;
        auto max = HashedAll.size() - 1;
        while (min <= max)
        {
            auto const cur = (min + max) / 2;
            auto pair = HashedAll[cur];

            if (name.rfind(pair.first, 0) == 0)
                return pair.second;

            // TODO: The expected behaviour is to compare ordinal,
            // need to verify that it is working as expected
            if (name.compare(pair.first) < 0)
                max = cur - 1;
            else
                min = cur + 1;
        }
        return ApiSetTarget();
    }
};

unique_ptr<ApiSetSchemaBase> ApiSetSchemaImpl::GetApiSetSchemaV6(API_SET_NAMESPACE_V6 const* const map)
{
    auto const base = reinterpret_cast<ULONG_PTR>(map);
    auto schema = unique_ptr<V6ApiSetSchema>(new V6ApiSetSchema());
    for (auto it = reinterpret_cast<PAPI_SET_NAMESPACE_ENTRY_V6>(map->EntryOffset + base), eit = it + map->Count; it < eit; ++it)
    {
        // Iterate over all the host dll for this contract
        auto targets = ApiSetTarget();
        for (auto it2 = static_cast<_API_SET_VALUE_ENTRY_V6* const>(reinterpret_cast<PAPI_SET_VALUE_ENTRY_V6>(base + it->ValueOffset)), eit2 = it2 + it->ValueCount; it2 < eit2; ++it2)
        {
            // Retrieve DLLs name implementing the contract
            auto const value_buffer = reinterpret_cast<PWCHAR>(base + it2->ValueOffset);
            auto const value = wstring(value_buffer, 0, it2->ValueLength / sizeof(WCHAR));
            targets.push_back(value);
        }

        // Retrieve api min-win contract name
        auto const name_buffer = reinterpret_cast<PWCHAR>(base + it->NameOffset);
        auto name = wstring(name_buffer, 0, it->NameLength / sizeof(WCHAR));
        auto hash_name = wstring(name_buffer, 0, it->HashedLength / sizeof(WCHAR));

        // force storing lowercase variant for comparison
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        auto const lower_name = move(name);
        transform(hash_name.begin(), hash_name.end(), hash_name.begin(), ::tolower);
        auto const lower_hash_name = move(name);

        schema->All.push_back(make_pair(lower_name, targets));
        schema->HashedAll.push_back(make_pair(lower_hash_name, targets));
    }
    return schema;
}

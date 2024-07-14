#pragma once

namespace RE {
    class StringSetting {
    private:
        uintptr_t _vtable = RE::VTABLE_Setting[0].address();  // 0x00

    public:
        char* str;         // 0x08
        const char* name;  // 0x10

        std::string GetString() const { return str; }
        const char* GetCString() const { return str; }
        std::string_view GetView() const { return str; }

        StringSetting(const char* a_name, const char* a_value) {
            name = a_name;
            str = _strdup(a_value);
        }
        const char* operator*() { return str; }
        operator std::string_view() const { return str; }
        operator const char*() const { return str; }
        operator RE::Setting*() { return reinterpret_cast<RE::Setting*>(this); }
    };
    static_assert(sizeof(StringSetting) == 0x18);
}
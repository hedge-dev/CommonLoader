#pragma once
#include <string>

// we roll our own embed over here
namespace clrhost
{
    class resource
    {
    public:
        std::string name;
        const void* ptr{};
        size_t size{};

        resource(std::string_view name, const void* ptr, size_t size);
        static const resource* find(std::string_view name);
    };
}

#define CLRHOST_CONCAT2(A, B) A##B
#define CLRHOST_CONCAT(A, B) CLRHOST_CONCAT2(A, B)
#ifdef CLRHOST_RESOURCE_IMPL
    #define CLRHOST_DECLARE_RESOURCE(DATA, SIZE, NAME) clrhost::resource CLRHOST_CONCAT(CLRHOST_RES_, DATA)((NAME), (const void*)(DATA), (size_t)(SIZE));
#else
    #define CLRHOST_DECLARE_RESOURCE(DATA) extern clrhost::resource CLRHOST_CONCAT(CLRHOST_RES_, DATA);
    #define CLRHOST_RESOURCE(DATA) CLRHOST_CONCAT(CLRHOST_RES_, DATA)
#endif
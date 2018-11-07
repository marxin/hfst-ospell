#ifndef HFST_OSPELL_OFFICE_H_
#define HFST_OSPELL_OFFICE_H_

#include <memory>
#include <vector>
#include <string>
#include <iosfwd>

#ifdef _WIN32
    #ifdef LIBHFSTOSPELLOFFICE_EXPORTS
        #define OSPELL_OFFICE_API __declspec(dllexport)
    #else
        #define OSPELL_OFFICE_API __declspec(dllimport)
    #endif
#else
    #ifdef LIBHFSTOSPELLOFFICE_EXPORTS
        #define OSPELL_OFFICE_API __attribute__ ((visibility ("default")))
    #else
        #define OSPELL_OFFICE_API
    #endif
#endif

namespace hfst_ospell {

class OfficeSpeller {
public:
    OSPELL_OFFICE_API OfficeSpeller(const char* zhfst, bool verbatim = false);
    OSPELL_OFFICE_API ~OfficeSpeller();

    OSPELL_OFFICE_API OfficeSpeller(OfficeSpeller &&) noexcept;
    OSPELL_OFFICE_API OfficeSpeller& operator=(OfficeSpeller &&) noexcept;

    OSPELL_OFFICE_API void ispell(std::istream& in, std::ostream& out);
    OSPELL_OFFICE_API bool check(const std::string& word);
    OSPELL_OFFICE_API const std::vector<std::string>& correct(const std::string& word, size_t suggs = 5);

private:
    class Impl;
    std::unique_ptr<Impl> im;
};

}

#endif

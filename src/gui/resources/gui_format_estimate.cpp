#include "nfs/gui/resources/gui_format.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>
namespace nfs::gui {
namespace {
int add(int left, int right) {
    return static_cast<int>(Dword(left) + Dword(right));
}
template <class Char> int decimal(const Char *text) {
    if constexpr (sizeof(Char) == 1)
        return std::atoi(text);
    else {
        char converted[20];
        WideCharToMultiByte(CP_ACP, 0, text, -1, converted, 20, nullptr, nullptr);
        return std::atoi(converted);
    }
}
// The CRT classifies these format digits in the C locale. Non-C legacy classification and
// overflowing/malformed format strings remain unaccepted.
template <class Char> int estimate(const Char *format, const Dword *arguments) {
    int total = 0;
    while (*format) {
        if (*format != '%') {
            total = add(total, 1);
            ++format;
            continue;
        }
        ++format;
        if (*format == '%') {
            total = add(total, 1);
            ++format;
            continue;
        }
        int width = 0, precision = 0, item = 0;
        while (*format) {
            const int flag = *format;
            if (flag == '#')
                total = add(total, 2);
            else if (flag == '*')
                width = static_cast<int>(*arguments++);
            else if (flag != '-' && flag != '+' && flag != '0' && flag != ' ')
                break;
            ++format;
        }
        if (!width) {
            width = decimal(format);
            while (*format >= '0' && *format <= '9')
                ++format;
        }
        if (*format == '.') {
            ++format;
            if (*format == '*') {
                precision = static_cast<int>(*arguments++);
                ++format;
            } else {
                precision = decimal(format);
                while (*format >= '0' && *format <= '9')
                    ++format;
            }
        }
        Dword modifier = 0;
        if (format[0] == 'I' && format[1] == '6' && format[2] == '4') {
            modifier = 262144;
            format += 3;
        } else if (*format == 'h') {
            modifier = 65536;
            ++format;
        } else if (*format == 'l') {
            modifier = 131072;
            ++format;
        } else if (*format == 'F' || *format == 'L' || *format == 'N')
            ++format;
        const Dword spec = Dword(static_cast<int>(*format)) | modifier;
        const bool character = spec == 'c' || spec == 'C' || spec == 65635 || spec == 65603 ||
                               spec == 131171 || spec == 131139;
        bool narrow_string = false, wide_string = false;
        if constexpr (sizeof(Char) == 1) {
            narrow_string = spec == 's' || spec == 'S' || spec == 65651 || spec == 65619;
            wide_string = spec == 131187 || spec == 131155;
        } else {
            narrow_string = spec == 65651 || spec == 65619;
            wide_string = spec == 's' || spec == 'S' || spec == 131187 || spec == 131155;
        }
        if (character || narrow_string || wide_string) {
            if (character)
                item = 2;
            else if (!*arguments)
                item = 6;
            else {
                item =
                    wide_string
                        ? static_cast<int>(
                              std::wcslen(reinterpret_cast<const wchar_t *>(*arguments)))
                        : static_cast<int>(std::strlen(reinterpret_cast<const char *>(*arguments)));
                if (item < 1)
                    item = 1;
            }
            ++arguments;
            if (precision && precision <= item)
                item = precision;
            total = add(total, width < item ? item : width);
        } else {
            switch (*format) {
            case 'G':
            case 'e':
            case 'g':
                arguments += 2;
                total = add(total, add(width, precision) < 128 ? 128 : add(width, precision));
                break;
            case 'X':
            case 'd':
            case 'i':
            case 'o':
            case 'u':
            case 'x':
                arguments += (modifier & 262144) ? 2 : 1;
                total = add(total, add(width, precision) < 32 ? 32 : add(width, precision));
                break;
            case 'p':
                ++arguments;
                total = add(total, add(width, precision) < 32 ? 32 : add(width, precision));
                break;
            case 'n':
                ++arguments;
                break;
            case 'f': {
                const int bytes = add(precision, 318) < width ? width : add(precision, 318);
                auto *temporary = static_cast<char *>(kernel::memory_allocate(nullptr, bytes, 0));
                if (temporary) {
                    double value;
                    std::memcpy(&value, arguments, 8);
                    arguments += 2;
                    std::sprintf(temporary, "%*.*f", width, add(precision, 6), value);
                    item = static_cast<int>(std::strlen(temporary));
                    kernel::memory_free(temporary);
                }
                total = add(total, item);
                break;
            }
            default:
                return 0;
            }
        }
        ++format;
    }
    return total;
}
} // namespace

int estimate_ascii(const char *format, const Dword *arguments) {
    return estimate(format, arguments);
}

int estimate_wide(const wchar_t *format, const Dword *arguments) {
    return estimate(format, arguments);
}
} // namespace nfs::gui

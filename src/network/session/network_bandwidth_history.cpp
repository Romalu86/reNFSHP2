#include "network_bandwidth.hpp"
#include "../../math/precision/extended_float.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::network::bandwidth {
Dword history[5]{};

Dword quantize_rate(Dword rate) {
    char text[100]{};
    std::sprintf(text, "%d", static_cast<int>(rate));
    for (unsigned index = 2; index < std::strlen(text); ++index)
        text[index] = '0';
    return static_cast<Dword>(std::atoi(text));
}

void save_history(Dword rate) {
    std::memset(history, 0, sizeof(history));
    char path[260]{};
    std::sprintf(path, "%s%s", graphics::startup::configuration_path(), "bandwidthhistory.txt");
    if (FILE *input = std::fopen(path, "r")) {
        char line[1024]{};
        while (std::fgets(line, 1024, input)) {
            Dword value{}, checksum{};
            std::sscanf(line, "%d %d", reinterpret_cast<int *>(&value),
                        reinterpret_cast<int *>(&checksum));
            if (value && kernel::checksum32(reinterpret_cast<const unsigned char *>(&value), 4) ==
                             checksum) {
                for (unsigned index = 0; index < 4; ++index)
                    history[index] = history[index + 1];
                history[4] = value;
            }
        }
        std::fclose(input);
    }
    for (unsigned index = 0; index < 4; ++index)
        history[index] = history[index + 1];
    history[4] = rate;
    if (FILE *output = std::fopen(path, "w")) {
        for (Dword &value : history)
            if (value)
                std::fprintf(output, "%d %d\n", static_cast<int>(value),
                             static_cast<int>(kernel::checksum32(
                                 reinterpret_cast<const unsigned char *>(&value), 4)));
        std::fclose(output);
    }
}

Dword read_history() {
    char path[260]{};
    std::sprintf(path, "%s%s", graphics::startup::configuration_path(), "bandwidthhistory.txt");
    Dword sum{};
    int count{};
    if (FILE *input = std::fopen(path, "r")) {
        char line[1024]{};
        while (std::fgets(line, 1024, input)) {
            Dword value{}, checksum{};
            std::sscanf(line, "%d %d", reinterpret_cast<int *>(&value),
                        reinterpret_cast<int *>(&checksum));
            if (value && kernel::checksum32(reinterpret_cast<const unsigned char *>(&value), 4) ==
                             checksum) {
                sum += value;
                count = static_cast<int>(static_cast<Dword>(count) + 1);
            }
        }
        std::fclose(input);
    }
    if (count <= 0)
        return 0;
    const auto format = kernel::numeric::current_format();
    return quantize_rate(Dword((kernel::numeric::Extended::integer(sum, format) /
                               kernel::numeric::Extended::integer(count, format))
                                  .truncate_to_integer()));
}
} // namespace nfs::network::bandwidth

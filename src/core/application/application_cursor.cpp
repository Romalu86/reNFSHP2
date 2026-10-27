#include "nfs/core/application/application.hpp"
#include "nfs/core/configuration/launch_options.hpp"
namespace nfs::application {

void show_cursor(Byte shown) {
    cursor_shown = shown;
    if (!shown) {
        if (!launch::field<Dword>(8627740)) {
            int count = ShowCursor(FALSE);
            if (count != -1) {
                while (count < -1)
                    count = ShowCursor(TRUE);
                if (count >= 0)
                    do {
                        count = ShowCursor(FALSE);
                    } while (count >= 0);
            }
        }
    } else if (!launch::field<Dword>(8627740)) {
        int count = ShowCursor(TRUE);
        if (count != 0) {
            while (count < 0)
                count = ShowCursor(TRUE);
            if (count > 0)
                do {
                    count = ShowCursor(FALSE);
                } while (count > 0);
        }
    }
}

// This entry does not write cursor_shown.
void hide_cursor() {
    if (!launch::field<Dword>(8627740)) {
        int count = ShowCursor(FALSE);
        if (count != -1) {
            while (count < -1)
                count = ShowCursor(TRUE);
            if (count >= 0)
                do {
                    count = ShowCursor(FALSE);
                } while (count >= 0);
        }
    }
}
} // namespace nfs::application

namespace nfs::application {
// Synchronizes Win32's counter with the saved desired cursor state.
void synchronize_cursor() {
    if (launch::field<Dword>(8627740))
        return;
    int count = ShowCursor(cursor_shown ? TRUE : FALSE);
    const int wanted = cursor_shown ? 0 : -1;
    while (count < wanted)
        count = ShowCursor(TRUE);
    while (count > wanted)
        count = ShowCursor(FALSE);
}
} // namespace nfs::application

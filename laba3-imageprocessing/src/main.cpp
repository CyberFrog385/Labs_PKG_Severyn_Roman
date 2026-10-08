#include <string>

#include <FL/Fl.H>

#include "core/cli.h"
#include "ui/main_window.h"

int main(int argc, char** argv) {
    if (argc >= 2 && std::string(argv[1]) == "cli") {
        return run_cli(argc, argv);
    }
    MainWindow window;
    if (argc >= 2) {
        window.open_path(argv[1]);
    }
    window.show();
    return Fl::run();
}

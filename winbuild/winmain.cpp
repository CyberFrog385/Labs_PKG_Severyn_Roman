#include <windows.h>

#include <shellapi.h>

#include <cstdlib>
#include <vector>

int main(int argc, char *argv[]);

static void winToUtf8(const wchar_t *src, std::vector<char> &out)
{
    int n = WideCharToMultiByte(CP_UTF8, 0, src, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 0)
        return;
    out.resize(n);
    WideCharToMultiByte(CP_UTF8, 0, src, -1, out.data(), n, nullptr, nullptr);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    int argc = 0;
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!wargv)
        return 1;

    std::vector<std::vector<char>> storage;
    std::vector<char *> argv;
    storage.reserve(argc);
    argv.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        storage.emplace_back();
        winToUtf8(wargv[i], storage.back());
        argv.push_back(storage.back().data());
    }
    LocalFree(wargv);

    return main(argc, argv.data());
}
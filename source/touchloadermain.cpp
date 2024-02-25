#include "comp.h"

#include <iostream>
#include <memory>
#include <filesystem>
#include <thread>
#include <windows.h>

bool exitCommand = false;
std::mutex mutex;

void enableRawMode()
{
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hIn, &mode);
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT); // Disable echo and line input
    SetConsoleMode(hIn, mode);
}

void disableRawMode()
{
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hIn, &mode);
    mode |= ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT;
    SetConsoleMode(hIn, mode);
}

void pollTerminal()
{
    bool running = true;
    enableRawMode();
    while (running)
    {
        char buffer[4]; // Increased buffer size to avoid buffer overflow 
        DWORD read;
        ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buffer, 1, &read, NULL);
        char ch = buffer[0]; // First character from the buffer

        if (ch == 'q')
        {
            std::lock_guard<std::mutex> lock(mutex);
            exitCommand = true;
            running = false;
            break;
        }
    }
    disableRawMode();
}

int main(int argc, char* argv[])
{
    // get path to which is ../../../test/test1.tox
    std::filesystem::path projectDir(CMAKE_PROJECT_DIR);
    std::filesystem::path compPath = projectDir / "test" / "TopChopDatIO.tox";
   
    std::cout << "Absolute file path: " << compPath.string() << std::endl;

    //std::unique_ptr<Comp> tox = std::make_unique<Comp>(compPath.string());

    std::unique_ptr<Comp> comp = std::make_unique<Comp>();
    comp->loadTox(compPath.string());

    // poll terminal in separate thread for exit command q
    std::thread terminal(pollTerminal);

    bool running = true;
    while (running)
    {
        comp->update();

        if (comp->ready())
            comp->pars()["Scale"].set(0.5);

        {
            std::lock_guard<std::mutex> lock(mutex);
            running = !exitCommand;
        }
    }

    if (terminal.joinable()) terminal.join();

    return 0;
}
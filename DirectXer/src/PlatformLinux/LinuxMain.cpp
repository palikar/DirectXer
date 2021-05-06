#include <Platform.hpp>
#include <Memory.hpp>
#include <Resources.hpp>
#include <App.hpp>


extern App* InitMain(char** argv, int argc);

struct WindowSettings
{
    char* ClassName;
    char* WindowTitle;
    size_t InitialWidth;
    size_t InitialHeight;
    bool Fullscreen;
};

class LinuxWindow
{
  public:

    App* Application;

    void Init(WindowSettings settings);
    int Run();

};

int main(int argc, char** argv)
{

    WindowSettings settings{0};
    settings.ClassName = "DirectXClass";
    settings.WindowTitle = "DirectXer";
	settings.InitialWidth = 1080;
	settings.InitialHeight = 720;
	settings.Fullscreen = 0;
        
    LinuxWindow window;
    window.Application = InitMain(argv, argc);

    window.Init(settings);
    return window.Run();
}

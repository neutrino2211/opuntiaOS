#include <libui/App.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    fork();
    new UI::App();
    auto* window_ptr = new UI::Window();
    window_ptr->run();
    window_ptr->set_superview(new UI::View(window_ptr->bounds()));
    return UI::App::the().run();
}
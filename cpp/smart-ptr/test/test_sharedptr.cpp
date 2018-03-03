#include <iostream>
#include <memory>

struct Foo
{
    Foo(int n = 0) noexcept : bar(n)
    {
        std::cout << "Foo: constructor, bar = " << bar << '\n';
    }
    ~Foo()
    {
        std::cout << "Foo: destructor, bar = " << bar << '\n';
    }
    int getBar() const noexcept
    {
        return bar;
    }

  private:
    int bar;
};

int main()
{
    std::shared_ptr<Foo> sptr = std::make_shared<Foo>(1);
    std::cout << "The first Foo's bar is " << sptr->getBar() << "\n";

    auto backup = sptr;

    // reset the shared_ptr, hand it a fresh instance of Foo
    // (the old instance will be destroyed after this call)
    sptr.reset(new Foo);
    std::cout << "The second Foo's bar is " << sptr->getBar() << "\n";
    std::cout << "The second Foo's bar is " << backup->getBar() << "\n";
}
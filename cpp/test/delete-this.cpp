class Test
{
  public:
    int  x = 10;
    void fun()
    {
        delete this;
        int y = this->x;
    }

  private:
};

int main(int argc, char** argv)
{
    auto x = new Test();

    x->fun();

    int num = x->x;

    return 0;
}
#include <limits>
#include <utility>

namespace moon {

template <class T> class allocator
{
  public:
    typedef T              value_type;
    typedef T *            pointer;
    typedef const T *      const_pointer;
    typedef T &            reference;
    typedef const T &      const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    /* constructor */
    allocator() {}
    allocator(const allocator &other) {}
    template <class U> allocator(const allocator<U> &other) {}

    /* destructor */
    ~allocator() {}

    /* ==,!= */
    template <class U> bool operator==(const allocator<U> &b)
    {
        return true;
    }
    template <class U> bool operator!=(const allocator<U> &b)
    {
        return false;
    }

    /* address() */
    pointer address(reference x)
    {
        return &x;
    }
    const_pointer address(const_reference x)
    {
        return &x;
    }

    /* allocate() */
    pointer allocate(size_type n)
    {
        return (T *)(::operator new(n));
    }

    /* deallocate() */
    void deallocate(T *p, std::size_t n)
    {
        return ::operator delete((void *)p);
    }

    /* max_size() */
    size_type max_size()
    {
        return std::numeric_limits<int>::max();
    }

    /* construct */
    template <class U, class... Args> void construct(U *p, Args... args)
    {
        ::new ((void *)p) U(std::forward<Args>(args)...);
    }

    /* destruct */
    template <class U> void destruct(U *p)
    {
        p->~U();
    }

  private:
};
}  // namespace moon
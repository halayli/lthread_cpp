#ifndef LTHREAD_CPP_LTHREAD_H
#define LTHREAD_CPP_LTHREAD_H

#include <iosfwd>
#include <type_traits>
#include <cstddef>
#include <functional>
#include <memory>
#include <system_error>
#include <tuple>
#include <exception>

#include <lthread.h>


// look away - I am hideous

namespace lthread_cpp {

void LthreadExecute(void* arg);
class Lthread
{
 public:
  class LthreadTimeout  {};

  typedef lthread_t*         native_handle_type;
  struct _Impl_base;
  typedef std::shared_ptr<_Impl_base>  __shared_base_type;

  // Simple base type that the templatized, derived class containing
  // an arbitrary functor can be converted to and called.
  struct _Impl_base
  {
    __shared_base_type    _M_this_ptr;

    inline virtual ~_Impl_base();
    virtual void _M_run() = 0;
  };

  template<typename _Callable>
  struct _Impl : public _Impl_base
  {
    _Callable       _M_func;

    _Impl(_Callable&& __f) : _M_func(std::forward<_Callable>(__f)) { }

    void _M_run() { _M_func(); }
  };

  class Id
  {
    native_handle_type lt_;

   public:
    Id() noexcept : lt_() { }

    explicit Id(native_handle_type __id) : lt_(__id) { }

   private:
    friend class Lthread;
    friend struct std::hash<Lthread::Id>;

    friend bool operator==(Lthread::Id __x, Lthread::Id __y) noexcept
    {
      return __x.lt_ == __y.lt_;
    }

    friend bool operator<(Lthread::Id __x, Lthread::Id __y) noexcept
    {
      return __x.lt_ < __y.lt_;
    }

  };


  Lthread() noexcept {}
  Lthread(Lthread&& lt) noexcept { Swap(lt); }

  Lthread& operator=(Lthread&& lt) noexcept
  {

    if (Joinable())
      std::terminate();

    Swap(lt);
    return *this;
  }

  Lthread::Id getId() const noexcept { return id_; }

  void Swap(Lthread& lt) noexcept {std::swap(id_, lt.id_);}

  template<typename _Callable, typename... _Args>
  explicit Lthread(_Callable&& __f, _Args&&... __args)
  {

    LthreadStart(LthreadMakeRoutine(std::bind(std::forward<_Callable>(__f),
                                              std::forward<_Args>(__args)...)));
  }

  template<typename _Callable>
  std::shared_ptr<_Impl<_Callable>> LthreadMakeRoutine(_Callable&& __f)
  {
    // Create and allocate full data structure, not base.
    return std::make_shared<_Impl<_Callable>>(std::forward<_Callable>(__f));
  }

  void LthreadStart(__shared_base_type f)
  {
    f->_M_this_ptr = f;
    int r = lthread_create(&id_.lt_, LthreadExecute, f.get());
    if (r != 0) {
      f.reset();
      throw std::system_error(r, std::system_category());
    }
  }

  void Join(uint64_t timeout_ms=0);
  void Detach();
  bool Joinable() const noexcept { return !(id_ == Id()); }

  Lthread(const Lthread&) = delete;
  Lthread& operator=(const Lthread&) = delete;

  static void Run() { lthread_run(); }

  ~Lthread()
  {
    if (Joinable())
      std::terminate();
  }

 private:
  Id id_;
};

inline void Swap(Lthread& __x, Lthread& __y) noexcept
{
  __x.Swap(__y);
}

inline Lthread::_Impl_base::~_Impl_base() = default;

inline bool operator!=(Lthread::Id __x, Lthread::Id __y) noexcept
{
  return !(__x == __y);
}

inline bool operator<=(Lthread::Id __x, Lthread::Id __y) noexcept
{
  return !(__y < __x);
}

inline bool operator>(Lthread::Id __x, Lthread::Id __y) noexcept
{
  return __y < __x;
}

inline bool operator>=(Lthread::Id __x, Lthread::Id __y) noexcept
{
  return !(__x < __y);
}
}

#endif

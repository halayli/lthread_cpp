#include "lthread_cpp/lthread.h"

using namespace lthread_cpp;
void Lthread::Detach()
{
  if (id_ != Id())
    lthread_detach2(id_.lt_);

  id_ = Id();
}

void Lthread::Join(uint64_t timeout_ms)
{
  if (id_ != Id()) {
    int r = lthread_join(id_.lt_, nullptr, timeout_ms);
    if (r == -2)
      throw LthreadTimeout();
  }

  id_ = Id();
}

void lthread_cpp::LthreadExecute(void* arg)
{
  Lthread::_Impl_base* t = static_cast<Lthread::_Impl_base*>(arg);
  Lthread::__shared_base_type local;
  local.swap(t->_M_this_ptr);

  try {
    t->_M_run();
  } catch(...) {
    std::terminate();
  }
}



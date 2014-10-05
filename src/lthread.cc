#include "lthread_cpp/lthread.h"

using namespace lthread_cpp;
void Lthread::Detach()
{
  if (id_ != Id())
    lthread_detach2(id_.lt_);

  id_ = Id();
}

void Lthread::Join()
{
  if (id_ != Id())
    lthread_join(id_.lt_, nullptr, 0);

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



#ifndef __NON_COPYABLE_H__
#define __NON_COPYABLE_H__

class NonCopyable {
public:
  NonCopyable(const NonCopyable &) = delete;

  NonCopyable(NonCopyable &&) = delete;

  NonCopyable &operator=(const NonCopyable &) = delete;

  NonCopyable &operator=(NonCopyable &&) = delete;

protected:
  NonCopyable() = default;

  ~NonCopyable() = default;
};

#endif
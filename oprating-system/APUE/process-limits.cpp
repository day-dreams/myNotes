#include <iostream>
#include <sys/resource.h>

rlim_t get_resource(rlim_t resource_type, bool cur = true) {
  static rlimit limit;
  if (getrlimit(resource_type, &limit) == -1)
    return -1;
  if (cur)
    return limit.rlim_cur;
  else
    return limit.rlim_max;
}

int main(int argc, char const *argv[]) {
  std::cout << "stack limits:\t" << get_resource(RLIMIT_STACK) << '\t'
            << get_resource(RLIMIT_STACK, false) << '\n';
  return 0;
}

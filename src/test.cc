#include "pls.h"

PLS_INCLUDE_HEADER_ONLY_CURRENT();
PLS_ADD_DEP("hiredis", "https://github.com/redis/hiredis");

#include "redis.h"
const std::string golden =
  "Authorization failed\n"
"set a 1\n"
"get a\n"
"a=1\n"
"Command failed\n"
"eval=ERR wrong number of arguments for 'eval' command\n"
"list len=1 \n"
"elementslen=2 \n"
"field1\n"
"field2";

int main(int argc, char **argv) {
  /* Test plan:
      1. Incorrect auth
      2. Correct auth
      3. Status
      4. String command
      5. Failed command
      6. Integer command
      7. Array command
   */
  if (argc > 1 && argv[1] == std::string("--golden")) {
    std::cout << golden << std::endl;
    return 0;
  }
  int is_fail = 0;

  // Auth failed
  auto c = RedisClient("127.0.0.1", 6379, "worng-login", "test");

  // Correct auth
  c = RedisClient("127.0.0.1", 6379, "default", "test");
  c.EnableKeepAlive();

  // String command with status
  auto resp = c.SendCommand("set a 1");
  std::cout << "set a 1" << std::endl;
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
    is_fail = 1;
  }

  // Command with string result
  resp = c.SendCommand("get a");
  std::cout << "get a" << std::endl;
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
    is_fail = 1;
  }
  std::cout << "a=" << resp.string << std::endl;

  // failed command
  resp = c.SendCommand("eval \"1+1\"");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
  } else {
    is_fail = 1;
  }
  std::cout << "eval=" << resp.string << std::endl;

  // Command with number result
  resp = c.SendCommand("LPUSH mylist hello");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
    is_fail = 1;
  }
  resp = c.SendCommand("LLEN mylist");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
    is_fail = 1;
  }
  std::cout << "list len=" << resp.integer << " " << resp.string << std::endl;

  // Command with elements result
  resp = c.SendCommand("HSET myhash field1 \"Hello\"");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
    is_fail = 1;
  }
  resp = c.SendCommand("HSET myhash field2 \"World\"");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
    is_fail = 1;
  }
  resp = c.SendCommand("HKEYS myhash");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
    is_fail = 1;
  }
  std::cout << "elementslen=" << resp.elements.size() << " " << resp.string << std::endl;
  for (auto elem : resp.elements) {
    std::cout << elem.string << std::endl;
  }

  return is_fail;
}

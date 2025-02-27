## redis
Blocking redis client for Current

Features:
* Automatic reconnect (from hiredis lib)
* RESP 2 protocol support (string, integer, status and array)
* Keep-alive
* TCP timeout
* RESP2 to CUrrent struct parser

Build
```
#pip install plsbuild
#pls build
```

Testing (requires docker)
```
make test
```

Minimal example
```C++
  // Connect and auth
  auto client = RedisClient("127.0.0.1", 6379, "default", "test");

  // Basic command
  auto resp = client.SendCommand("set a 1");

  // Command with result
  resp = client.SendCommand("get a");
  std::cout << "a=" << resp.string << std::endl;

  // Command with result
  resp = client.SendCommand("LLEN mylist");
  std::cout << "list len=" << resp.integer << std::endl;
```

Full example
```C++
  auto c = RedisClient("127.0.0.1", 6379, "default", "test");
  c.EnableKeepAlive();

  auto resp = c.SendCommand("set a 1");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
  }

  resp = c.SendCommand("get a");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
  }
  if(!resp.IsString()) {
    std::cout << "Wrong response" << std::endl;
  }
  std::cout << "a=" << resp.string << std::endl;

  resp = c.SendCommand("LLEN mylist");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
  }
  if(!resp.IsInteger()) {
    std::cout << "Wrong response" << std::endl;
  }
  std::cout << "list len=" << resp.integer << std::endl;

  resp = c.SendCommand("HKEYS myhash");
  if (!resp.IsOK()) {
    std::cout << "Command failed" << std::endl;
  }
  if(!resp.IsArray()) {
    std::cout << "Wrong response" << std::endl;
  }
  std::cout << "elementslen=" << resp.elements.size() << std::endl;
  for(auto elem : resp.elements) {
    if(!elem.IsString()) {
      std::cout << "Wrong response" << std::endl;
    }
    std::cout << elem.string << std::endl;
  }

```

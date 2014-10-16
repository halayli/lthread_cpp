#ifndef LTHREAD_CPP_SHELL_H
#define LTHREAD_CPP_SHELL_H

#include <inttypes.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <memory>
#include <array>
#include <functional>

#include "lthread_cpp/lthread.h"
#include "lthread_cpp/listener.h"
#include "lthread_cpp/socket.h"

using namespace lthread_cpp;
using namespace lthread_cpp::net;

const int kMaxLineLength = 1024 * 1024 * 16;
const int kMaxArguments = 64;

enum ShellArgumentValueType {
  kShellArgumentTypeString,
  kShellArgumentTypeNumber,
  kShellArgumentTypeUnknown
};

namespace lthread_cpp {
namespace net {
// An instance of a shell argument passed to us.
class ShellArgument {
 public:
  ShellArgument() {}
  inline void SetArgument(const char* value, ShellArgumentValueType t)
  {
    value_ = value;
    type_ = t;
  }

  inline bool Verify() const
  {
    if (type_ == kShellArgumentTypeNumber) {
      char *q;
      long r = strtol(value_, &q, 10);
      if ((r == 0 || errno == ERANGE) && value_ == q)
        return false;
    }

    return true;
  }

  inline long Number(void) const
  {
    return strtol(value_, NULL, 10);
  }

  inline const char* Str() const
  {
    return value_;
  }

 private:
  const char* value_;
  ShellArgumentValueType type_;
};

class ShellClientHandler;

using ShellArguments = std::array<ShellArgument, kMaxArguments>;
// Callback signature

using ShellCallback = std::function<void(ShellClientHandler*, const ShellArguments&)>;

// callback to handle specific mnemonics
typedef void (*MnemonicCallback)(std::string filename);

// An instance of a registered command.
struct ShellCommand {
  std::vector<std::pair<std::string, ShellArgumentValueType>> declarations;
  ShellCallback callback;
  std::string help;
  std::string Description(void) const;
};

class Shell {
 public:
  Shell (const std::string &ip_address, uint16_t port);
  ~Shell();
  uint16_t port() const { return port_;}
  const std::string& ip_address() const { return ip_address_; }
  const std::string Help();
  const std::string DescribeCommand(const std::string& cmd) const;
  bool RegisterCommand(const std::string& name,
                       const std::string& desc,
                       const std::string &declaration,
                       ShellCallback callback);
  void RegisterCallback(const std::string& mnemonic, MnemonicCallback callback);
  const ShellCommand* GetCommand(const std::string &command_name) const;
  void Start();
  void Stop() { shutdown_ = true; listener_.Close(); }
  Shell(const Shell& that) = delete;
  bool shutdown() const { return shutdown_; }

  ShellArgumentValueType GetShellArgumentValueType(const char* p) const;

 private:
  Shell();
  void Cleaner();
  void Listen();
  void Accept();
  void HandleClient(Socket&);

  std::string ip_address_;
  uint16_t port_;
  std::unordered_map<std::string, ShellCommand> commands_;
  std::vector<Lthread> clients_;
  Lthread acceptor_thread_;
  Lthread cleaner_thread_;
  bool shutdown_;
  TcpListener listener_;
};

class ShellClientHandler {
 public:
  ShellClientHandler(Socket& client, Shell* shell)
    : client_(std::move(client)),
      shell_(shell), enable_console_(false), read_(0), pos_(0) {
      line_ = new char[kMaxLineLength];
    }
  ~ShellClientHandler() {
    delete line_;
  }
  void Run(void);
  void ParseExecCommand(char* line, int len);
  void Write(const std::string& output, bool newline=true, bool bare=false);
  void InvalidArgument(const std::string& desc);
  void CommandNotFound();
  void EnableConsole();
  void Close();
  Shell* shell() const { return shell_; }

 private:
  bool ReadLine(char** line, int* len);
  Socket client_;
  Shell* shell_;
  bool enable_console_;
  char* line_;
  int read_;
  int pos_;
  std::array<ShellArgument, kMaxArguments> arguments_;
};

}
}

#endif

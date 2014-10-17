#include "lthread_cpp/shell.h"

#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include <string>
#include <map>
#include <vector>

static std::string shell_types[] = {
  std::string("string"),
  std::string("number"),
};

std::string ShellCommand::Description(void) const
{
  std::string tmp;

  for (auto it : declarations)
    tmp += "<" + it.first + ":"+ shell_types[it.second] +"> ";

  return tmp;
}

void EnableConsoleCmd(ShellClientHandler* sh_client, const ShellArguments& args)
{
  sh_client->EnableConsole();
  sh_client->Write("Console echo turned on");
  sh_client->Write("Webmon common backdoor.");
}

void PrintHelpCmd(ShellClientHandler* sh_client, const ShellArguments& args)
{
  sh_client->Write(sh_client->shell()->Help());
}

void HelpCmd(ShellClientHandler* sh_client, const ShellArguments& args)
{
  auto& cmd = args[0];
  sh_client->Write(sh_client->shell()->DescribeCommand(cmd.Str()));
}

void QuitCmd(ShellClientHandler* sh_client, const ShellArguments& args)
{
  sh_client->Write("Bye!");
  sh_client->Close();
}

void ShellClientHandler::Write(const std::string& output,
                               bool newline,
                               bool bare)
{

  if (!client_.IsConnected() || !enable_console_)
    return;

  std::string final;
  if (bare)
    final = output;
  else
    final = "% " + output;
  if (newline)
    final += "\n";

  try {
    client_.Send(final.c_str(), final.length(), 0);
  } catch (...) {
    Close();
  }
}

void ShellClientHandler::InvalidArgument(const std::string& desc)
{
  std::string s("Invalid argument: ");
  s += desc;
  Write(s.c_str());
}

void ShellClientHandler::CommandNotFound()
{
  Write("Command not found");
}

void ShellClientHandler::Close()
{
  client_.Close();
}

void ShellClientHandler::EnableConsole(void)
{
  enable_console_ = true;
}

enum ParseState {
  kParseQuotedField,
  kParseEndQuotedField,
  kParseBeginField,
  kParseEndField,
  kParseEndRec,
  kParseUnknown
};

void ShellClientHandler::ParseExecCommand(char* line, int len)
{

  ParseState st = kParseUnknown;
  char* field_start = nullptr;
  char* tmp = line;
  bool strip_quote = true;
  char quote = '\0';
  char* tokens[kMaxArguments];
  int num_tokens = 0;

  if (!line || !*line || *line == '\n')
    return;

  while (tmp < (line + len)) {
    switch (st) {
      case kParseBeginField:
        if (isspace(*tmp) || *tmp == '\n')
          st = kParseEndField;
        break;
      case kParseEndField:
        if (kMaxArguments == num_tokens) {
          st = kParseUnknown;
          break;
        }
        if (quote && strip_quote)
          field_start += 1;
        tokens[num_tokens++] = field_start;
        st = (*tmp == '\n') ? kParseEndRec : kParseUnknown;
        if (quote && strip_quote)
          *(tmp - 1) = '\0';
        else
          *tmp = '\0';
        quote = '\0';
        break;
      case kParseUnknown:
        field_start = tmp;
        switch(*tmp) {
          case '"':
          case '\'':
            quote = *tmp;
            st = kParseQuotedField;
            break;
          case '\n':
            st = kParseUnknown;
            break;
          default:
            st = kParseBeginField;
        }
        break;
      case kParseQuotedField:
        if (*tmp == quote || *tmp == '\n')
          st = kParseEndQuotedField;
        break;

      case kParseEndQuotedField:
        if (!isspace(*tmp) && *tmp != '\n') {
          st = kParseQuotedField;
          break;
        }
        st = kParseEndField;
        break;
      default:
        break;
    }

    // don't advance to give kParseEndField state
    // a chance to terminate the field with a null
    if (st == kParseEndRec)
      break;
    if (st != kParseEndField && st != kParseEndRec)
      tmp++;
  }

  // empty command
  if (num_tokens == 0) {
    return;
  }

  // command not found. return an error
  const ShellCommand* command = shell_->GetCommand(tokens[0]);
  if (!command) {
    CommandNotFound();
    return;
  }

  // command arguments don't match with declaration
  // return;
  if (command->declarations.size() != (unsigned)(num_tokens - 1)) {
    std::string tmp = std::string(line) + "\n";
    tmp += "Expecting: ";
    tmp += command->Description();
    InvalidArgument(tmp);
    return;
  }

  for (int i = 0; i < num_tokens - 1; i++) {
    arguments_[i].SetArgument(tokens[i + 1], command->declarations[i].second);
    if (!arguments_[i].Verify()) {
      std::string tmp;
      tmp = "Failed to convert " + command->declarations[i].first;
      tmp += " to a number\n";
      tmp += command->Description();
      InvalidArgument(tmp);
      return;
    }
  }

  command->callback(this, arguments_);
}

bool ShellClientHandler::ReadLine(char** line, int* len)
{

  while (!shell_->shutdown()) {

    bool found = false;
    int j = 0;
    for (int i = pos_; i < read_; i++, j++)
      if (line_[i] == '\n') {
          *len = j + 1;
      *line = &line_[pos_];
      pos_ = i + 1;
      found = true;
      break;
      } else if (line_[i] == '\0') {
        line_[i] = '?';
        continue;
      }

    if (found) {
      break;
    } else if (read_ != pos_ && read_ > (0.9 * kMaxLineLength)) {
      // move and start over when the buffer is 90% used
      memmove(line_, line_ + pos_, read_ - pos_);
      read_ = read_ - pos_;
      pos_ = 0;
    }

    while (!shell_->shutdown()) {
      try {
        read_ += client_.Recv(line_ + read_, kMaxLineLength - read_, 1000);
        break;
      } catch (SocketTimeout& e) {
        continue;
      }
    }

  }
  return true;
}

void ShellClientHandler::Run(void)
{
  EnableConsole();
  while (!shell_->shutdown()) {
    Write("", false);
    char* line = nullptr;
    int len = 0;
    try {
      ReadLine(&line, &len);
      ParseExecCommand(line, len);
    } catch(...) {
        break;
    }
  }
  client_.Close();
}

const ShellCommand* Shell::GetCommand(const std::string& command_name) const
{
  auto it = commands_.find(command_name);
  if (it == commands_.end())
    return nullptr;

  return &it->second;
}

void Shell::Accept()
{
  cleaner_thread_ = Lthread{&Shell::Cleaner, this};
  while (!shutdown_) {
    try {
      Socket cli_socket = listener_.Accept();
      clients_.push_back(Lthread{&Shell::HandleClient, this, std::move(cli_socket)});
    } catch(...) {
      break;
    }
  }
}

void Shell::Cleaner()
{
  while (!shutdown_ || clients_.size()) {
    for (auto it = clients_.begin(); it != clients_.end(); )
      try {
        it->Join(100);
        it = clients_.erase(it);
      } catch(Lthread::LthreadTimeout& e) {
        ++it;
      }

    lthread_sleep(500);
  }
}

// Handles the lifetime of a single shell client connection
void Shell::HandleClient(Socket& client_socket)
{
  ShellClientHandler client_shell(client_socket, this);
  client_shell.Run();
}

const std::string Shell::DescribeCommand(const std::string& cmd) const
{
  const ShellCommand* command = GetCommand(cmd);
  if (!command)
    return std::string("Command not found");

  return command->Description();
}

Shell::~Shell()
{
  shutdown_ = true;
  acceptor_thread_.Join();
  cleaner_thread_.Join();
}

Shell::Shell(const std::string& ip_address, uint16_t port)
{
  shutdown_ = false;
  ip_address_ = ip_address;
  port_ = port;
  RegisterCommand("enable.console",
                  "Turns console echo on",
                  "",
                  EnableConsoleCmd);

  RegisterCommand("?", "Print shell commands", "", PrintHelpCmd);
  RegisterCommand("help", "Prints command description", "cmd:string", HelpCmd);
  RegisterCommand("quit", "Quit shell", "", QuitCmd);
}

ShellArgumentValueType Shell::GetShellArgumentValueType(const char* p) const
{
  if (strcasecmp(p, "string") == 0)
    return kShellArgumentTypeString;
  if (strcasecmp(p, "number") == 0)
    return kShellArgumentTypeNumber;

  return kShellArgumentTypeUnknown;
}

bool Shell::RegisterCommand(const std::string& name,
                            const std::string& help,
                            const std::string& declaration,
                            ShellCallback callback)
{
  std::vector<std::pair<std::string, ShellArgumentValueType>> declarations;
  char *brkc;

  for (char *argument = strtok_r((char *)declaration.c_str(), ",", &brkc);
       argument;
       argument = strtok_r(NULL, ",", &brkc)) {
    char *p = strchr(argument, ':');
    if (!p)
      return false;

    *p = '\0';
    p++;
    ShellArgumentValueType t = Shell::GetShellArgumentValueType(p);
    if (t == kShellArgumentTypeUnknown) {
      warnx("Unknown shell argument type: %s", p);
      continue;
    }

    while (isspace(*argument))
      argument++;
    declarations.push_back(
      std::pair<std::string, ShellArgumentValueType>(std::string(argument), t));
  }

  ShellCommand command;
  command.callback = callback;
  command.declarations = declarations;
  command.help = help;

  commands_[name] = command;

  return true;
}

void Shell::Start()
{

  listener_ = TcpListener(ip_address_, port_);
  listener_.Listen();
  acceptor_thread_ = Lthread{&Shell::Accept, this};
}

const std::string Shell::Help() {
  std::stringstream ss;

  // sort commands
  std::map<std::string, ShellCommand> ordered(commands_.begin(), commands_.end());
  for (auto it : ordered) {
    ss.flags(std::ios::left);
    ss.width(50);
    ss << it.first;
    ss << it.second.help << "\n";
  }

  return ss.str();
}

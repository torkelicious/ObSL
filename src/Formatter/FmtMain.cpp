#include "Formatter.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
  bool stdout_mode = false;
  bool check = false;
};

bool read_all(const std::string &path, std::string &out) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return false;
  std::ostringstream ss;
  ss << in.rdbuf();
  out = ss.str();
  return true;
}

bool write_all(const std::string &path, const std::string &data) {
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out)
    return false;
  out.write(data.data(), static_cast<std::streamsize>(data.size()));
  return static_cast<bool>(out) && !out.bad();
}

void print_usage() {
  std::cout
      << "usage: obsl_fmt [files...]\n"
         "       obsl_fmt --stdout [files...]   # print formatted output\n"
         "       obsl_fmt --check [files...]    # list files that need "
         "formatting\n"
         "       obsl_fmt < file              # format stdin to stdout\n";
}

} // namespace

int main(const int argc, char *argv[]) {
  Options options;
  std::vector<std::string> files;

  for (int i = 1; i < argc; ++i) {
    const std::string_view arg(argv[i]);
    if (arg == "--stdout") {
      options.stdout_mode = true;
    } else if (arg == "--check") {
      options.check = true;
    } else if (arg == "--help" || arg == "-h") {
      print_usage();
      return 0;
    } else if (!arg.empty() && arg.front() == '-') {
      std::cerr << "obsl_fmt: unknown option: " << arg << '\n';
      print_usage();
      return 2;
    } else {
      files.emplace_back(arg);
    }
  }

  const bool read_stdin = files.empty();
  if (read_stdin && options.check) {
    std::cerr << "obsl_fmt: --check needs at least one file\n";
    return 2;
  }

  if (read_stdin) {
    std::ostringstream ss;
    ss << std::cin.rdbuf();
    try {
      std::cout << ObSL::format_source(ss.str());
    } catch (const std::exception &e) {
      std::cerr << "obsl_fmt: <stdin>: " << e.what() << '\n';
      return 2;
    }
    return 0;
  }

  int exit_code = 0;
  for (const std::string &path : files) {
    std::string source;
    if (!read_all(path, source)) {
      std::cerr << "obsl_fmt: cannot read: " << path << '\n';
      exit_code = 2;
      continue;
    }

    std::string result;
    try {
      result = ObSL::format_source(source);
    } catch (const std::exception &e) {
      std::cerr << "obsl_fmt: " << path << ": " << e.what() << '\n';
      exit_code = 2;
      continue;
    }

    if (options.stdout_mode) {
      std::cout << result;
    } else if (options.check) {
      if (result != source) {
        std::cout << path << '\n';
        exit_code = 1;
      }
    } else if (result != source && !write_all(path, result)) {
      std::cerr << "obsl_fmt: cannot write: " << path << '\n';
      exit_code = 2;
    }
  }

  return exit_code;
}

#include <ObSL/ScriptRuntime.h>
#include <iostream>

namespace ObSL {
void ScriptRuntime::init(const std::string &script_root,
                         const size_t worker_count) {
  if (!m_Workers.empty())
    return;
  m_Workers.reserve(worker_count);
  for (size_t i = 0; i < worker_count; ++i) {
    m_Workers.push_back(std::make_unique<ScriptWorker>(script_root));
  }
  std::cout << "[ScriptRuntime] Initialized with " << worker_count
            << " script worker(s)\n";
}

void ScriptRuntime::set_stdout(std::ostream &out) const {
  for (const auto &worker : m_Workers) {
    worker->GetInterpreter().Set_Stdout(out);
  }
}

void ScriptRuntime::set_script_root(const std::string &path) const {
  for (const auto &worker : m_Workers) {
    worker->GetInterpreter().set_script_root(path);
  }
}
} // namespace ObSL

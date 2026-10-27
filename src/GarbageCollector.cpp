#include <ObSL/GarbageCollector.h>
#include <ObSL/Interpreter.h>
#include <ObSL/Parser/ast.h>

namespace ObSL {
    void mark_value(const Value &val) {
        if (const auto obj = std::get_if<ObSLObject *>(&val); obj && *obj && !(*obj)->is_marked) {
            (*obj)->is_marked = true;
            g_mark_pending.push_back(*obj);
        } else if (const auto arr = std::get_if<ObSLArray *>(&val); arr && *arr && !(*arr)->is_marked) {
            (*arr)->is_marked = true;
            g_mark_pending.push_back(*arr);
        } else if (const auto callable = std::get_if<ObSLCallable *>(&val); callable && *callable && !(*callable)->is_marked) {
            (*callable)->is_marked = true;
            g_mark_pending.push_back(*callable);
        }
    }

    void GarbageCollector::collect() {
        // mark external roots first (shared)
        {
            std::lock_guard root_lock(m_RootMutex);
            for (auto *root : m_ExternalRoots) {
                if (root)
                    root->mark();
            }
        }

        interpreter->mark_roots();

        while (!g_mark_pending.empty()) {
            GCObject *current = g_mark_pending.back();
            g_mark_pending.pop_back();
            current->mark();
        }

        GCObject **obj = &first_obj;
        while (*obj != nullptr) {
            if (!(*obj)->is_marked) {
                const GCObject *unreached = *obj;
                *obj = unreached->next;
                delete unreached;
                allocated_objs--;
            } else {
                (*obj)->is_marked = false;
                obj = &(*obj)->next;
            }
        }
        // keep a floor so an empty heap does not collect every allocation
        gc_threshold = std::max<size_t>(allocated_objs * 2, 256);
    }
} // namespace ObSL

#include <ObSL/GarbageCollector.h>
#include <ObSL/Interpreter.h>
#include <ObSL/Parser/ast.h>

namespace ObSL {
    void mark_value(const Value &val) {
        if (const auto obj = std::get_if<ObSLObject *>(&val); obj && *obj) {
            (*obj)->mark();
        } else if (const auto arr = std::get_if<ObSLArray *>(&val); arr && *arr) {
            (*arr)->mark();
        } else if (const auto callable = std::get_if<ObSLCallable *>(&val); callable && *callable) {
            (*callable)->mark();
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

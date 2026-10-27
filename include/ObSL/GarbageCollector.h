#pragma once

#include <algorithm>
#include <mutex>
#include <utility>
#include <vector>

namespace ObSL {
    class Interpreter;

    struct GCObject {
        bool is_marked = false;
        GCObject *next = nullptr;

        virtual ~GCObject() = default;

        virtual void mark() = 0;
    };

    inline thread_local std::vector<GCObject *> g_mark_pending;

    class GarbageCollector {
        GCObject *first_obj = nullptr;
        size_t allocated_objs = 0;
        size_t gc_threshold = 1000;
        Interpreter *interpreter;
        std::vector<GCObject *> m_ExternalRoots;
        mutable std::mutex m_RootMutex;

    public:
        explicit GarbageCollector(Interpreter *interpreter) : interpreter(interpreter) {}

        GarbageCollector(const GarbageCollector &) = delete;
        GarbageCollector &operator=(const GarbageCollector &) = delete;

        ~GarbageCollector() {
            const GCObject *obj = first_obj;
            while (obj != nullptr) {
                const GCObject *next = obj->next;
                delete obj;
                obj = next;
            }
        }

        template <typename T, typename... Args> T *allocate(Args &&...args) {
            if (allocated_objs >= gc_threshold) {
                collect();
            }
            T *object = new T(std::forward<Args>(args)...);
            object->next = first_obj;
            first_obj = object;
            allocated_objs++;
            return object;
        }

        void collect();

        // thread-safe: collect() holds the same lock
        void add_root(GCObject *obj) {
            if (!obj)
                return;
            std::lock_guard lock(m_RootMutex);
            m_ExternalRoots.push_back(obj);
        }

        void remove_root(GCObject *obj) {
            if (!obj)
                return;
            std::lock_guard lock(m_RootMutex);
            if (const auto it = std::find(m_ExternalRoots.begin(), m_ExternalRoots.end(), obj); it != m_ExternalRoots.end())
                m_ExternalRoots.erase(it);
        }
    };
} // namespace ObSL

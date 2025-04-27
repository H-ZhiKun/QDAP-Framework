#pragma once
#include <atomic>
#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

namespace _Kits
{
    template <typename T>
    class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>>
    {
      private:
        struct ObjectNode
        {
            T *obj = nullptr;
            ObjectNode *next = nullptr;
        };

        std::atomic<ObjectNode *> m_head{nullptr};     // 对象池栈
        std::atomic<ObjectNode *> m_nodePool{nullptr}; // Node 缓存栈
        std::atomic<size_t> m_nodePoolSize{0};         // 当前缓存的 Node 数量
        const size_t m_maxNodeCacheSize;               // 最大 Node 缓存数

      public:
        explicit ObjectPool(size_t maxNodeCacheSize = 64)
            : m_maxNodeCacheSize(maxNodeCacheSize)
        {
        }

        ~ObjectPool()
        {
            auto node = m_head.load();
            while (node)
            {
                auto next = node->next;
                delete node->obj;
                recycleNode(node); // 回收 Node 到缓存池
                node = next;
            }

            node = m_nodePool.load();
            while (node)
            {
                auto next = node->next;
                delete node;
                node = next;
            }
        }

        template <typename... Args>
        std::shared_ptr<T> getObject(Args &&...args)
        {
            T *rawPtr = nullptr;
            ObjectNode *node = popNode();
            if (node)
            {
                rawPtr = node->obj;
                recycleNode(node);
                rawPtr->reset();
            }
            else
            {
                rawPtr = new T(std::forward<Args>(args)...);
            }

            std::weak_ptr<ObjectPool> weakSelf = this->shared_from_this();

            return std::shared_ptr<T>(rawPtr, [weakSelf](T *ptr) {
                if (auto self = weakSelf.lock())
                {
                    self->pushNode(ptr);
                }
                else
                {
                    delete ptr;
                }
            });
        }

      private:
        void pushNode(T *ptr)
        {
            ObjectNode *node = obtainNode();
            node->obj = ptr;

            ObjectNode *old_head = m_head.load(std::memory_order_relaxed);
            do
            {
                node->next = old_head;
            } while (!m_head.compare_exchange_weak(old_head,
                                                   node,
                                                   std::memory_order_release,
                                                   std::memory_order_relaxed));
        }

        ObjectNode *popNode()
        {
            ObjectNode *old_head = m_head.load(std::memory_order_acquire);
            while (old_head)
            {
                ObjectNode *next = old_head->next;
                if (m_head.compare_exchange_weak(old_head,
                                                 next,
                                                 std::memory_order_acquire,
                                                 std::memory_order_relaxed))
                {
                    return old_head;
                }
            }
            return nullptr;
        }

        ObjectNode *obtainNode()
        {
            ObjectNode *node = m_nodePool.load(std::memory_order_acquire);
            while (node)
            {
                ObjectNode *next = node->next;
                if (m_nodePool.compare_exchange_weak(node,
                                                     next,
                                                     std::memory_order_acquire,
                                                     std::memory_order_relaxed))
                {
                    m_nodePoolSize--;
                    return node;
                }
            }
            return new ObjectNode;
        }

        void recycleNode(ObjectNode *node)
        {
            node->obj = nullptr;

            if (m_nodePoolSize.load(std::memory_order_relaxed) >=
                m_maxNodeCacheSize)
            {
                delete node;
                return;
            }

            ObjectNode *old_head = m_nodePool.load(std::memory_order_relaxed);
            do
            {
                node->next = old_head;
            } while (
                !m_nodePool.compare_exchange_weak(old_head,
                                                  node,
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed));

            m_nodePoolSize++;
        }
    };
} // namespace _Kits
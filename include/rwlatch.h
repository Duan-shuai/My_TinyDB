#ifndef RWLATCH
#define RWLATCH

#include <mutex>
#include <shared_mutex>
namespace db
{
    class ReaderWriterLatch
    {
    public:
        void WLock()
        {
            rwlock.lock();
        }
        void WUnlock()
        {
            rwlock.unlock();
        }
        void Rlock()
        {
            rwlock.lock_shared();
        }
        void RUnlock()
        {
            rwlock.unlock_shared();
        }

    private:
        std::shared_mutex rwlock;
    };
}

#endif
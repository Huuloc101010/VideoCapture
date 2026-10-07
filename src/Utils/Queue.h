#ifndef QUEUE
#define QUEUE

#include <condition_variable>
#include <deque>
#include <mutex>

template<typename T>
class Queue
{
public:
    Queue(int LimitSize = 100)
    {
        m_LimitSize = LimitSize;
        m_IsStop = false;
    }

    void Push(T Value)
    {
        std::unique_lock<std::mutex> Lock(m_Mutex);
        m_ConditionVariable.wait(Lock, [&]
        {
            return ((m_Data.size() < m_LimitSize) || (m_IsStop == true));
        });
        if(m_IsStop == true)
        {
            LOGW("Can not push, Stop flag is true");
            return;
        }
        m_Data.push_back(std::move(Value));
        Lock.unlock();
        m_ConditionVariable.notify_one();
    }

    T Pop()
    {
        std::unique_lock<std::mutex> Lock(m_Mutex);
        m_ConditionVariable.wait(Lock, [&]
        {
            return ((m_Data.size() > 0) || (m_IsStop == true));
        });
        if((m_Data.size() == 0) && (m_IsStop == true))
        {
            return {};
        }
        T Tmp = std::move(m_Data.front());
        m_Data.pop_front();
        Lock.unlock();
        m_ConditionVariable.notify_one();
        return std::move(Tmp);
    }

    void Clear()
    {
        std::unique_lock<std::mutex> Lock(m_Mutex);
        m_Data.clear();
        m_ConditionVariable.notify_all();
    }

    void Stop()
    {
        std::unique_lock<std::mutex> Lock(m_Mutex);
        m_Data.clear();
        m_IsStop = true;
        m_ConditionVariable.notify_all();
    }

    void Start()
    {
        std::unique_lock<std::mutex> Lock(m_Mutex);
        m_Data.clear();
        m_IsStop = false;
        m_ConditionVariable.notify_all();
    }

private:
    std::deque<T>   m_Data;
    std::mutex      m_Mutex;
    int             m_LimitSize;
    std::atomic<bool>       m_IsStop;
    std::condition_variable m_ConditionVariable;
};

#endif // QUEUE
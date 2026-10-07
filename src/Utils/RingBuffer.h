#ifndef RING_BUFFER
#define RING_BUFFER

#include <array>
#include <mutex>
#include <condition_variable>
#include <deque>

// template<typename T, int Size>
// class RingBuffer
// {
// public:
//     RingBuffer()
//     {
//         m_Data = {};
//         m_Head = 1;
//         m_Tail = 0;
//     }

//     void Push(T Data)
//     {
//         std::lock_guard<std::mutex> Lock(m_Mutex);
//         m_Data[m_Head] = Data;
//         m_Head = (m_Head + 1) % Size;
//     }

//     T Pop()
//     {
//         std::lock_guard<std::mutex> Lock(m_Mutex);
//         T Retval = std::move(m_Data[m_Tail]);
//         m_Tail = (m_Tail + 1) % Size;
//         return Retval;
//     }

//     void Stop()
//     {

//     }

// private:
//     std::mutex          m_Mutex;
//     std::array<T, Size> m_Data;
//     int                 m_Head;
//     int                 m_Tail;
//     std::condition_variable m_CV;
// };


template<typename T>
class RingBuffer
{
public:
    RingBuffer()
    {

    }

    void Push(T Data)
    {
        std::lock_guard<std::mutex> Lock(m_Mutex);
        m_Data.push_back(std::move(Data));
    }

    T Pop()
    {
        std::lock_guard<std::mutex> Lock(m_Mutex);
        T Retval = {};
        if(m_Data.size())
        {
            Retval = std::move(m_Data[0]);
            m_Data.pop_front();
        }
        else
        {
            LOGW("No data in queue");
        }
        return Retval;
    }

    void Stop()
    {
        
    }

private:
    std::deque<T>   m_Data;
    std::mutex      m_Mutex;
};

#endif // RING_BUFFER
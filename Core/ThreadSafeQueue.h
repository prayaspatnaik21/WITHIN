#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>


template <typename T>
class ThreadSafeQueue
{
    public:
        ThreadSafeQueue(int capacity)
        :capacity(capacity)
        {

        }

        void push(T data)
        {
            {
                std::unique_lock<std::mutex> lock(mtx);
                
                if(buffer.size() >= capacity)
                {
                    buffer.pop();
                }

                buffer.push(std::move(data));
            }

            cv_not_empty.notify_one();
        }

        T pop()
        {
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv_not_empty.wait(lock , [this](){
                    return buffer.size() > 0;
                });

                T data = buffer.front();
                buffer.pop();
            }
            return data;
        }
    private:
        int capacity;
        std::queue<T> buffer;
        std::mutex mtx;
        std::condition_variable cv_not_empty;
};
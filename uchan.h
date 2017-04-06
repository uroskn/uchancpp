/**
 *  Copyright [current year] and all that hardcore legal stuff.
 *               -- uroskn <uros@knuples.net>
 *
 *  Before anyone asks for license; It's WTFPL.
 **/

#include <list>
#include <thread>
#include <condition_variable>
#include <stdexcept>
#include <chrono>

template <typename T>
class uchan {
private:
    std::list<T>            queue;
    size_t                  size;
    bool                    c_closed;
    std::mutex              mtx;
    std::condition_variable cv;
public:

    uchan() : size(1), c_closed(false) { /* lol */ };
    uchan(int size) : size(size), c_closed(false) { /* lol */ };

    size_t getSize() { return this->size; }
    
    void resize(size_t new_size)
    {
        std::unique_lock<std::mutex> lck(this->mtx);
        this->size = new_size;
        this->cv.notify_all();
    }

    bool put(T value) { return this->put(value, -1); }

    bool put(T value, int timeout)
    {
        std::unique_lock<std::mutex> lck(this->mtx);
        if (this->queue.size() >= this->size)
        {
            if (timeout == 0) return false;
            else if (timeout == -1)
                this->cv.wait(lck, [&]() { return ((this->queue.size() < this->size) || (this->c_closed)); });
            else
            {
                bool res = this->cv.wait_for(
                    lck, std::chrono::seconds(timeout),
                    [&]() { return ((this->queue.size() < this->size) || (this->c_closed)); }
                );
                if (res == false) return false;
            }
        }
        if (this->c_closed)
            return false;
        this->queue.push_back(value);
        this->cv.notify_all();
        return true;
    }

    bool get(T& result) { return this->get(result, -1); }

    bool get(T& result, int timeout)
    {
        std::unique_lock<std::mutex> lck(this->mtx);
        if (this->queue.empty())
        {
            if ((this->c_closed) || (timeout == 0)) return false;
            if (timeout == -1)
                this->cv.wait(lck, [&]() { return (this->c_closed || !this->queue.empty()); });
            else
            {
                bool res = this->cv.wait_for(
                    lck, std::chrono::seconds(timeout),
                    [&]() { return (this->c_closed || !this->queue.empty()); }
                );
                if (res == false) return false;
            }
        }
        if (this->queue.empty())
            return false;
        result = this->queue.front();
        this->queue.pop_front();
        this->cv.notify_one();
        return true;
    }

    size_t waiting()
    {
        std::unique_lock<std::mutex> lck(this->mtx);
        return this->queue.size();
    }

    void close()
    {
        std::unique_lock<std::mutex> lck(this->mtx);
        if (this->c_closed)
            throw new std::invalid_argument("Closing closed channel");
        this->c_closed = true;
        this->cv.notify_all();
    }

    bool closedWrite()
    {
        std::unique_lock<std::mutex> lck(this->mtx);
        return this->c_closed;
    }

    bool closed()
    {
        std::unique_lock<std::mutex> lck(this->mtx);
        return (this->c_closed && (this->queue.size() == 0));
    }
};

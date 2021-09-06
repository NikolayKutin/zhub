// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

class Event { // <----------- ToDo: std(boost)
public:
    Event() {  }

    ~Event() {  }

    bool wait(std::chrono::duration<int, std::milli> timeout) { 
         std::unique_lock<std::mutex> lock(m);
         return (cond_var.wait_for(lock, timeout) != std::cv_status::timeout);
    }

    void set() { cond_var.notify_all(); }

    void reset() { }
private:
    Event(const Event& copiedObject) = delete;
    Event& operator= (const Event& event) = delete;

    std::condition_variable cond_var;
    std::mutex m;
};

template <typename ARGUMENT_T> class EventEmitter { // TODO: KEY_T
public:
    struct Listener {
        std::shared_ptr<Event> event;
        std::shared_ptr<ARGUMENT_T> argument;
    };

    void emit(int event_id, ARGUMENT_T argument) {
        Listener listener = getListener(event_id);

        std::lock_guard<std::mutex> lock(argument_mutex);
        *(listener.argument) = argument;
        listener.event->set();
    }

    std::optional<ARGUMENT_T> wait(int event_id, std::chrono::duration<int, std::milli> timeout) {
        Listener listener = getListener(event_id);

        if (!listener.event->wait(timeout))
            return std::nullopt;
        else {
            std::lock_guard<std::mutex> lock(argument_mutex);
            return  *(listener.argument);
        } 
    }

    void clear(int event_id) {
        Listener listener = getListener(event_id);

        std::lock_guard<std::mutex> lock(argument_mutex);
        *(listener.argument) = ARGUMENT_T();
        listener.event->reset();
    }

private:

    Listener getListener(int event_id) {
        std::lock_guard<std::mutex> lock(find_mutex);

        if (!listeners.count(event_id))
            listeners.insert(std::pair<int, Listener>(event_id, { std::make_shared<Event>(), std::make_shared<ARGUMENT_T>() }));

        return (listeners.find(event_id))->second;
    }

    std::map<int, Listener> listeners;

    std::mutex find_mutex, argument_mutex;
};
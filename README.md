# ThreadPool
Small Light Weight ThreadPool

[![CodeFactor](https://www.codefactor.io/repository/github/khubaibumer/threadpool/badge)](https://www.codefactor.io/repository/github/khubaibumer/threadpool)

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/4cbbe30fef1843929bae962f6eee8deb)](https://www.codacy.com/gh/khubaibumer/ThreadPool/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=khubaibumer/ThreadPool&amp;utm_campaign=Badge_Grade)

## Initialization

### 1. Set Queue Functions
> ThreadPool->set_queue_functions(new_func, init_func, push_back_func, pop_head_func)

### 2. Initialize/Destroy ThreadPool
> ThreadPool->init(thread_count)

> ThreadPool->destroy()


## ThreadPool Functions
### 1. Counters
> ThreadPool->get_total()

> ThreadPool->get_available()

> ThreadPool->get_busy()

### 2. Base Functions
> ThreadPool->get_tid() 

> ThreadPool->set_logfile(FILE*)

> ThreadPool->post(job, args)


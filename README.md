# ThreadPool
Small Light Weight ThreadPool

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


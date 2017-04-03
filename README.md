# μchan++
### Tiny go-inspired channel implementation for C++

Example:

```c++
// Compile with: g++ -o ex{,.cpp} -lpthread
#include "uchan.h"

#include <thread>
#include <iostream>

void fibonnaci(uchan<int>& jobs, uchan<int>& results)
{
    while (1)
    {
        int value;
        if (!jobs.get(value)) return;
        int v1 = 1, v2 = 1, n = 1;
        for (int i = 1; i <= value; i++)
        {
            n = v1 + v2;
            v1 = v2;
            v2 = n;
        }
        results.put(n);
    }
}

int main() 
{ 
    uchan<int> jobs;
    uchan<int> results;
    
    std::thread th(fibonnaci, std::ref(jobs), std::ref(results));
    for (int i = 1; i < 30; i++)
    {
        jobs.put(i);
        
        // Do something else ...
        
        int result;
        results.get(result);
        std::cout << i << ": " << result << std::endl;
    }
    jobs.close();
    th.join();
    
    return 0;
}
```

## Caveats/Limitations
* No select statement
* Trying to pass channel by value will result in error
* Can't have channel of channels.

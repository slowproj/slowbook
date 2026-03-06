// test-redis.cpp //

#include <iostream>
#include <slowbook/datastore_Redis.hpp>


int main(void)
{
    slowbook::Redis redis("redis://127.0.0.1/1");
    
    auto reply = redis.command("PING");
    std::cout << "PING --> " << reply << std::endl;

    return 0;
}

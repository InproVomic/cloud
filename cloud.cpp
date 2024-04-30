#include "util.hpp"
#include "config.hpp"
#include "data.hpp"
#include "hot.hpp"
#include "server.hpp"
#include <thread>

cloud::DataManager* _data;

void HotTest()
{
  cloud::HotManager hot;
  hot.RunModule();
}

void ServerTest()
{
  cloud::Server svr;
  svr.RunModule();
}

int main(int argc,char* argv[])
{ 
  _data = new cloud::DataManager();
  std::thread thread_hot_manager(HotTest);
  std::thread thread_server_manager(ServerTest);

  thread_hot_manager.join();
  thread_server_manager.join();
  return 0;
}

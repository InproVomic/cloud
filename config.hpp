#ifndef __MY_CONFIG_
#define __MY_CONFIG_
#include "util.hpp"
#include <mutex>

namespace cloud
{
  #define CONFIG_FILE "./cloud.conf"
  class Config
  {
    private:
      static Config* _instance;
      static std::mutex _mutex;
      Config()
      {
        ReadConfigFile();
      };
    public:
      static Config* GetInstance()
      {
        if(_instance == NULL)
        {
          _mutex.lock();
          if(_instance == NULL)
          {
            _instance = new Config();
          }
          _mutex.unlock();
        }
        return _instance;
      }
      std::string GetServerIp()
      {
        return server_ip;
      }
      int GetServerPort()
      {
        return server_port;
      }
      int GetHotTime()
      {
        return hot_time;
      }
      std::string GetDownloadPrefix()
      {
        return download_prefix;
      }
      std::string GetPackFileSuffix()
      {
        return packfile_suffix;
      }
      std::string GetBackDir()
      {
        return back_dir;
      }
      std::string GetPackDir()
      {
        return pack_dir;
      }
      std::string GetBackupFile()
      {
        return backup_file;
      }
    private:
      int hot_time;
      std::string server_ip;
      int server_port;
      std::string download_prefix;
      std::string packfile_suffix;
      std::string back_dir;
      std::string pack_dir;
      std::string backup_file;
      bool ReadConfigFile()
      {
        FileUtil fu(CONFIG_FILE);
        std::string body;
        if(fu.GetContent(&body)==false)
        {
          std::cout<<"load config file failed!\n";
          return false;
        }
        Json::Value root;
        if(JsonUtil::UnSerialize(&root,body)==false)
        {
          std::cout<<"parse config file failed!\n";
            return false;
        }
        hot_time = root["hot_time"].asInt();
        server_ip = root["server_ip"].asString();
        server_port = root["server_port"].asInt();
        download_prefix = root["download_prefix"].asString();
        packfile_suffix = root["packfile_suffix"].asString();
        back_dir = root["back_dir"].asString();
        pack_dir = root["pack_dir"].asString();
        backup_file = root["backup_file"].asString();
        return true;
      }
  };
  Config* Config::_instance = NULL;
  std::mutex Config::_mutex;
}

#endif

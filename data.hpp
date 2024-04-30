#ifndef __MY_DATA__
#define __MY_DATA__
#include "util.hpp"
#include "config.hpp"
#include <pthread.h>
#include <unordered_map>

namespace cloud
{
  struct BackupInfo
  {
    bool pack_flag;
    size_t fsize;
    time_t mtime;
    time_t atime;
    std::string real_path;
    std::string pack_path;
    std::string url;
    bool NewBackupInfo(const std::string& realpath)
    { 
      cloud::FileUtil fu(realpath);
      if(fu.Exists()==false)
      {
        std::cout<<"new backupinfo file is not exists!\n";
        return false;
      }
      Config* config = Config::GetInstance();
      std::string packdir = config->GetPackDir();
      std::string packsuffix = config->GetPackFileSuffix();
      std::string download_prefix = config->GetDownloadPrefix();
      pack_flag = false;
      fsize = fu.FileSize();
      mtime = fu.LastMTime();
      atime = fu.LastATime();
      real_path = realpath;
      pack_path = packdir + fu.FileName() + packsuffix;
      url = download_prefix + fu.FileName();
    }
  };

  class DataManager
  {
    private:
      std::string _backup_file;
      pthread_rwlock_t _rwlock;
      std::unordered_map<std::string,BackupInfo> _table;
    public:
      DataManager()
      {
        pthread_rwlock_init(&_rwlock,NULL);
        _backup_file = Config::GetInstance()->GetBackupFile();
        InitLoad();
      }
      ~DataManager()
      {
        pthread_rwlock_destroy(&_rwlock);
      }
      bool Insert(const BackupInfo& info)
      {
        pthread_rwlock_wrlock(&_rwlock);
        _table[info.url] = info;
        pthread_rwlock_unlock(&_rwlock);
        Storage();
        return true;
      }
      bool Update(const BackupInfo& info)
      {
        pthread_rwlock_wrlock(&_rwlock);
        _table[info.url] = info;
        pthread_rwlock_unlock(&_rwlock);
        Storage();
        return true;
      }
      bool GetOneByUrl(const std::string& url,BackupInfo* info)
      {
        pthread_rwlock_rdlock(&_rwlock);
        auto it = _table.find(url);
        if(it == _table.end())
        {
          pthread_rwlock_unlock(&_rwlock);
          return false;
        }
        *info = it->second;
        pthread_rwlock_unlock(&_rwlock);
        return true;
      }
      bool GetOneByRealPath(const std::string& realpath,BackupInfo* info)
      {
        pthread_rwlock_rdlock(&_rwlock);
        auto it = _table.begin();
        for(;it != _table.end();++it)
          if(it->second.real_path == realpath)
          {
            *info = it->second;
            pthread_rwlock_unlock(&_rwlock);
            return true;
          }
        pthread_rwlock_unlock(&_rwlock);
        return false;
      }
      bool GetAll(std::vector<BackupInfo> *arry)
      {
        pthread_rwlock_rdlock(&_rwlock);
        for(auto it = _table.begin();it != _table.end();++it)
          arry->push_back(it->second);
        pthread_rwlock_unlock(&_rwlock);
        return true;
      }
      bool Storage()
      {
        //读取所有数据
        std::vector<BackupInfo> arry;
        GetAll(&arry);
        //添加到Json::Value
        Json::Value root;
        for(int i = 0;i < arry.size();++i)
        {
          Json::Value tmp;
          tmp["pack_flag"] = arry[i].pack_flag;
          tmp["fsize"] = (Json::Int64)arry[i].fsize;
          tmp["mtime"] = (Json::Int64)arry[i].mtime;
          tmp["atime"] = (Json::Int64)arry[i].atime;
          tmp["real_path"] = arry[i].real_path;
          tmp["pack_path"] = arry[i].pack_path;
          tmp["url"] = arry[i].url;
          root.append(tmp);
        }
        //对Json序列化
        std::string body;
        JsonUtil::Serialize(root,&body);
        //写入文件
        FileUtil fu(_backup_file);
        fu.SetContent(body);
        return true;
      }
      bool InitLoad()
      {
        //1.把文件里的数据读取出来
        FileUtil fu(_backup_file);
        if(fu.Exists()==false)
          return false;
        std::string body;
        fu.GetContent(&body);
        //2.反序列化
        Json::Value root;
        JsonUtil::UnSerialize(&root,body);
        //3.将反序列化后的数据添加到table中
        for(int i = 0;i < root.size();++i)
        {
          BackupInfo info;
          info.fsize = root[i]["fsize"].asInt64();
          info.atime = root[i]["atime"].asInt64();
          info.mtime = root[i]["mtime"].asInt64();
          info.pack_flag = root[i]["pack_flag"].asBool();
          info.pack_path = root[i]["pack_path"].asString();
          info.real_path = root[i]["real_path"].asString();
          info.url = root[i]["url"].asString();
          Insert(info);
        }
        return true;
      }
  };
}
#endif

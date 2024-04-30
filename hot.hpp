#ifndef __MY_HOT__
#define __MY_HOT__
#include "data.hpp"
#include <unistd.h>
extern cloud::DataManager* _data;

namespace cloud
{
  class HotManager
  {
    private:
      std::string _pack_dir;
      std::string _back_dir;
      std::string _pack_suffix;
      int _hot_time;
    public:
      HotManager()
      {
        Config* config = Config::GetInstance();
        _pack_dir = config->GetPackDir();
        _back_dir = config->GetBackDir();
        _pack_suffix = config->GetPackFileSuffix();
        _hot_time = config->GetHotTime();
        FileUtil tmp1(_pack_dir);
        FileUtil tmp2(_back_dir);
        tmp1.CreateDirectory();
        tmp2.CreateDirectory();
      }
      bool HotJudge(const std::string& file)
      {
        FileUtil fu(file);
        time_t last_atime = fu.LastATime();
        time_t current_time = time(NULL);
        if(current_time - last_atime >= _hot_time)
          return true;
        return false;
      }
      bool RunModule()
      {
        while(1)
        {
          //1.遍历文件目录，获取文件信息
          std::vector<std::string> arry;
          FileUtil fu(_back_dir);
          fu.ScanDirectory(&arry);
          //2.判断文件是否是热点文件
          for(auto& e : arry)
          {
            if(HotJudge(e)==false)
              continue;
            //3.获取备份信息并对非热点文件进行压缩处理
            BackupInfo info;
            if(_data->GetOneByRealPath(e,&info)==false)
            {
              info.NewBackupInfo(e);
            }
            FileUtil fu(e);
            //3.将非热点文件压缩处理后保存到_pack_dir中    
            fu.Compress(info.pack_path);
            //4.删除非热点文件源文件，并更新文件信息
            fu.Remove();
            info.pack_flag=true;
            _data->Update(info);
          }
          usleep(1000);
        }

       return true; 
      }
  };
}

#endif

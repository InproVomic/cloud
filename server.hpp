#ifndef __MY_SERVER__
#define __MY_SERVER__
#include "httplib.h"
#include "data.hpp"

namespace cloud
{
  class Server
  {
    private:
      std::string _server_ip;
      int _server_port;
      std::string _download_prefix;
      httplib::Server _server;
    private:
      static void Upload(const httplib::Request& req,httplib::Response &rsp)
      {
        auto ret = req.has_file("file");
        if(ret == false)
        {
          rsp.status = 400;
          return;
        }
        const auto& file = req.get_file_value("file");
        std::string back_dir = Config::GetInstance()->GetBackDir();
        std::string real_path = back_dir + FileUtil(file.filename).FileName();
        FileUtil fu(real_path);
        fu.SetContent(file.content);
        BackupInfo info;
        info.NewBackupInfo(real_path);
        _data->Insert(info);
      }
      static std::string TimetoStr(const time_t& t)
      {
        return std::string(std::ctime(&t));
      }
      static void ListShow(const httplib::Request& req,httplib::Response &rsp)
      {
        //1.获取文件备份信息
        std::vector<BackupInfo> arry;
        _data->GetAll(&arry);
        //2.根据备份信息，组织html文件
        std::stringstream ss;
        ss << "<html><head><title>Download</title></head>";
        ss << "<body><h1>Download</h1><table>";
        for(auto& a: arry)
        {
          ss << "<tr>";
          std::string filename = FileUtil(a.real_path).FileName();
          ss << "<td><a href='" << a.url <<"'>" <<filename<< "</a></td>";
          ss << "<td align='right'>" << TimetoStr(a.mtime)<<"</td>";
          ss << "<td align='right'>" << a.fsize / 1024 << "k</td>";
          ss << "</tr>";
        }
        ss << "</table></body></html>";
        rsp.body = ss.str();
        rsp.set_header("Content-Type", "text/html");
        rsp.status = 200;
      }
      static std::string GetETag(const BackupInfo& info)
      {
        // etage : filename-fsize-mtime
        FileUtil fu(info.real_path);
        std::string etag = fu.FileName();
        etag += "-";
        etag += std::to_string(info.fsize);
        etag += "-";
        etag += std::to_string(info.mtime);
        return etag;
      }
      static void Download(const httplib::Request& req,httplib::Response &rsp)
      {
        //1.获得客户端请求的资源路径path   req.path
        BackupInfo info;
        //2.根据资源路径，获取文件备份信息
        _data->GetOneByUrl(req.path,&info);
        //3.判断文件是否被压缩，如果被压缩先解压到备份目录
        if(info.pack_flag==true)
        {
          FileUtil fu(info.pack_path);
          fu.UnCompress(info.real_path);
          //4.删除压缩包，更新文件备份信息
          fu.Remove();
          info.pack_flag = false;
          _data->Update(info);
        }

        bool retrans = false;
        std::string old_etag;
        if(req.has_header("If-Range"))
        {
          old_etag = req.get_header_value("If-Range");
          std::cout<<"old: "<<old_etag<<std::endl;
          std::cout<<"new: "<<GetETag(info)<<std::endl;
          if(old_etag == GetETag(info))
          {
            retrans = true;
          }
        }

        //4.读取文件数据，放入rsp.body中
        FileUtil fu(info.real_path);
        fu.GetContent(&rsp.body);
        //5.设置响应头部字段 ETage 和Accept-Ranges: bytes
        if(retrans == false)
        {
          rsp.set_header("Accept-Ranges", "bytes");
          rsp.set_header("ETag", GetETag(info));
          rsp.set_header("Content-Type", "application/octet-stream");
          rsp.status = 200;
        }
        else
        {
          rsp.set_header("Accept-Ranges", "bytes");
          rsp.set_header("ETag", GetETag(info));
          rsp.set_header("Content-Type", "application/octet-stream");
          rsp.status = 206;
        }
      }
    public:
      Server()
      {
        Config* config = Config::GetInstance();
        _server_ip = config->GetServerIp();
        _server_port = config->GetServerPort();
        _download_prefix = config->GetDownloadPrefix();
      }
      bool RunModule()
      {
        _server.Post("/upload",Upload);
        _server.Get("/listshow",ListShow);
        _server.Get("/",ListShow);
        std::string download_url = _download_prefix + "(.*)";
        _server.Get(download_url,Download);
        _server.listen("0.0.0.0",_server_port);
        return true;
      }
  };
}


#endif

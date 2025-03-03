#ifndef __MY_UTIL__
#define __MY_UTIL__
#include <iostream>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include "bundle.h"
#include <string>
#include <memory>
#include <jsoncpp/json/json.h>
#include <experimental/filesystem>

namespace cloud
{
  namespace fs = std::experimental::filesystem;
  class FileUtil//工具类
  {
    private:
      std::string _filename;//文件名(也可能是文件地址)
    public:
      FileUtil(const std::string& filename):_filename(filename){}

      int64_t FileSize()//返回文件大小
      {
        struct stat st;
        if(stat(_filename.c_str(), &st)<0)
        {
          std::cout<<"get file size failed!\n";
          return -1;
        }
        return st.st_size;
      }
      time_t LastMTime()//返回最后一次修改时间
      {  
        struct stat st;
        if(stat(_filename.c_str(), &st)<0)
        {
          std::cout<<"get file MTime failed!\n";
          return -1;
        }
        return st.st_mtime;
      }
      time_t LastATime()//返回最后访问时间
      {
        struct stat st;
        if(stat(_filename.c_str(), &st)<0)
        {
          std::cout<<"get file MTime failed!\n";
          return -1;
        }
        return st.st_atime;
      }
      std::string FileName()//返回文件名
      {
        int pos = _filename.find_last_of('/');
        if(pos == std::string::npos)
          return _filename;
        return _filename.substr(pos+1);
      }
      bool GetPosLen(std::string *body, size_t pos, size_t len)
      {
        size_t fsize = this->FileSize();
        if(pos+len > fsize)
        {
          std::cout<<"get file len error\n";
          return false;
        }
        std::ifstream ifs;
        ifs.open(_filename,std::ios::binary);
        if(ifs.is_open()==false)
        {
          std::cout<<"read open file failed\n";
          return false;
        }
        ifs.seekg(pos, std::ios::beg);
        body->resize(len);
        ifs.read(&(*body)[0],len);
        if(ifs.good()==false)
        {
          std::cout<<"get file content failed\n";
          ifs.close();
          return false;
        }
        ifs.close();
        return true;
      }
      bool GetContent(std::string* body)
      {
        size_t fsize = this->FileSize();
        return this->GetPosLen(body,0,fsize);
      }
      bool SetContent(const std::string &body)
      {
        std::ofstream ofs;
        ofs.open(_filename,std::ios::binary);
        if(ofs.is_open()==false)
        {
          std::cout<<"write open file failed\n";
          return false;
        }
        ofs.write(&body[0],body.size());
        if(ofs.good()==false)
        {
          std::cout<<"write body file failed\n";
          ofs.close();
          return false;
        }
        ofs.close();
        return true;
      }
      bool Compress(const std::string& packname)
      {
        //先读取文件数据
        std::string body;
        if(this->GetContent(&body)==false)
        {
          std::cout<<"compress get file content failed!\n";
          return false;
        }
        //再压缩数据
        std::string packed = bundle::pack(bundle::LZIP, body);
        //将压缩后的数据写入文件中
        FileUtil fu(packname);
        if(fu.SetContent(packed)==false)
        {
          std::cout<<"compress write compress data failed!\n";
          return false;
        }
        return true;
      }
      bool UnCompress(const std::string& filename)
      {
        std::string body;
        //先读取数据
        if(this->GetContent(&body)==false)
        {
          std::cout<<"uncompress get file content failed!\n";
          return false;
        }
        //解压数据
        std::string unpacked = bundle::unpack(body);
        //将解压后的数据写入文件中
        FileUtil fu(filename);
        if(fu.SetContent(unpacked)==false)
        {
          std::cout<<"uncompress write uncompress data failed!\n";
          return false;
        }
        return true;
      }
      bool Exists()
      {
        return fs::exists(_filename);
      }
      bool CreateDirectory()
      {
        return fs::create_directories(_filename);
      }
      bool ScanDirectory(std::vector<std::string> *arry)
      {
        for(auto& p : fs::directory_iterator(_filename))
        {
          if(fs::is_directory(p)==true)
            continue;
          //relative_path是带有路径的文件名
          arry->push_back(fs::path(p).relative_path().string());
        }
        return true;
      }
      bool Remove()
      {
        if(this->Exists()==false)
          return true;
        remove(_filename.c_str());
        return true;
      }
  };
  class JsonUtil
  {
  public:
    static bool Serialize(const Json::Value &root, std::string *str)
    {
      Json::StreamWriterBuilder swb;
      std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
      std::stringstream ss;
      sw->write(root,&ss);
      *str = ss.str();
      return true;
    }
    static bool UnSerialize(Json::Value *root,const std::string& str)
    {
      Json::CharReaderBuilder crb;
      std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
      std::string err;
      bool ret = cr->parse(str.c_str(),str.c_str()+str.size(),root,&err);
      if(ret == false)
      {
        std::cout<<"parse error\n";
        return false;
      }
      return true;
    }
  };
}

#endif

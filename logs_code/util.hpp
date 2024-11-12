// 工具：
// 1. 获取系统时间
// 2. 判断文件是否存在
// 3. 获取文件所在目录
// 4. 创建目录
// 5. 获取配置文件内容
#pragma once
#include <sys/stat.h>
#include <sys/types.h>
#include <jsoncpp/json/json.h>

#include <ctime>
#include <fstream>
#include <iostream>
using std::cout;
using std::endl;
namespace xqlog
{
    namespace Util
    {
        class Date
        {
        public:
            static time_t Now() { return time(nullptr); }
        };
        class File
        {
        public:
            static bool Exists(const std::string &filename)
            {
                // return (0==access(filename.c_str(),F_OK)); 不具有可移植性，仅在linux上可以跑 \
        采用POSIX提供的stat
                struct stat st;
                return (0 == stat(filename.c_str(), &st));
            }
            static std::string Path(const std::string &filename)
            {
                if (filename.empty())
                    return "";
                int pos = filename.find_last_of("/\\");
                if (pos != std::string::npos)
                    return filename.substr(0, pos + 1);
                return "";
            }
            static void CreateDirectory(const std::string &pathname)
            {
                if (pathname.empty())
                    perror("文件所给路径为空：");
                // 文件不存在再创建
                if (!Exists(pathname))
                {
                    size_t pos, index = 0;
                    size_t size = pathname.size();
                    while (index < size)
                    {
                        pos = pathname.find_first_of("/\\", index);
                        if (pos == std::string::npos)
                        {
                            mkdir(pathname.c_str(), 0755);
                            return;
                        }
                        if (pos == index)
                        {
                            index = pos + 1;
                            continue;
                        }

                        std::string sub_path = pathname.substr(0, pos);
                        if (sub_path == "." || sub_path == "..")
                        {
                            index = pos + 1;
                            continue;
                        }
                        if (Exists(sub_path))
                        {
                            index = pos + 1;
                            continue;
                        }

                        mkdir(sub_path.c_str(), 0755);
                        index = pos + 1;
                    }
                }
            }

            int64_t FileSize(std::string filename)
            {
                struct stat s;
                auto ret = stat(filename.c_str(), &s);
                if (ret == -1)
                {
                    perror("Get file size failed");
                    return -1;
                }
                return s.st_size;
            }
            // 获取文件内容
            bool GetContent(std::string *content, std::string filename)
            {
                // 打开文件
                std::ifstream ifs;
                ifs.open(filename.c_str(), std::ios::binary);
                if (ifs.is_open() == false)
                {
                    std::cout << "file open error" << std::endl;
                    return false;
                }

                // 读入content
                ifs.seekg(0, std::ios::beg); // 更改文件指针的偏移量
                size_t len = FileSize(filename);
                content->resize(len);
                ifs.read(&(*content)[0], len);
                if (!ifs.good())
                {
                    std::cout << __FILE__ << __LINE__ << "-"
                              << "read file content error" << std::endl;
                    ifs.close();
                    return false;
                }
                ifs.close();

                return true;
            }
        }; // class file
        class JsonUtil
        {
        public:
            static bool Serialize(const Json::Value &val, std::string *str)
            {
                // 建造者生成->建造者实例化json写对象->调用写对象中的接口进行序列化写入str
                Json::StreamWriterBuilder swb;
                std::unique_ptr<Json::StreamWriter> usw(swb.newStreamWriter());
                std::stringstream ss;
                if (usw->write(val, &ss) != 0)
                {
                    std::cout << "serialize error" << std::endl;
                    return false;
                }
                *str = ss.str();
                return true;
            }
            static bool UnSerialize(const std::string &str, Json::Value *val)
            {
                // 操作方法类似序列化
                Json::CharReaderBuilder crb;
                std::unique_ptr<Json::CharReader> ucr(crb.newCharReader());
                std::string err;
                if (ucr->parse(str.c_str(), str.c_str() + str.size(), val, &err) == false)
                {
                    std::cout <<__FILE__<<__LINE__<<"parse error" << err<<std::endl;
                    return false;
                }
                return false;
            }
        };
    } // namespace Util
} // namespace xqlog
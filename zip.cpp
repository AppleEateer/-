#include <iostream>
#include <fstream>
#include <vector>
#include <zlib.h>

const int BUFFER_SIZE = 16384;  // 每次读取和写入的缓冲区大小

// 文件压缩函数
bool compressFile(const std::string& inputFilePath, const std::string& outputFilePath) 
{
    std::ifstream inputFile(inputFilePath, std::ios_base::binary);  // 打开输入文件，以二进制模式读取
    if (!inputFile.is_open())  // 检查输入文件是否成功打开
    {  
        std::cerr << "Error: 无法打开输入文件 " << inputFilePath << "\n";
        return false;
    }

    std::ofstream outputFile(outputFilePath, std::ios_base::binary);  // 打开输出文件，以二进制模式写入
    if (!outputFile.is_open()) // 检查输出文件是否成功打开
    {  
        std::cerr << "Error: 无法打开输出文件 " << outputFilePath << "\n";
        return false;
    }

    std::vector<char> inputBuffer(BUFFER_SIZE);  // 输入缓冲区
    std::vector<char> outputBuffer(BUFFER_SIZE);  // 输出缓冲区

    z_stream compressionStream = {0};             // 初始化zlib流结构
    deflateInit(&compressionStream, Z_DEFAULT_COMPRESSION);  // 初始化压缩流，使用默认压缩级别

    int flush;
    do {
        inputFile.read(inputBuffer.data(), inputBuffer.size());  // 从输入文件读取数据到缓冲区
        compressionStream.avail_in = inputFile.gcount();  // 设置流的可用输入字节数
        flush = inputFile.eof() ? Z_FINISH : Z_NO_FLUSH;  // 检查是否为文件结尾，是则设置为Z_FINISH
        compressionStream.next_in = reinterpret_cast<Bytef*>(inputBuffer.data());  // 设置流的输入数据

        do {
            compressionStream.avail_out = outputBuffer.size();  // 设置流的可用输出字节数
            compressionStream.next_out = reinterpret_cast<Bytef*>(outputBuffer.data());  // 设置流的输出数据
            deflate(&compressionStream, flush);  // 执行压缩操作
            std::streamsize bytesCompressed = outputBuffer.size() - compressionStream.avail_out;  // 计算压缩后的数据量
            outputFile.write(outputBuffer.data(), bytesCompressed);  // 将压缩后的数据写入输出文件
        } while (compressionStream.avail_out == 0);  // 如果输出缓冲区满了，继续压缩
    } while (flush != Z_FINISH);  // 如果还没有读完文件，继续循环

    deflateEnd(&compressionStream);  // 结束压缩流
    inputFile.close();  // 关闭输入文件
    outputFile.close();  // 关闭输出文件
    return true;  // 返回成功
}

// 文件解压缩函数
bool decompressFile(const std::string& inputFilePath, const std::string& outputFilePath) 
{
    std::ifstream inputFile(inputFilePath, std::ios_base::binary);  // 打开输入文件，以二进制模式读取
    if (!inputFile.is_open()) // 检查输入文件是否成功打开
    {  
        std::cerr << "Error: 无法打开输入文件 " << inputFilePath << "\n";
        return false;
    }

    std::ofstream outputFile(outputFilePath, std::ios_base::binary);  // 打开输出文件，以二进制模式写入
    if (!outputFile.is_open()) /// 检查文件是否成功打开
    {  
        std::cerr << "Error: 无法打开输出文件 " << outputFilePath << "\n";
        return false;
    }

    std::vector<char> inputBuffer(BUFFER_SIZE);  // 输入缓冲区
    std::vector<char> outputBuffer(BUFFER_SIZE);  // 输出缓冲区

    z_stream decompressionStream = {0};  // 初始化zlib流结构
    inflateInit(&decompressionStream);  // 初始化解压缩流

    int result;
    do {
        inputFile.read(inputBuffer.data(), inputBuffer.size());  // 从输入文件读取数据到缓冲区
        decompressionStream.avail_in = inputFile.gcount();  // 设置流的可用输入字节数
        if (decompressionStream.avail_in == 0)  // 如果没有可用的输入数据，则结束
            break;
        decompressionStream.next_in = reinterpret_cast<Bytef*>(inputBuffer.data());  // 设置流的输入数据

        do {
            decompressionStream.avail_out = outputBuffer.size();  // 设置流的可用输出字节数
            decompressionStream.next_out = reinterpret_cast<Bytef*>(outputBuffer.data());  // 设置流的输出数据
            result = inflate(&decompressionStream, Z_NO_FLUSH);  // 执行解压缩操作
            switch (result) // 处理解压缩的返回值
            {  
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&decompressionStream);
                    return false;
            }
            std::streamsize bytesDecompressed = outputBuffer.size() - decompressionStream.avail_out;  // 计算解压缩后的数据量
            outputFile.write(outputBuffer.data(), bytesDecompressed);  // 将解压缩后的数据写入输出文件
        } while (decompressionStream.avail_out == 0);  // 如果输出缓冲区满了，继续解压缩
    } while (result != Z_STREAM_END);  // 如果还没有读完文件，继续循环

    inflateEnd(&decompressionStream);  // 结束解压缩流
    inputFile.close();  // 关闭输入文件
    outputFile.close();  // 关闭输出文件
    return result == Z_STREAM_END;  // 返回是否成功解压缩到流的结尾
}

// 主函数
int main(int argc, char* argv[]) 
{
    if (argc != 4) // 检查命令行参数是否正确
    {  
        std::cerr << "Usage: " << argv[0] << " <compress|decompress> <source> <destination>\n";
        return 1;
    }

    std::string operation = argv[1];  // 获取操作类型（压缩或解压缩）
    std::string source = argv[2];  // 获取源文件路径
    std::string destination = argv[3];  // 获取目标文件路径

    if (operation == "compress") // 如果操作是压缩
    {  
        if (compressFile(source, destination)) // 调用压缩函数
        {  
            std::cout << "文件压缩成功。\n";
        } 
        else 
        {
            std::cerr << "文件压缩失败。\n";
        }
    } else if (operation == "decompress") // 如果操作是解压缩
    {  
        if (decompressFile(source, destination)) // 调用解压缩函数
        {  
            std::cout << "文件解压缩成功。\n";
        } 
        else 
        {
            std::cerr << "文件解压缩失败。\n";
        }
    } 
    else 
    {
        std::cerr << "无效的操作。使用 'compress' 或 'decompress'。\n";
        return 1;
    }

    return 0;
}

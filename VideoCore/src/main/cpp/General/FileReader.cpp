//
// Created by geier on 07/02/2020.
//

#include "FileReader.h"
#include "../NALU/NALU.hpp"
#include <iterator>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fstream>

static bool endsWith(const std::string& str, const std::string& suffix){
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}
static const constexpr auto TAG="FileReader";
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

void FileReader::passDataInChunks(const std::vector<uint8_t> &data) {
    int offset=0;
    const int len=(int)data.size();
    while(receiving){
        const int len_left=len-offset;
        if(len_left>=CHUNK_SIZE){
            nReceivedB+=CHUNK_SIZE;
            onDataReceivedCallback(&data[offset],CHUNK_SIZE);
            offset+=CHUNK_SIZE;
        }else{
            nReceivedB+=len_left;
            onDataReceivedCallback(&data[offset],len_left);
            return;
        }
    }
}

std::vector<uint8_t>
FileReader::loadAssetAsRawVideoStream(AAssetManager *assetManager, const std::string &path) {
    if(endsWith(path,".mp4")){
        //Use MediaExtractor to parse .mp4 file
        AAsset* asset = AAssetManager_open(assetManager,path.c_str(), 0);
        off_t start, length;
        auto fd=AAsset_openFileDescriptor(asset,&start,&length);
        if(fd<0){
            LOGD("ERROR AAsset_openFileDescriptor %d",fd);
            return std::vector<uint8_t>();
        }
        AMediaExtractor* extractor=AMediaExtractor_new();
        AMediaExtractor_setDataSourceFd(extractor,fd,start,length);
        const auto trackCount=AMediaExtractor_getTrackCount(extractor);
        //This will save all data as RAW
        //SPS/PPS in the beginning, rest afterwards
        std::vector<uint8_t> rawData;
        for(size_t i=0;i<trackCount;i++){
            AMediaFormat* format= AMediaExtractor_getTrackFormat(extractor,i);
            const char* s;
            AMediaFormat_getString(format,AMEDIAFORMAT_KEY_MIME,&s);
            LOGD("Track is %s",s);
            if(std::string(s).compare("video/avc")==0){
                const auto mediaStatus=AMediaExtractor_selectTrack(extractor,i);
                auto tmp=getBufferFromMediaFormat("csd-0",format);
                rawData.insert(rawData.end(),tmp.begin(),tmp.end());
                tmp=getBufferFromMediaFormat("csd-1",format);
                rawData.insert(rawData.end(),tmp.begin(),tmp.end());
                LOGD("Video track found %d %s",mediaStatus, AMediaFormat_toString(format));
                break;
            }
            AMediaFormat_delete(format);
        }
        std::vector<uint8_t> tmpBuffer(1024*1024);
        while(true){
            const auto sampleSize=AMediaExtractor_readSampleData(extractor,tmpBuffer.data(),tmpBuffer.size());
            const auto flags=AMediaExtractor_getSampleFlags(extractor);
            LOGD("Read sample %d flags %d",sampleSize,flags);
            const NALU nalu(tmpBuffer.data(),sampleSize);
            LOGD("NALU %s",nalu.get_nal_name().c_str());
            if(sampleSize<0){
                break;
            }
            rawData.insert(rawData.end(),tmpBuffer.begin(),tmpBuffer.begin()+sampleSize);
            AMediaExtractor_advance(extractor);
        }
        AMediaExtractor_delete(extractor);
        return rawData;
    }else if(endsWith(path,".h264")){
        //Read raw data from file (without MediaExtractor)
        AAsset* asset = AAssetManager_open(assetManager, path.c_str(), 0);
        if(!asset){
            LOGD("Error asset not found:%s",path.c_str());
        }else{
            const size_t size=(size_t)AAsset_getLength(asset);
            AAsset_seek(asset,0,SEEK_SET);
            std::vector<uint8_t> rawData(size);
            int len=AAsset_read(asset,rawData.data(),size);
            AAsset_close(asset);
            LOGD("The entire file content (asset) is in memory %d",(int)size);
            return rawData;
        }
    }
    LOGD("Error not supported file %s",path.c_str());
    return std::vector<uint8_t>();
}

std::vector<uint8_t> FileReader::getBufferFromMediaFormat(const char *name, AMediaFormat *format) {
    uint8_t* data;
    size_t data_size;
    AMediaFormat_getBuffer(format,name,(void**)&data,&data_size);
    std::vector<uint8_t> ret(data_size);
    memcpy(ret.data(),data,data_size);
    return ret;
}

void FileReader::receiveLoop() {
    nReceivedB=0;
    if(assetManager!= nullptr){
        //from Assets we support both raw and mp4
        const auto data=loadAssetAsRawVideoStream(assetManager,filename);
        passDataInChunks(data);
    }else{
        //from file we only support mp4
        parseMP4FileAsRawVideoStream(filename);
    }
}

//Helper, return size of file in bytes
ssize_t fsize(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1;
}

void FileReader::parseMP4FileAsRawVideoStream(const std::string &filename) {
    const auto fileSize=fsize(filename.c_str());
    if(fileSize<=0){
        LOGD("Error file size %d",(int)fileSize);
        return;
    }
    const int fd = open(filename.c_str(), O_RDONLY, 0666);
    if(fd<0){
        LOGD("Error open file: %s fp: %d",filename.c_str(),fd);
        return;
    }
    AMediaExtractor* extractor=AMediaExtractor_new();
    auto mediaStatus=AMediaExtractor_setDataSourceFd(extractor,fd,0,fileSize);
    if(mediaStatus!=AMEDIA_OK){
        LOGD("Error open File %s,mediaStatus: %d",filename.c_str(),mediaStatus);
        AMediaExtractor_delete(extractor);
        close(fd);
        return;
    }
    const auto trackCount=AMediaExtractor_getTrackCount(extractor);
    bool videoTrackFound=false;
    std::vector<uint8_t> csd0;
    std::vector<uint8_t> csd1;
    for(size_t i=0;i<trackCount;i++){
        AMediaFormat* format= AMediaExtractor_getTrackFormat(extractor,i);
        const char* s;
        AMediaFormat_getString(format,AMEDIAFORMAT_KEY_MIME,&s);
        LOGD("Track is %s",s);
        if(std::string(s).compare("video/avc")==0){
            mediaStatus=AMediaExtractor_selectTrack(extractor,i);
            csd0=getBufferFromMediaFormat("csd-0",format);
            csd1=getBufferFromMediaFormat("csd-1",format);
            LOGD("Video track found %d %s",mediaStatus, AMediaFormat_toString(format));
            AMediaFormat_delete(format);
            videoTrackFound=true;
            break;
        }
    }
    if(!videoTrackFound){
        LOGD("Cannot find video track");
        AMediaExtractor_delete(extractor);
        close(fd);
    }
    //All good, feed configuration, then load & feed data one by one
    passDataInChunks(csd0);
    passDataInChunks(csd1);
    //Loop until done
    std::vector<uint8_t> buffer(NALU::NALU_MAXLEN);
    while(receiving){
        buffer.resize(buffer.capacity());
        const auto sampleSize=AMediaExtractor_readSampleData(extractor,buffer.data(),buffer.size());
        if(sampleSize<0){
            break;
        }
        buffer.resize((unsigned long)sampleSize);
        const auto flags=AMediaExtractor_getSampleFlags(extractor);
        passDataInChunks(buffer);
        AMediaExtractor_advance(extractor);
    }
    AMediaExtractor_delete(extractor);
    close(fd);
}

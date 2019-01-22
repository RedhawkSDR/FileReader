/*
 * MetaDataParser.h
 *
 *  Created on: Sep 12, 2018
 *      Author: gbfaiks
 */

#ifndef METADATAPARSER_H_
#define METADATAPARSER_H_

#include <expat.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <bulkio/bulkio.h>
#include "Queue.hpp"

struct attribute {
    std::string name;
    std::string value;
};

struct Element{
    std::string name;
    std::queue<attribute> attributes;
};

struct PacketData {
    std::string streamID;
    long dataLength;
    bool EOS;
    BULKIO::PrecisionUTCTime tstamp;
};

class MetaDataParser {
    ENABLE_LOGGING
    typedef threadsafe::Queue<size_t> ts_queue_size_t;

public:
    MetaDataParser(bulkio::InLongPort *metadataQueuePtr, ts_queue_size_t *packetSizeQueuePtr);
    ~MetaDataParser();
    void parseData(std::vector<char> xmldata);
    void startElement(const XML_Char *name, const XML_Char **atts);
    void endElement(const XML_Char *name);
    void charData(std::string data);
private:
    void reset();
    void resetParser();
    void resetPacket();
    void resetSRI();
    bulkio::InLongPort *metadataQueue;
    ts_queue_size_t *packetSizeQueue;

    std::vector<std::string> text;
    std::vector<Element> elementStack;
    BULKIO::StreamSRI currentSri;
    unsigned int keywordCount;
    bool initialSri;
    PacketData currentPacket;
    XML_Parser parser;

};

#endif /* METADATAPARSER_H_ */

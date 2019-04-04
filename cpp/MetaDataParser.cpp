/*
 * MetaDataParser.cpp
 */

#include "MetaDataParser.h"
PREPARE_LOGGING(MetaDataParser)

void startElementWrapper(void *userData, const XML_Char *name, const XML_Char **atts) {
    ((MetaDataParser*) userData)->startElement(name, atts);
}

void endElementWrapper(void *userData, const XML_Char *name) {
    ((MetaDataParser*) userData)->endElement(name);
}

void charDataWrapper(void *userData,const XML_Char *chardata,int len) {
    std::string data(chardata,len);
    ((MetaDataParser*) userData)->charData(data);
}

MetaDataParser::MetaDataParser(bulkio::InLongPort *metadataQueuePtr, ts_queue_size_t *packetSizeQueuePtr) :
        metadataQueue(metadataQueuePtr), packetSizeQueue(packetSizeQueuePtr), keywordCount(0), initialSri(true) {
    parser = XML_ParserCreate(NULL);
    resetPacket();
    resetSRI();
}

void MetaDataParser::reset() {
    resetParser();
    resetPacket();
    resetSRI();
    keywordCount = 0;
    initialSri = true;
}

void MetaDataParser::resetParser() {
    XML_ParserReset(parser, NULL);
    XML_SetUserData(parser, this);
    XML_SetElementHandler(parser,startElementWrapper, endElementWrapper);
    XML_SetCharacterDataHandler(parser,charDataWrapper);
}

void MetaDataParser::resetPacket() {
    currentPacket.streamID.clear();
    currentPacket.dataLength = 0;
    currentPacket.EOS = false;
    currentPacket.tstamp.tfsec = 0;
    currentPacket.tstamp.twsec = 0;
}

void MetaDataParser::resetSRI() {
    currentSri.streamID="";
    currentSri.blocking = true;
    currentSri.hversion=1;
    currentSri.subsize=0;
    currentSri.xstart=0.0;
    currentSri.xdelta=0.0;
    currentSri.xunits=1;
    currentSri.ystart=0.0;
    currentSri.ydelta=0.0;
    currentSri.yunits=1;
    currentSri.mode=0;
    currentSri.keywords.length(0);
}

void MetaDataParser::startElement(const XML_Char *name, const XML_Char **atts) {
    Element currentElement;
    currentElement.name = name;
    LOG_TRACE(MetaDataParser,"MetaDataParser::startElement - "<<currentElement.name);
    //Get attributes. Attributes are each two strings, one for name and one for value. The list is null terminated.
    unsigned int i = 0;
    while (atts[i]!=0) {
        attribute currentAttr;
        currentAttr.name=atts[i];
        currentAttr.value=atts[i+1];
        currentElement.attributes.push(currentAttr);
        i+=2;
    }

    elementStack.push_back(currentElement);

}
void MetaDataParser::charData(std::string data) {
    text.push_back(data);
}

void MetaDataParser::endElement(const XML_Char *name) {
    Element currentElement;
    Element parentElement;
    currentElement = elementStack.back();
    elementStack.pop_back();
    LOG_TRACE(MetaDataParser,"MetaDataParser::endElement - "<<currentElement.name);
    if (currentElement.name !=name) {
        LOG_ERROR(MetaDataParser,"currentElement.name != name");
    }

    if (currentElement.name=="FileWriter_metadata") {
        // done
        keywordCount = 0;
        resetPacket();
        resetSRI();
        text.clear();
        elementStack.clear();
    } else if (currentElement.name=="sri") {
        bool sriChanged = initialSri;
        while (!sriChanged && !currentElement.attributes.empty()){
            if (currentElement.attributes.front().name == "new") {
                if (currentElement.attributes.front().value=="TRUE" || currentElement.attributes.front().value=="True" || currentElement.attributes.front().value=="true") {
                    sriChanged = true;
                } else {
                    sriChanged = false;
                }
                break;
            }
            currentElement.attributes.pop();
        }
        if (sriChanged) {
            metadataQueue->pushSRI(currentSri);
            initialSri = false;
        }
        resetSRI();
    } else if (currentElement.name=="packet") {
        PortTypes::LongSequence data_length;
        data_length.length(1);
        data_length[0] = currentPacket.dataLength;
        metadataQueue->pushPacket(data_length,currentPacket.tstamp,currentPacket.EOS, currentPacket.streamID.c_str());
        packetSizeQueue->push(currentPacket.dataLength);
        if (currentPacket.dataLength == 0 && !currentPacket.EOS) {
            LOG_DEBUG(MetaDataParser, "MetaDataParser::endElement - added empty packet that is NOT an EOS packet ("<<currentPacket.streamID<<"|ws:"<<currentPacket.tstamp.twsec<<"|fs:"<<currentPacket.tstamp.tfsec<<"|PSQ:"<<packetSizeQueue->getUsage()<<"|MQ:"<<metadataQueue->getCurrentQueueDepth()<<")");
        } else if (currentPacket.dataLength < 0) {
            LOG_ERROR(MetaDataParser, "MetaDataParser::endElement - added negative-sized packet ("<<currentPacket.streamID<<"|ws:"<<currentPacket.tstamp.twsec<<"|fs:"<<currentPacket.tstamp.tfsec<<"|PSQ:"<<packetSizeQueue->getUsage()<<"|MQ:"<<metadataQueue->getCurrentQueueDepth()<<")");
        }
        resetPacket();
    } else {

        // If the element has a parent, get it.
        if (elementStack.size()>0) {
            parentElement = elementStack.back();
        }

        // If there is text stored, get it.
        std::string lastText = "";
        if (!text.empty()) {
            lastText = text.back();
            text.pop_back();
        }

        // Look at the parent and the element to determine what to do

        if (parentElement.name=="sri") {
            // Parent is sri, element is either keyword, or an individual sri element
            if (currentElement.name=="keyword") {
                // keyword elements have 2 attributes (id,type) and a value
                // value is stored in 'lastText'
                // add it to the currentSri.keywords list

                keywordCount++;

                size_t cl  =  currentSri.keywords.length();
                currentSri.keywords.length(cl+1);

                CORBA::TCKind keywordKind= CORBA::tk_null;
                std::string keywordID;
                while (!currentElement.attributes.empty()){
                    if (currentElement.attributes.front().name == "type") {
                        keywordKind = static_cast<CORBA::TCKind>(std::atoi(currentElement.attributes.front().value.c_str()));
                    } else if (currentElement.attributes.front().name == "id") {
                        keywordID = currentElement.attributes.front().value;
                    }
                    currentElement.attributes.pop();
                }

                currentSri.keywords[cl].id = CORBA::string_dup(keywordID.c_str());
                if (keywordKind==CORBA::tk_short) {
                    currentSri.keywords[cl].value <<= CORBA::Short(std::atoi(lastText.c_str()));
                } else if (keywordKind==CORBA::tk_char) {
                    currentSri.keywords[cl].value <<= CORBA::Any::from_char(lastText.c_str()[0]);
                } else if (keywordKind==CORBA::tk_float) {
                    currentSri.keywords[cl].value <<= CORBA::Float(std::atof(lastText.c_str()));
                } else if (keywordKind==CORBA::tk_double) {
                    currentSri.keywords[cl].value <<= CORBA::Double(std::atof(lastText.c_str()));
                } else if (keywordKind==CORBA::tk_long) {
                    currentSri.keywords[cl].value <<= CORBA::Long(std::atol(lastText.c_str()));
                } else if (keywordKind==CORBA::tk_ushort) {
                    currentSri.keywords[cl].value <<= CORBA::UShort(std::atol(lastText.c_str()));
                } else if (keywordKind==CORBA::tk_octet) {
                    currentSri.keywords[cl].value <<= CORBA::Any::from_octet(std::atoi(lastText.c_str()));
                } else if (keywordKind==CORBA::tk_string) {
                    currentSri.keywords[cl].value <<= CORBA::string_dup(lastText.c_str());
                } else if (keywordKind==CORBA::tk_boolean) {
                    if (lastText =="false" || lastText =="FALSE" || lastText =="False") {
                        currentSri.keywords[cl].value <<= CORBA::Boolean(false);
                    } else if (lastText =="true" || lastText =="TRUE" || lastText =="True"){
                        currentSri.keywords[cl].value <<= CORBA::Boolean(true);
                    } else {
                        currentSri.keywords[cl].value <<= CORBA::Boolean((atoi(lastText.c_str()) != 0));
                    }
                }
            } else {
                // each sri element only has a value
                // value is stored in 'lastText'
                if (currentElement.name =="hversion")         currentSri.hversion = CORBA::Long(atol(lastText.c_str()));
                else if (currentElement.name =="xstart")    currentSri.xstart = CORBA::Double(atof(lastText.c_str()));
                else if (currentElement.name == "xdelta")     currentSri.xdelta = CORBA::Double(atof(lastText.c_str()));
                else if (currentElement.name == "xunits")     currentSri.xunits = CORBA::Short(atoi(lastText.c_str()));
                else if (currentElement.name == "subsize")     currentSri.subsize = CORBA::Long(atol(lastText.c_str()));
                else if (currentElement.name == "ystart")     currentSri.ystart = CORBA::Double(atof(lastText.c_str()));
                else if (currentElement.name == "ydelta")     currentSri.ydelta = CORBA::Double(atof(lastText.c_str()));
                else if (currentElement.name == "yunits")     currentSri.yunits = CORBA::Short(atoi(lastText.c_str()));
                else if (currentElement.name == "mode")     currentSri.mode = CORBA::Short(atoi(lastText.c_str()));
                else if (currentElement.name == "streamID") currentSri.streamID = CORBA::String_member(lastText.c_str());
                else if (currentElement.name == "blocking") {
                    LOG_WARN(MetaDataParser,"Metadata file SRI blocking flag is overridden by default_sri.blocking property");
                } else {
                    LOG_ERROR(MetaDataParser,"Unknown sri sub-element with name "<<currentElement.name);
                }
            }
            // end if parentElement is 'sri'
        } else if (parentElement.name=="timecode") {
            // Parent is timecode, to one of the timecode sub elements
            if (currentElement.name=="tcmode") {          currentPacket.tstamp.tcmode = CORBA::Short(atoi(lastText.c_str()));
            } else if (currentElement.name=="tcstatus") { currentPacket.tstamp.tcstatus = CORBA::Short(atoi(lastText.c_str()));
            } else if (currentElement.name=="toff") {     currentPacket.tstamp.toff = CORBA::Double(atof(lastText.c_str()));
            } else if (currentElement.name=="twsec") {    currentPacket.tstamp.twsec = CORBA::Double(atof(lastText.c_str()));
            } else if (currentElement.name=="tfsec") {    currentPacket.tstamp.tfsec = CORBA::Double(atof(lastText.c_str()));
            } else { LOG_ERROR(MetaDataParser,"Unknown timecode sub-element with name "<<currentElement.name);
            }
            // end if parentElement is 'timecode'
        } else if (parentElement.name=="packet") {
            // Parent is packet, element is either timecode or an individual packet element
            if (currentElement.name == "timecode") {
                // put 'lastText' back onto 'text' vector because we didn't use it.
                if (!lastText.empty()) text.push_back(lastText);
            } else if (currentElement.name =="datalength") {
                currentPacket.dataLength = atoi(lastText.c_str());
            } else if (currentElement.name =="streamID") {
                currentPacket.streamID = CORBA::String_member(lastText.c_str());
            } else if (currentElement.name == "EOS") {
                if (lastText =="false" || lastText =="FALSE" || lastText =="False") {
                    currentPacket.EOS = CORBA::Boolean(false);
                } else if (lastText =="true" || lastText =="TRUE" || lastText =="True"){
                    currentPacket.EOS = CORBA::Boolean(true);
                } else {
                    currentPacket.EOS = (atoi(lastText.c_str()) != 0);
                }
            } else {
                LOG_ERROR(MetaDataParser,"Unknown packet sub-element with name "<<currentElement.name);
            }
            // end if parentElement is 'packet'
        } else {
            LOG_ERROR(MetaDataParser,"Unknown element/sub-element combo. parent="<<parentElement.name<<" and sub="<<currentElement.name);
        }
    }
}

void MetaDataParser::parseData(std::vector<char> xmldata) {
    reset();
    int done=1;
    if (!XML_Parse(parser, &xmldata[0],xmldata.size(), done)) {
        LOG_ERROR(MetaDataParser,"Parse Error at line "<<XML_GetCurrentLineNumber(parser)<<"\nError:\n"<<XML_ErrorString(XML_GetErrorCode(parser)));
    }
}

MetaDataParser::~MetaDataParser() {
    // TODO Auto-generated destructor stub
    XML_ParserFree(parser);
}


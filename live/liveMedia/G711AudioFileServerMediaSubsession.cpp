#include "G711AudioFileServerMediaSubsession.hh"
#include "G711AudioFileSource.hh"
#include "SimpleRTPSink.hh"

G711AudioFileServerMediaSubsession* G711AudioFileServerMediaSubsession
::createNew(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource) {
    return new G711AudioFileServerMediaSubsession(env, fileName,reuseFirstSource);
}

G711AudioFileServerMediaSubsession
::G711AudioFileServerMediaSubsession(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource)
    :FileServerMediaSubsession(env, fileName, reuseFirstSource){
}

G711AudioFileServerMediaSubsession
::~G711AudioFileServerMediaSubsession() {
}

FramedSource* G711AudioFileServerMediaSubsession
::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
    FramedSource* resultSource = NULL;
    do {
        G711AudioFileSource* g711Source
            = G711AudioFileSource::createNew(envir(),fFileName);
        if (g711Source == NULL) break;

        // Get attributes of the audio source:

        fBitsPerSample = g711Source->bitsPerSample();
        if (!(fBitsPerSample == 4 || fBitsPerSample == 8 || fBitsPerSample == 16)) {
            envir() << "The input file contains " << fBitsPerSample << " bit-per-sample audio, which we don't handle\n";
            break;
        }
        fSamplingFrequency = g711Source->samplingFrequency();
        fNumChannels = g711Source->numChannels();
        unsigned bitsPerSecond
            = fSamplingFrequency*fBitsPerSample*fNumChannels;

        resultSource = g711Source;

        estBitrate = (bitsPerSecond+500)/1000; // kbps
        return resultSource;
    } while (0);

    // An error occurred:
    Medium::close(resultSource);
    return NULL;
}

RTPSink* G711AudioFileServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
        unsigned char rtpPayloadTypeIfDynamic,
        FramedSource* /*inputSource*/) {
    const char *mimeType = "PCMA";
    unsigned char payloadFormatCode;
    if (fSamplingFrequency == 8000 && fNumChannels == 1) {
        payloadFormatCode = 8; // a static RTP payload type
    } else {
        payloadFormatCode = rtpPayloadTypeIfDynamic;
    }
    return SimpleRTPSink::createNew(envir(), rtpGroupsock,
            payloadFormatCode, fSamplingFrequency,
            "audio", mimeType, fNumChannels);
}

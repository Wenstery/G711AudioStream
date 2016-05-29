#ifndef _G711_AUDIO_FILE_SERVER_MEDIA_SUBSESSION_HH
#define _G711_AUDIO_FILE_SERVER_MEDIA_SUBSESSION_HH

#ifndef _FILE_SERVER_MEDIA_SUBSESSION_HH
#include "FileServerMediaSubsession.hh"
#endif

class G711AudioFileServerMediaSubsession: public FileServerMediaSubsession{
public:
  static G711AudioFileServerMediaSubsession*
  createNew(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource);

protected:
  G711AudioFileServerMediaSubsession(UsageEnvironment& env,
				      char const* fileName, Boolean reuseFirstSource);
      // called only by createNew();
  virtual ~G711AudioFileServerMediaSubsession();

protected: // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);

protected:
  // The following parameters of the input stream are set after
  // "createNewStreamSource" is called:
  unsigned char fBitsPerSample;
  unsigned fSamplingFrequency;
  unsigned fNumChannels;
};

#endif

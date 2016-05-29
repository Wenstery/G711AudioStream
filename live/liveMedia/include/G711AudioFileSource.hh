#ifndef _G711_AUDIO_FILE_SOURCE_HH
#define _G711_AUDIO_FILE_SOURCE_HH

#ifndef _FRAMED_FILE_SOURCE_HH
#include "FramedFileSource.hh"
#endif

class G711AudioFileSource: public FramedFileSource {
public:
  unsigned char bitsPerSample() const { return fBitsPerSample; }
  unsigned char numChannels() const { return fNumChannels; }
  unsigned samplingFrequency() const { return fSamplingFrequency; }

  static G711AudioFileSource* createNew(UsageEnvironment& env,
				       char const* fileName);

protected:
  G711AudioFileSource(UsageEnvironment& env, FILE* fid);
	// called only by createNew()

  virtual ~G711AudioFileSource();

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();

private:
  unsigned char fNumChannels;
  unsigned fSamplingFrequency;
  unsigned char fBitsPerSample;
  unsigned fPreferredFrameSize;
  Boolean fLimitNumBytesToStream;
  unsigned fNumBytesToStream;
  unsigned fLastPlayTime;
  double fPlayTimePerSample; // useconds
};

#endif

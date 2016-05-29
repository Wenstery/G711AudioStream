#include "G711AudioFileSource.hh"
#include "InputFile.hh"
#include <GroupsockHelper.hh>
static int audioGetOneFrame(unsigned char *buf, unsigned int *size,FILE *fid){
    int ret;

    ret = fread(buf, 1, 320, fid);
    if(ret < 0) {
        printf("ERR : %s : %d\n", __FILE__, __LINE__);
        return -1;
    }

    *size = 320;

    return 0;
}
G711AudioFileSource*
G711AudioFileSource::createNew(UsageEnvironment& env,char const* fileName) {
	FILE* fid = NULL;
    do {
         fid = OpenInputFile(env, fileName);
         if (fid == NULL) break;
		 return new G711AudioFileSource(env, fid);
    } while (0);
    CloseInputFile(fid);
    return NULL;
}

G711AudioFileSource::G711AudioFileSource(UsageEnvironment& env,FILE *fid)
    : FramedFileSource(env,fid),
    fNumChannels(0), fSamplingFrequency(0),
    fBitsPerSample(0),
    fLimitNumBytesToStream(False),
    fNumBytesToStream(0),
    fLastPlayTime(0),
    fPlayTimePerSample(0){

    fNumChannels = 1;
    fSamplingFrequency = 8000;
    fBitsPerSample = 8;

    fPlayTimePerSample = 1e6/(double)fSamplingFrequency;
    // Although PCM is a sample-based format, we group samples into
    // 'frames' for efficient delivery to clients.  Set up our preferred
    // frame size to be close to 20 ms, if possible, but always no greater
    // than 1400 bytes (to ensure that it will fit in a single RTP packet)
    unsigned maxSamplesPerFrame = (1400*8)/(fNumChannels*fBitsPerSample);
    unsigned desiredSamplesPerFrame = (unsigned)(0.04*fSamplingFrequency);
    unsigned samplesPerFrame = desiredSamplesPerFrame < maxSamplesPerFrame ? desiredSamplesPerFrame : maxSamplesPerFrame;
    fPreferredFrameSize = (samplesPerFrame*fNumChannels*fBitsPerSample)/8;
}

G711AudioFileSource::~G711AudioFileSource() {
    CloseInputFile(fFid);
}

// Note: We should change the following to use asynchronous file reading, #####
// as we now do with ByteStreamFileSource. #####
void G711AudioFileSource::doGetNextFrame() {
    // Try to read as many bytes as will fit in the buffer provided (or "fPreferredFrameSize" if less)
    if (fLimitNumBytesToStream && fNumBytesToStream < fMaxSize) {
        fMaxSize = fNumBytesToStream;
    }
    if (fPreferredFrameSize < fMaxSize) {
        fMaxSize = fPreferredFrameSize;
    }
    unsigned bytesPerSample = (fNumChannels*fBitsPerSample)/8;
    if (bytesPerSample == 0) bytesPerSample = 1; // because we can't read less than a byte at a time
    //unsigned bytesToRead = fMaxSize - fMaxSize%bytesPerSample;

    //fFrameSize : 1000
    audioGetOneFrame(fTo, &fFrameSize,fFid);

    // Set the 'presentation time' and 'duration' of this frame:
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
        // This is the first frame, so use the current time:
        gettimeofday(&fPresentationTime, NULL);
    } else {
        // Increment by the play time of the previous data:
        unsigned uSeconds	= fPresentationTime.tv_usec + fLastPlayTime;
        fPresentationTime.tv_sec += uSeconds/1000000;
        fPresentationTime.tv_usec = uSeconds%1000000;
    }

    // Remember the play time of this data:
    fDurationInMicroseconds = fLastPlayTime
        = (unsigned)((fPlayTimePerSample*fFrameSize)/bytesPerSample);

    // Switch to another task, and inform the reader that he has data:
#if defined(__WIN32__) || defined(_WIN32)
    // HACK: One of our applications that uses this source uses an
    // implementation of scheduleDelayedTask() that performs very badly
    // (chewing up lots of CPU time, apparently polling) on Windows.
    // Until this is fixed, we just call our "afterGetting()" function
    // directly.  This avoids infinite recursion, as long as our sink
    // is discontinuous, which is the case for the RTP sink that
    // this application uses. #####
    afterGetting(this);
#else
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
            (TaskFunc*)FramedSource::afterGetting, this);
#endif
}



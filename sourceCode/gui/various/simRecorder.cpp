#include <simRecorder.h>
#include <oGL.h>
#include <tt.h>
#include <utils.h>
#include <QScreen>
//#include <QDesktopWidget>
#include <imgLoaderSaver.h>
#include <auxLibVideo.h>
#include <simStrings.h>
#include <vDateTime.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CSimRecorder::CSimRecorder(const char* initialPath)
{
    _isRecording = false;
    _recorderEnabled = false;
    _showCursor = false;
    _showButtonStates = false;
    _hideInfoTextAndStatusBar = true;
    _frameCount = 0;
    _frameRate =
        20; // was 30 until 18/1/2013. With 20, people get less confused about how to record a real-time video ;)
    _recordDesktopInstead = false;
    _autoFrameRate = true;
    _simulationFrameCount = 0;
    _recordEveryXRenderedFrame = 1;
    _aviGenInitialized = false;
    _buffer = nullptr;
    _tempBuffer = nullptr;
    _resolution[0] = 640;
    _resolution[1] = 480;
    _showSavedMessage = true;
    _path = initialPath;
    _pathAndFilename = "";
    _manualStart = true;
    _encoderIndex = 0;
    _simulationTimeOfLastFrame = -1.0;
}

CSimRecorder::~CSimRecorder()
{
    stopRecording(true);
    stopRecording(false);
}

void CSimRecorder::setShowSavedMessage(bool s)
{
    _showSavedMessage = s;
}

void CSimRecorder::setShowCursor(bool show)
{
    _showCursor = show;
}

bool CSimRecorder::getShowCursor()
{
    if (_recordDesktopInstead)
        return (false);
    return (_showCursor);
}

void CSimRecorder::setAutoFrameRate(bool a)
{
    _autoFrameRate = a;
}

bool CSimRecorder::getAutoFrameRate()
{
    return (_autoFrameRate);
}

void CSimRecorder::setShowButtonStates(bool show)
{
    _showButtonStates = show;
}

bool CSimRecorder::getShowButtonStates()
{
    if (_recordDesktopInstead)
        return (false);
    return (_showButtonStates);
}

bool CSimRecorder::getHideInfoTextAndStatusBar()
{
    return (_hideInfoTextAndStatusBar);
}

void CSimRecorder::setHideInfoTextAndStatusBar(bool hide)
{
    _hideInfoTextAndStatusBar = hide;
}

void CSimRecorder::setEncoderIndex(int index)
{
    _encoderIndex = index;
}

int CSimRecorder::getEncoderIndex()
{
    return (_encoderIndex);
}

void CSimRecorder::setRecordingSizeChanged(int newXsize, int newYsize)
{
    _resolutionInfo[0] = newXsize;
    _resolutionInfo[1] = newYsize;
}

void CSimRecorder::getRecordingSize(int& x, int& y)
{
    x = _resolutionInfo[0];
    y = _resolutionInfo[1];
}

std::string CSimRecorder::getEncoderString(int index)
{
    std::string retStr;
    char txt[201];
    if ('\0' == CAuxLibVideo::video_recorderGetEncoderString(index, txt))
        retStr = txt;
    return (retStr);
}

bool CSimRecorder::recordFrameIfNeeded(int resX, int resY, int posX, int posY)
{                               // return value false indicates a failed initialization
    static bool inside = false; // this function is not re-entrant!
    if (inside)
        return (true);
    TRACE_INTERNAL;
    inside = true;
    if (_isRecording)
    {
        bool validFrame = true;
        if (!getManualStart())
        {
            double simTime = App::currentWorld->simulation->getSimulationTime();
            if (_simulationTimeOfLastFrame != simTime)
                _simulationTimeOfLastFrame = simTime;
            else
                validFrame = false;
        }

        if (validFrame)
        {
            _simulationFrameCount++;
            if (_simulationFrameCount >= _recordEveryXRenderedFrame)
            { // we record this frame
                if ((!_aviGenInitialized) && (!_initFailed))
                { // Initialize the recorder

                    if (_recordDesktopInstead)
                    {
                        QList<QScreen*> screens = QGuiApplication::screens();
                        int screenIndex = 0;
                        if ((App::userSettings->desktopRecordingIndex >= 0) &&
                            (App::userSettings->desktopRecordingIndex < screens.size()))
                            screenIndex = App::userSettings->desktopRecordingIndex;
                        QPixmap pixmap(screens[screenIndex]->grabWindow(0));
                        QImage img(pixmap.toImage());
                        if ((App::userSettings->desktopRecordingWidth > 100) &&
                            (App::userSettings->desktopRecordingWidth < 4000))
                            img =
                                (img.scaledToWidth(App::userSettings->desktopRecordingWidth, Qt::SmoothTransformation));
                        _resolution[0] = img.width();
                        _resolution[1] = img.height();
                    }
                    else
                    {
                        _resolution[0] = resX; //&0x0ffc; // must be multiple of 4!
                        _resolution[1] = resY; //&0x0ffc; // must be multiple of 4!
                    }
                    char txt[201];
                    CAuxLibVideo::video_recorderGetEncoderString(_encoderIndex, txt);
                    char userSet;
                    _filenameAndPathAndExtension = getPath(&userSet);
                    if (userSet == 0)
                    {
                        _filenameAndPathAndExtension += "/recording_";
                        int year, month, day, hour, minute, second;
                        VDateTime::getYearMonthDayHourMinuteSecond(&year, &month, &day, &hour, &minute, &second);
                        _filenameAndPathAndExtension += utils::getIntString(false, year, 4) + "_";
                        _filenameAndPathAndExtension += utils::getIntString(false, month, 2) + "_";
                        _filenameAndPathAndExtension += utils::getIntString(false, day, 2) + "-";
                        _filenameAndPathAndExtension += utils::getIntString(false, hour, 2) + "_";
                        _filenameAndPathAndExtension += utils::getIntString(false, minute, 2) + "-";
                        _filenameAndPathAndExtension += utils::getIntString(false, second, 2);
                    }

                    if (strncmp(txt, "AVI/", 4) == 0)
                        _filenameAndPathAndExtension += ".avi";
                    else
                    {
                        if (strncmp(txt, "MP4/", 4) == 0)
                            _filenameAndPathAndExtension += ".mp4";
                        else
                            _filenameAndPathAndExtension += ".bin";
                    }
                    char res = CAuxLibVideo::video_recorderInitialize(_resolution[0], _resolution[1],
                                                                      _filenameAndPathAndExtension.c_str(),
                                                                      getFrameRate(), _encoderIndex);
                    _aviGenInitialized = (res != 'e');
                    if (_aviGenInitialized)
                    {
                        App::logMsg(sim_verbosity_msgs, IDSNS_VIDEO_COMPRESSOR_INITIALIZED);
                        if (res == 'w')
                            App::logMsg(sim_verbosity_msgs, IDSNS_VIDEO_USING_PADDING);
                    }
                    else
                    {
                        App::logMsg(sim_verbosity_errors, IDSNS_VIDEO_COMPRESSOR_FAILED_TO_INITIALIZE);
                        GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, "Video Recorder",
                                                             IDSN_VIDEO_COMPRESSOR_FAILED_INITIALIZING_WARNING,
                                                             VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
                    }
                    _initFailed = (!_aviGenInitialized);
                    if (!_initFailed)
                    {
                        _buffer = new unsigned char[_resolution[0] * _resolution[1] * 3];
                        _tempBuffer = new unsigned char[_resolution[0] * _resolution[1] * 3];
                    }
                }

                if (_aviGenInitialized)
                { // Ok to record:
                    if (_recordDesktopInstead)
                    {
                        QList<QScreen*> screens = QGuiApplication::screens();
                        int screenIndex = 0;
                        if ((App::userSettings->desktopRecordingIndex >= 0) &&
                            (App::userSettings->desktopRecordingIndex < screens.size()))
                            screenIndex = App::userSettings->desktopRecordingIndex;
                        QPixmap pixmap(screens[screenIndex]->grabWindow(0));
                        /*
                        if (_showCursor)
                        {
                            QPixmap pixmapM(":/targaFiles/cur_arrow.tga");
                            QPainter painter(&pixmap);
                            painter.drawPixmap(GuiApp::mainWindow->cursor().pos(),pixmapM);
                        }
                        */
                        QImage img(pixmap.toImage());

                        if ((App::userSettings->desktopRecordingWidth > 100) &&
                            (App::userSettings->desktopRecordingWidth < 4000))
                            img =
                                (img.scaledToWidth(App::userSettings->desktopRecordingWidth, Qt::SmoothTransformation));

                        for (int i = 0; i < img.height(); i++)
                        {
                            for (int j = 0; j < img.width(); j++)
                            {
                                QRgb pix(img.pixel(j, i));
                                _buffer[3 * (j + i * img.width()) + 0] = qRed(pix);
                                _buffer[3 * (j + i * img.width()) + 1] = qGreen(pix);
                                _buffer[3 * (j + i * img.width()) + 2] = qBlue(pix);
                            }
                        }
                    }
                    else
                    {
                        glPixelStorei(GL_PACK_ALIGNMENT, 1);
                        glReadPixels(posX, posY, _resolution[0], _resolution[1], GL_RGB, GL_UNSIGNED_BYTE, _tempBuffer);
                        glPixelStorei(GL_PACK_ALIGNMENT, 4); // important to restore! Really?
                        for (int j = 0; j < _resolution[1]; j++)
                        {
                            int yp = j * _resolution[0];
                            int yq = (_resolution[1] - j - 1) * _resolution[0];
                            for (int i = 0; i < _resolution[0]; i++)
                            {
                                _buffer[3 * (yp + i) + 0] = _tempBuffer[3 * (yq + i) + 0];
                                _buffer[3 * (yp + i) + 1] = _tempBuffer[3 * (yq + i) + 1];
                                _buffer[3 * (yp + i) + 2] = _tempBuffer[3 * (yq + i) + 2];
                            }
                        }
                    }
                    CAuxLibVideo::video_recorderAddFrame(_buffer);
                    _simulationFrameCount = 0;
                    _frameCount++;
                }
            }
        }
    }
    inside = false;
    return ((!_initFailed) || (!_isRecording));
}

bool CSimRecorder::willNextFrameBeRecorded()
{
    if (_isRecording)
    {
        bool validFrame = true;
        if (!getManualStart())
        {
            if (_simulationTimeOfLastFrame == App::currentWorld->simulation->getSimulationTime())
                validFrame = false;
        }

        if (validFrame)
            return ((_simulationFrameCount + 1) >= _recordEveryXRenderedFrame);
    }
    return (false);
}

bool CSimRecorder::getIsRecording()
{
    return (_isRecording);
}

bool CSimRecorder::startRecording(bool manualStart)
{
    if (_recorderEnabled)
    {
        _simulationTimeOfLastFrame = -1.0;
        _manualStart = manualStart;
        _isRecording = true;
        _frameCount = 0;
        _simulationFrameCount = 0;
        _initFailed = false;
        _recorderEnabled = false;
        return (true);
    }
    return (false);
}

bool CSimRecorder::getManualStart()
{
    return (_manualStart);
}

void CSimRecorder::stopRecording(bool manualStop)
{
    TRACE_INTERNAL;
    bool doIt = false;
    if (!manualStop)
        doIt = !_manualStart;
    else
        doIt = _manualStart;
    if (doIt)
    {
        _isRecording = false;
        _frameCount = 0;
        _simulationFrameCount = 0;
        _manualStart = true;

        if (_aviGenInitialized)
        { // deinitialize the recorder:
            CAuxLibVideo::video_recorderEnd();
            _aviGenInitialized = false;
            delete[] _tempBuffer;
            delete[] _buffer;
            _buffer = nullptr;
            std::string tmp(IDS_AVI_FILE_WAS_SAVED);
            tmp += _filenameAndPathAndExtension + ")";

            App::logMsg(sim_verbosity_msgs, tmp.c_str());
            if (_showSavedMessage)
                GuiApp::uiThread->messageBox_information(GuiApp::mainWindow, IDSN_AVI_RECORDER, tmp.c_str(),
                                                         VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
            _showSavedMessage = true; // reset this flag
            GuiApp::setFullDialogRefreshFlag();
        }
    }
}

int CSimRecorder::getFrameCount()
{
    return (_frameCount);
}

void CSimRecorder::setRecordEveryXRenderedFrame(int x)
{
    if (x < 1)
        x = 1;
    if (x > 10000)
        x = 10000;
    _recordEveryXRenderedFrame = x;
    _simulationFrameCount = 0;
}

int CSimRecorder::getRecordEveryXRenderedFrame()
{
    return (_recordEveryXRenderedFrame);
}

void CSimRecorder::setFrameRate(int fps)
{
    if (fps > 200)
        fps = 200;
    if (fps < 1)
        fps = 1;
    _frameRate = fps;
}

int CSimRecorder::getFrameRate()
{
    if (_autoFrameRate)
    {
        int frate = int((1.0 / App::currentWorld->simulation->getTimeStep()) + 0.5);
        return (tt::getLimitedInt(1, 120, frate)); // the recorder probably doesn't support that high (120)
    }
    return (_frameRate);
}

void CSimRecorder::setDesktopRecording(bool dr)
{
    _recordDesktopInstead = dr;
}

bool CSimRecorder::getDesktopRecording() const
{
    return (_recordDesktopInstead);
}

void CSimRecorder::setPath(const char* path)
{
    _path = std::string(path);
    _pathAndFilename = "";
}

void CSimRecorder::setPathAndFilename(const char* pathAndF)
{ // when set via API
    _pathAndFilename = std::string(pathAndF);
}

std::string CSimRecorder::getPath(char* userSet)
{
    if (_pathAndFilename.length() > 0)
    {
        if (userSet != nullptr)
            userSet[0] = 1;
        return (_pathAndFilename);
    }
    if (userSet != nullptr)
        userSet[0] = 0;
    return (_path);
}

void CSimRecorder::setRecorderEnabled(bool e)
{
    _recorderEnabled = e;
}

bool CSimRecorder::getRecorderEnabled()
{
    return (_recorderEnabled);
}

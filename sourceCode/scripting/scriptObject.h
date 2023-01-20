#pragma once

#include "simTypes.h"
#include "outsideCommandQueueForScript.h"
#include "interfaceStack.h"
#include <random>

#define DEFAULT_MAINSCRIPT_CODE "-- The main script is not supposed to be modified, except in special cases.\nrequire('defaultMainScript')"
#define DEFAULT_NONTHREADEDCHILDSCRIPT "childScript"
#define DEFAULT_THREADEDCHILDSCRIPT "childScript-threaded"
#define DEFAULT_NONTHREADEDCUSTOMIZATIONSCRIPT "customizationScript"
#define DEFAULT_THREADEDCUSTOMIZATIONSCRIPT "customizationScript-threaded"
#define SIM_SCRIPT_HANDLE "sim_script_handle" // keep this global, e.g. not _S.sim_script_handle

// Old:
// **********************
#include "userParameters.h"
#include "vMutex.h"
#include "vThread.h"
#include "customData_old.h"
#define DEFAULT_THREADEDCHILDSCRIPTOLD "dlttscptbkcomp"
#define SIM_SCRIPT_NAME_INDEX_OLD "sim_script_name_index" // keep this global, e.g. not _S.sim_script_name_index
// **********************

class CSceneObject;

class CScriptObject
{
public:
    enum {
        scriptState_unloaded=0,
        scriptState_uninitialized,
        scriptState_initialized,
        scriptState_ended,
        scriptState_error=8,
        scriptState_suspended=16 // only add-ons
    };
    enum {
        lang_undefined=-1,
        lang_lua=0,
        lang_python
    };

    CScriptObject(int scriptTypeOrMinusOneForSerialization);
    virtual ~CScriptObject();

    static void destroy(CScriptObject* obj,bool registeredObject);

    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationAboutToEnd();
    void simulationEnded();

    int getScriptHandle() const;
    size_t getSimpleHash() const;

    std::string getDescriptiveName() const;
    std::string getShortDescriptiveName() const;
    void setAddOnName(const char* name);
    std::string getScriptName() const;

    CScriptObject* copyYourself();
    void serialize(CSer& ar);
    void performSceneObjectLoadingMapping(const std::map<int,int>* map);
    bool announceSceneObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    int flagScriptForRemoval();
    void setObjectHandleThatScriptIsAttachedTo(int newObjectHandle);
    int getObjectHandleThatScriptIsAttachedTo(int scriptTypeToConsider) const;

    void setScriptText(const char* scriptTxt);
    bool setScriptTextFromFile(const char* filename);
    const char* getScriptText();

    void resetCalledInThisSimulationStep();
    bool getCalledInThisSimulationStep() const;

    int systemCallMainScript(int optionalCallType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int systemCallScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool addOnManuallyStarted=false);
    int callCustomScriptFunction(const char* functionName,CInterfaceStack* inOutStack);
    bool shouldTemporarilySuspendMainScript();

    int executeScriptString(const char* scriptString,CInterfaceStack* outStack);

    void terminateScriptExecutionExternally(bool generateErrorMsg);

    bool resetScript();
    bool initScript();
    bool hasInterpreterState() const;
    bool isSimulationScript() const;
    bool isEmbeddedScript() const;
    bool isSceneSwitchPersistentScript() const;
    int getNumberOfPasses() const;
    void setNumberOfPasses(int p);
    void setTreeTraversalDirection(int dir);
    int getTreeTraversalDirection() const;
    void flagForDestruction();
    bool getFlaggedForDestruction() const;
    int getScriptType() const;
    void setScriptIsDisabled(bool isDisabled);
    bool getScriptIsDisabled() const;
    bool getScriptEnabledAndNoErrorRaised() const;
    void getPreviousEditionWindowPosAndSize(int posAndSize[4]) const;
    void setPreviousEditionWindowPosAndSize(const int posAndSize[4]);

    int getScriptExecutionTimeInMs() const;
    void resetScriptExecutionTime();

    double getRandomDouble();
    void setRandomSeed(unsigned int s);

    std::string getAndClearLastStackTraceback();

    int getScriptState() const;
    void setScriptState(int state);

    void setScriptExecPriority(int priority);
    int getScriptExecPriority() const;


    bool addCommandToOutsideCommandQueue(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const double aux2Vals[8],int aux2Count);
    int extractCommandFromOutsideCommandQueue(int auxVals[4],double aux2Vals[8],int& aux2Count);

    bool hasSystemFunction(int callType) const;
    bool getOldCallMode() const;

    std::string getFilenameForExternalScriptEditor() const;
    void fromFileToBuffer(); // when using an external editor
    void fromBufferToFile() const;

    bool shouldAutoYield();
    bool canManualYield() const;
    int getDelayForAutoYielding() const;
    void setDelayForAutoYielding(int d);
    int changeAutoYieldingForbidLevel(int dx,bool absolute);
    int getAutoYieldingForbidLevel() const;
    int changeOverallYieldingForbidLevel(int dx,bool absolute);
    int getLanguage();

    void registerPluginFunctions();
    bool registerPluginVariables(bool onlyRequireStatements);

    int getAddOnUiMenuHandle() const;
    void setAddOnFilePath(const char* p);
    std::string getAddOnFilePath() const;

    void printInterpreterStack() const;

    bool hasFunctionHook(const char* sysFunc) const;
    int registerFunctionHook(const char* sysFunc,const char* userFunc,bool before);

    static void getMatchingFunctions(const char* txt,std::vector<std::string>& v);
    static void getMatchingConstants(const char* txt,std::vector<std::string>& v);
    static std::string getFunctionCalltip(const char* txt);
    static int isFunctionOrConstDeprecated(const char* txt);
    static bool canCallSystemCallback(int scriptType,bool threadedOld,int callType);
    static bool isSystemCallbackInReverseOrder(int callType);
    static bool isSystemCallbackInterruptible(int callType);
    static std::string getSystemCallbackString(int calltype,int what);
    static std::vector<int> getAllSystemCallbacks(int scriptType,bool threadedOld);
    static std::vector<std::string> getAllSystemCallbackStrings(int scriptType,int what);

    static void setInExternalCall(int scriptHandle);
    static int getInExternalCall();

    // Lua specific:
    // -----------------------------
    void registerNewFunctions_lua();
    static void buildFromInterpreterStack_lua(void* LL,CInterfaceStack* stack,int fromPos,int cnt);
    static void buildOntoInterpreterStack_lua(void* LL,const CInterfaceStack* stack,bool takeOnlyTop);
    static int getScriptHandleFromInterpreterState_lua(void* LL);
    static std::string getSearchPath_lua();
    static std::string getSearchCPath_lua();
    // -----------------------------

    // Python specific:
    // -----------------------------
    static std::string getSearchPath_python();
    // -----------------------------

    // Old:
    // *****************************************
    std::string getScriptPseudoName_old() const;
    int setScriptVariable_old(const char* variableName,CInterfaceStack* stack);
    void setObjectCustomData_old(int header,const char* data,int dataLength);
    int getObjectCustomDataLength_old(int header) const;
    void getObjectCustomData_old(int header,char* data) const;
    void setObjectCustomData_tempData_old(int header,const char* data,int dataLength);
    int getObjectCustomDataLength_tempData_old(int header) const;
    void getObjectCustomData_tempData_old(int header,char* data) const;
    void setRaiseErrors_backCompatibility(bool raise);
    bool getRaiseErrors_backCompatibility() const;
    VTHREAD_ID_TYPE getThreadedScriptThreadId_old() const;
    void setAutomaticCascadingCallsDisabled_old(bool disabled);
    bool getAutomaticCascadingCallsDisabled_old() const;
    bool checkAndSetWarningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014();
    bool checkAndSetWarning_simRMLPosition_oldCompatibility_30_8_2014();
    bool checkAndSetWarning_simRMLVelocity_oldCompatibility_30_8_2014();
    CUserParameters* getScriptParametersObject_backCompatibility();
    void setCustScriptDisabledDSim_compatibilityMode_DEPRECATED(bool disabled);
    bool getCustScriptDisabledDSim_compatibilityMode_DEPRECATED() const;
    void setCustomizationScriptCleanupBeforeSave_DEPRECATED(bool doIt);
    bool getCustomizationScriptCleanupBeforeSave_DEPRECATED() const;
    int appendTableEntry_DEPRECATED(const char* arrayName,const char* keyName,const char* data,const int what[2]); // deprecated
    int callScriptFunction_DEPRECATED(const char* functionName, SLuaCallBack* pdata);
    int clearScriptVariable_DEPRECATED(const char* variableName); // deprecated
    void setThreadedExecution_oldThreads(bool threadedExec);
    bool getThreadedExecution_oldThreads() const;
    bool getThreadedExecutionIsUnderWay_oldThreads() const;
    void setExecuteJustOnce_oldThreads(bool justOnce);
    bool getExecuteJustOnce_oldThreads() const;
    bool launchThreadedChildScript_oldThreads();
    int resumeThreadedChildScriptIfLocationMatch_oldThreads(int resumeLocation);
    void setLastError_old(const char* err);
    std::string getAndClearLastError_old();
    void setExecutionPriority_old(int order);
    int getExecutionPriority_old() const;
    static void setScriptNameIndexToInterpreterState_lua_old(void* LL,int index);
    static int getScriptNameIndexFromInterpreterState_lua_old(void* LL);
    // *****************************************

protected:
    bool _initInterpreterState(std::string* errorMsg);
    bool _killInterpreterState();
    void _announceErrorWasRaisedAndPossiblyPauseSimulation(const char* errMsg,bool runtimeError);
    bool _loadCode();
    int ___loadCode(const char* code,const char* functionsToFind,std::vector<bool>& functionsFound,std::string* errorMsg);
    int _callSystemScriptFunction(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int _callScriptFunction(const char* functionName,const CInterfaceStack* inStack,CInterfaceStack* outStack,std::string* errorMsg);
    int _callScriptFunc(const char* functionName,const CInterfaceStack* inStack,CInterfaceStack* outStack,std::string* errorMsg);
    bool _execScriptString(const char* scriptString,CInterfaceStack* outStack);
    void _handleInfoCallback();


    int _scriptHandle; // is unique since 25.11.2022
    int _scriptType;
    bool _scriptIsDisabled;
    int _scriptState;
    int _executionDepth;
    int _treeTraversalDirection;
    int _objectHandleAttachedTo;
    int _autoStartAddOn;
    int _addOnUiMenuHandle;
    int _scriptExecPriority; // only for add-ons. Not saved
    std::string _addOnFilePath;

    bool _calledInThisSimulationStep;

    int _timeForNextAutoYielding;
    int _delayForAutoYielding;
    int _forbidAutoYieldingLevel;
    int _forbidOverallYieldingLevel;

    std::string _scriptText;
    std::string _scriptTextExec; // the one getting executed!
    bool _externalScriptText;

    COutsideCommandQueueForScript* _outsideCommandQueue;

    void* _interpreterState;  // !! _interpreterState is not the same for a script when in normal or inside a coroutine !!

    int _numberOfPasses;

    bool _flaggedForDestruction;

    int _timeOfScriptExecutionStart;
    std::string _lastStackTraceback;

    std::vector<bool> _containedSystemCallbacks;
    bool _initiallyHadSystemCallback_event;
    bool _initiallyHadSystemCallback_dyn;
    bool _initiallyHadSystemCallback_contact;
    bool _initiallyHadSystemCallback_joint;
    void _printContext(const char* str,size_t p);

    std::string _addOnName;
    std::mt19937 _randGen;

    bool _initialValuesInitialized;
    int _previousEditionWindowPosAndSize[4];

    std::string _filenameForExternalScriptEditor;
    std::vector<std::string> _functionHooks_before;
    std::vector<std::string> _functionHooks_after;
    int _initFunctionHookCount;


    static int _nextIdForExternalScriptEditor;
    static int _nextScriptHandle;
    static std::vector<int> _externalScriptCalls;

    // Lua specific:
    // -----------------------------
    int _execSimpleString_safe_lua(void* LL,const char* string);
    int _loadBufferResult_lua;
    bool _loadBuffer_lua(const char* buff,size_t sz,const char* name);
    void _registerNewVariables_lua();
    static CInterfaceStackObject* _generateObjectFromInterpreterStack_lua(void* LL,int index,std::map<void*,bool>& visitedTables);
    static CInterfaceStackTable* _generateTableArrayFromInterpreterStack_lua(void* LL,int index,std::map<void*,bool>& visitedTables);
    static CInterfaceStackTable* _generateTableMapFromInterpreterStack_lua(void* LL,int index,std::map<void*,bool>& visitedTables);
    static int _countInterpreterStackTableEntries_lua(void* LL,int index);
    static void _pushOntoInterpreterStack_lua(void* LL,CInterfaceStackObject* obj);
    static void _hookFunction_lua(void* LL,void* arr);
    static void _setScriptHandleToInterpreterState_lua(void* LL,int h);
    // -----------------------------

    // Old:
    // *****************************************
    void _handleCallbackEx_old(int calltype);
    int _getScriptNameIndexNumber_old() const;
    bool _callScriptChunk_old(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    bool _checkIfMixingOldAndNewCallMethods_old();
    std::string _replaceOldApi(const char* txt,bool forwardAdjustment);
    bool _convertThreadedScriptToCoroutine_old(CScriptObject* scriptObject);
    void _adjustScriptText1_old(CScriptObject* scriptObject,bool doIt,bool doIt2);
    void _adjustScriptText2_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText3_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText4_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText5_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText6_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText7_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText10_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText11_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText12_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText13_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText14_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText15_old(CScriptObject* scriptObject,bool doIt);
    void _adjustScriptText16_old(CScriptObject* scriptObject,bool doIt);
    void _detectDeprecated_old(CScriptObject* scriptObject);
    void _insertScriptText_old(CScriptObject* scriptObject,bool toFront,const char* txt);
    bool _replaceScriptText_old(CScriptObject* scriptObject,const char* oldTxt,const char* newTxt);
    bool _replaceScriptText_old(CScriptObject* scriptObject,const char* oldTxt1,const char* oldTxt2,const char* oldTxt3,const char* newTxt);
    bool _replaceScriptTextKeepMiddleUnchanged_old(CScriptObject* scriptObject,const char* oldTxtStart,const char* oldTxtEnd,const char* newTxtStart,const char* newTxtEnd);
    bool _containsScriptText_old(CScriptObject* scriptObject,const char* txt);
    void _performNewApiAdjustments_old(CScriptObject* scriptObject,bool forwardAdjustment);
    void _splitApiText_old(const char* txt,size_t pos,std::string& beforePart,std::string& apiWord,std::string& afterPart);
    std::string _lastError_old;
    bool _compatibilityMode_oldLua;
    bool _custScriptDisabledDSim_compatibilityMode_DEPRECATED;
    bool _customizationScriptCleanupBeforeSave_DEPRECATED;
    bool _mainScriptIsDefaultMainScript_old; // 16.11.2020
    bool _raiseErrors_backCompatibility;
    CUserParameters* _scriptParameters_backCompatibility;
    CCustomData_old* _customObjectData_old;
    CCustomData_old* _customObjectData_tempData_old;
    bool _warningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014;
    bool _warning_simRMLPosition_oldCompatibility_30_8_2014;
    bool _warning_simRMLVelocity_oldCompatibility_30_8_2014;
    bool _warning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016;
    bool _warning_simFindIkPath_oldCompatibility_2_2_2016;
    bool _automaticCascadingCallsDisabled_old; // reset to false at simulation start!
    bool _threadedExecution_oldThreads;
    VTHREAD_ID_TYPE _threadedScript_associatedFiberOrThreadID_oldThreads;
    bool _threadedExecutionUnderWay_oldThreads;
    bool _executeJustOnce_oldThreads;
    int _executionPriority_old;
    void _launchThreadedChildScriptNow_oldThreads();
    static std::map<std::string,std::string> _newApiMap_old;
    static VMutex _globalMutex_oldThreads;
    static std::vector<CScriptObject*> toBeCalledByThread_oldThreads;
    static VTHREAD_RETURN_TYPE _startAddressForThreadedScripts_oldThreads(VTHREAD_ARGUMENT_TYPE lpData);
    // *****************************************
};

// Old:
// *****************************************
struct SNewApiMapping
{
    std::string oldApi;
    std::string newApi;
};
const extern SNewApiMapping _simApiMapping[];
const extern SNewApiMapping _simBubbleApiMapping[];
const extern SNewApiMapping _simK3ApiMapping[];
const extern SNewApiMapping _simMTBApiMapping[];
const extern SNewApiMapping _simOpenMeshApiMapping[];
const extern SNewApiMapping _simSkeletonApiMapping[];
const extern SNewApiMapping _simQHullApiMapping[];
const extern SNewApiMapping _simRemoteApiApiMapping[];
const extern SNewApiMapping _simRRS1ApiMapping[];
const extern SNewApiMapping _simVisionApiMapping[];
const extern SNewApiMapping _simCamApiMapping[];
const extern SNewApiMapping _simJoyApiMapping[];
const extern SNewApiMapping _simWiiApiMapping[];
const extern SNewApiMapping _simURDFApiMapping[];
const extern SNewApiMapping _simBWFApiMapping[];
const extern SNewApiMapping _simUIApiMapping[];
const extern SNewApiMapping _simROSApiMapping[];
const extern SNewApiMapping _simICPApiMapping[];
const extern SNewApiMapping _simOMPLApiMapping[];
const extern SNewApiMapping _simSDFApiMapping[];
const extern SNewApiMapping _simSurfRecApiMapping[];
const extern SNewApiMapping _simxApiMapping[];
// *****************************************

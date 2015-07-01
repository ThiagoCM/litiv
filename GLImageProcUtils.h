#pragma once

#define GLUTILS_IMGPROC_DEFAULT_WORKGROUP           glm::uvec2(12,8)
#define GLUTILS_IMGPROC_TEXTURE_ARRAY_SIZE          4
#define GLUTILS_IMGPROC_USE_TEXTURE_ARRAYS          0
#define GLUTILS_IMGPROC_USE_INTEGER_TEX_FORMAT      1
#define GLUTILS_IMGPROC_USE_DOUBLE_PBO_INPUT        0
#define GLUTILS_IMGPROC_USE_DOUBLE_PBO_OUTPUT       1
#define GLUTILS_IMGPROC_USE_PBO_UPDATE_REALLOC      1 // @@@@@ unused?

#include "GLShaderUtils.h"

class GLImageProcAlgo {
public:
    GLImageProcAlgo( size_t nLevels, size_t nLayers, size_t nComputeStages,
                     int nOutputType, int nDebugType, int nInputType,
                     bool bUseOutputPBOs, bool bUseDebugPBOs, bool bUseInputPBOs,
                     bool bUseTexArrays, bool bUseDisplay, bool bUseTimers, bool bUseIntegralFormat);
    virtual ~GLImageProcAlgo();

    virtual std::string getVertexShaderSource() const;
    virtual std::string getFragmentShaderSource() const;
    virtual std::string getComputeShaderSource(size_t nStage) const = 0;

    inline bool setOutputFetching(bool b) {return (m_bFetchingOutput=(b&&m_bUsingOutput));}
    inline bool setDebugFetching(bool b) {return (m_bFetchingDebug=(b&&m_bUsingDebug));}
    inline bool getIsUsingDisplay() const {return m_bUsingDisplay;}
    inline GLuint getSSBOId(size_t n) const {glAssert(n<eBufferBindingsCount); return m_anSSBO[n];}
    size_t fetchLastOutput(cv::Mat& oOutput) const;
    size_t fetchLastDebug(cv::Mat& oDebug) const;

    virtual void initialize(const cv::Mat& oInitInput, const cv::Mat& oROI);
    virtual void apply(const cv::Mat& oNextInput, bool bRebindAll=false);

    const size_t m_nLevels;
    const size_t m_nLayers;
    const size_t m_nSxSDisplayCount;
    const size_t m_nComputeStages;
    const int m_nOutputType;
    const int m_nDebugType;
    const int m_nInputType;
    const bool m_bUsingOutputPBOs;
    const bool m_bUsingDebugPBOs;
    const bool m_bUsingInputPBOs;
    const bool m_bUsingOutput;
    const bool m_bUsingDebug;
    const bool m_bUsingInput;
    const bool m_bUsingTexArrays;
    const bool m_bUsingTimers;
    const bool m_bUsingIntegralFormat;
    const glm::uvec2 m_vDefaultWorkGroupSize; // make dynamic? @@@@@
protected:
    enum eImageLayoutList {
        eImage_OutputBinding=0,
        eImage_DebugBinding,
        eImage_GTBinding, // @@@@ make custom4? set all customs at end? simplify custom tex array?
        eImage_ROIBinding,
        eImage_CustomBinding1,
        eImage_CustomBinding2,
        eImage_CustomBinding3,
        eImage_InputBinding,
        eImageBindingsCount
    };
    enum eTextureLayoutList {
        eTexture_OutputBinding=0,
        eTexture_DebugBinding,
        eTexture_GTBinding,
        eTexture_InputBinding,
        eTextureBindingsCount
    };
    enum eBufferBindingList {
        eBuffer_CustomBinding1=0,
        eBuffer_CustomBinding2,
        eBuffer_CustomBinding3,
        eBuffer_CustomBinding4,
        eBufferBindingsCount
    };
    enum eGLTimersList {
        eGLTimer_TextureUpdate=0,
        eGLTimer_ComputeDispatch,
        eGLTimer_DisplayUpdate,
        eGLTimersCount
    };
    bool m_bUsingDisplay;
    bool m_bGLInitialized;
    cv::Size m_oFrameSize;
    size_t m_nInternalFrameIdx;
    size_t m_nLastOutputInternalIdx, m_nLastDebugInternalIdx;
    bool m_bFetchingOutput, m_bFetchingDebug;
    cv::Mat m_oLastOutput, m_oLastDebug;
    size_t m_nNextLayer,m_nCurrLayer,m_nLastLayer;
    size_t m_nCurrPBO, m_nNextPBO;
    GLuint m_nGLTimers[eGLTimersCount];
    GLuint64 m_nGLTimerVals[eGLTimersCount];
    std::vector<std::unique_ptr<GLDynamicTexture2D>> m_vpInputArray;
    std::vector<std::unique_ptr<GLDynamicTexture2D>> m_vpDebugArray;
    std::vector<std::unique_ptr<GLDynamicTexture2D>> m_vpOutputArray;
    std::unique_ptr<GLDynamicTexture2DArray> m_pInputArray;
    std::unique_ptr<GLDynamicTexture2DArray> m_pDebugArray;
    std::unique_ptr<GLDynamicTexture2DArray> m_pOutputArray;
    std::vector<std::unique_ptr<GLShader>> m_vpImgProcShaders;
    std::unique_ptr<GLPixelBufferObject> m_apInputPBOs[2];
    std::unique_ptr<GLPixelBufferObject> m_apDebugPBOs[2];
    std::unique_ptr<GLPixelBufferObject> m_apOutputPBOs[2];
    std::unique_ptr<GLTexture2D> m_pROITexture;
    std::unique_ptr<GLTexture2D> m_apCustomTextures[3];
    GLScreenBillboard m_oDisplayBillboard;
    GLShader m_oDisplayShader;

    virtual void dispatch(size_t nStage, GLShader& oShader);
    static const char* getCurrTextureLayerUniformName();
    static const char* getLastTextureLayerUniformName();
    static const char* getFrameIndexUniformName();
    static std::string getFragmentShaderSource_internal( size_t nLayers, int nOutputType, int nDebugType, int nInputType,
                                                         bool bUsingOutput, bool bUsingDebug, bool bUsingInput,
                                                         bool bUsingTexArrays, bool bUsingIntegralFormat);
private:
    GLImageProcAlgo& operator=(const GLImageProcAlgo&)=delete;
    GLImageProcAlgo(const GLImageProcAlgo&)=delete;
    friend class GLEvaluatorAlgo;
    GLuint m_anSSBO[GLImageProcAlgo::eBufferBindingsCount];
};

class GLEvaluatorAlgo : public GLImageProcAlgo {
public:
    GLEvaluatorAlgo(const std::shared_ptr<GLImageProcAlgo>& pParent, size_t nTotFrameCount, size_t nCountersPerFrame, size_t nComputeStages, int nDebugType, int nGroundtruthType);
    virtual ~GLEvaluatorAlgo();
    inline GLuint getAtomicBufferId() const {return m_nAtomicBuffer;}
    const cv::Mat& getAtomicCounterBuffer();
    virtual std::string getFragmentShaderSource() const;

    virtual void initialize(const cv::Mat& oInitInput, const cv::Mat& oInitGT, const cv::Mat& oROI);
    virtual void initialize(const cv::Mat& oInitGT, const cv::Mat& oROI);
    virtual void apply(const cv::Mat& oNextInput, const cv::Mat& oNextGT, bool bRebindAll=false);
    virtual void apply(const cv::Mat& oNextGT, bool bRebindAll=false);

protected:
    const size_t m_nTotFrameCount;
    const size_t m_nAtomicBufferRangeSize;
    const size_t m_nAtomicBufferSize;
    const size_t m_nAtomicBufferMaxSize;
    size_t m_nCurrAtomicBufferSize;
    size_t m_nCurrAtomicBufferOffsetPtr;
    size_t m_nCurrAtomicBufferOffsetBlock;
    std::shared_ptr<GLImageProcAlgo> m_pParent;
private:
    cv::Mat m_oAtomicCountersQueryBuffer;
    GLuint m_nAtomicBuffer;
};

class GLImagePassThroughAlgo : public GLImageProcAlgo {
public:
    GLImagePassThroughAlgo( size_t nLayers, int nFrameType, bool bUseOutputPBOs, bool bUseInputPBOs,
                            bool bUseTexArrays, bool bUseDisplay, bool bUseTimers, bool bUseIntegralFormat);
    virtual std::string getComputeShaderSource(size_t nStage) const;
};

class BinaryMedianFilter : public GLImageProcAlgo {
public:
    // @@@@@ add support for variable kernels? per-px kernel size could be provided via image load/store
    // @@@ currently not using ROI
    // via integral image: O(n) (where n is the total image size --- does not depend on r, the kernel size)
    BinaryMedianFilter( size_t nKernelSize, size_t nBorderSize, size_t nLayers, const cv::Mat& oROI,
                        bool bUseOutputPBOs, bool bUseInputPBOs, bool bUseTexArrays,
                        bool bUseDisplay, bool bUseTimers, bool bUseIntegralFormat);
    virtual std::string getComputeShaderSource(size_t nStage) const;
    const size_t m_nKernelSize;
    const size_t m_nBorderSize;
    static const size_t m_nPPSMaxRowSize;
    static const size_t m_nTransposeBlockSize;
protected:
    std::vector<std::string> m_vsComputeShaderSources;
    std::vector<glm::uvec3> m_vvComputeShaderDispatchSizes;
    static const GLuint eImage_PPSAccumulator;
    static const GLuint eImage_PPSAccumulator_T;
    virtual void dispatch(size_t nStage, GLShader& oShader);
};

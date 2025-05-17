#pragma once
#include <string>

#include "llama.h"

class LlamaLoader
{
private:
    llama_context *ctx = nullptr;
    bool isDisposed = false;

public:
    LlamaLoader(llama_context *p_ctx);
    void free();
    static llama_model* loadModelFromPath(std::string path); 
    static LlamaLoader loadCtxFromModel(llama_model *model); 
    llama_context* getCtx();
};


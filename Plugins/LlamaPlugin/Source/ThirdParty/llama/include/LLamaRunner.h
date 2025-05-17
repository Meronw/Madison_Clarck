#pragma once
#include <vector>

#include "LlamaLoader.h"

class LLamaRunner
{

private:
    LlamaLoader loader;
    std::string prompt;
    int n_threads = 4;
    std::vector<llama_token> embeds = {};

public:

    constexpr static int DEFAULT_N_TOKENS_TO_PREDICT = 128;
    
    LLamaRunner(LlamaLoader ldr);

    void clear();
    std::string infer(bool& eos, int tokens_To_Predict = DEFAULT_N_TOKENS_TO_PREDICT);
    LLamaRunner& with_threads(int threads);
    LLamaRunner& with_prompt(std::string prompt);
};

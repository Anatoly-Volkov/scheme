#pragma once
#include "tokenizer.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

class Interpreter {

public:
    std::string Run(const std::string& input);

private:
    bool UnpackList(std::shared_ptr<Object> node, std::vector<std::shared_ptr<Object>>& res);

    bool UnpackList2(std::shared_ptr<Object> node, std::vector<std::shared_ptr<Object>>& res);

    std::shared_ptr<Object> ReadFull(const std::string& str);

    std::shared_ptr<Object> GetResult(std::shared_ptr<Object> node);

    template <typename T>
    void CheckType(std::shared_ptr<Object> node) {
        if (!Is<T>(node)) {
            throw RuntimeError("");
        }
    }
};
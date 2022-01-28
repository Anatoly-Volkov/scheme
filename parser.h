#pragma once

#include <memory>

#include "object.h"
#include "tokenizer.h"

std::shared_ptr<Object> Read(Tokenizer* tokenizer, bool from_list = false);

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer);

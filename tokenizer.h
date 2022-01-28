#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <set>
#include <cassert>
#include "error.h"

#include <iostream>

using std::cerr;
using std::endl;

// struct SyntaxError {
//     SyntaxError() {}
// };

struct SymbolToken {
    std::string name;

    SymbolToken() {
    }

    SymbolToken(const std::string& val) : name(val) {
    }

    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    }
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const {
        return true;
    }
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    }
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    ConstantToken() {
    }
    ConstantToken(int val) : value(val) {
    }

    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    }
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in) : in_(in) {
        last_ = in_->get();
        Next();
    }

    bool IsEnd() {
        return (st_ == -1);
    }

    void Next() {
        // cerr << "Call next(" << st_ << ")" <<  endl;
        // cerr << "(" << c2 << ")" << endl;

        int sgn = 1;
        while (st_ == 0) {
            // cerr << "in loop1" << endl;
            if (last_ == EOF) {
                st_ = -1;
                return;
            }
            char c = last_;
            if (std::isspace(c)) {
                last_ = in_->get();
                continue;
            }
            if (IsSymbolStarter(c)) {
                cur_ = Token{SymbolToken()};
                std::get<SymbolToken>(cur_).name += c;
                st_ = 1;
                last_ = in_->get();
                break;
            }
            if (std::isdigit(c)) {
                cur_ = Token{ConstantToken(c - '0')};
                st_ = 2;
                last_ = in_->get();
                break;
            }
            if (IsIndependent(c) || c == '+') {
                if (c == '+') {
                    cur_ = Token{SymbolToken("+")};
                    last_ = in_->get();
                    st_ = 4;
                    sgn = 1;
                    break;
                }
                if (c == '(') {
                    cur_ = Token{BracketToken::OPEN};
                }
                if (c == ')') {
                    cur_ = Token{BracketToken::CLOSE};
                }
                if (c == '.') {
                    cur_ = Token{DotToken()};
                }
                if (c == '\'') {
                    cur_ = Token{QuoteToken()};
                }
                if (c == '/') {
                    cur_ = Token{SymbolToken("/")};
                }
                last_ = in_->get();
                st_ = 3;
                break;
            }
            if (c == '-') {
                sgn = -1;
                last_ = in_->get();
                st_ = 4;
                break;
            }
            throw SyntaxError("");
        }
        if (st_ == 1) {
            while (last_ != EOF) {
                char c = last_;
                if (IsSymbolContent(c)) {
                    std::get<SymbolToken>(cur_).name += c;
                    last_ = in_->get();
                    continue;
                }
                if (std::isspace(c)) {
                    st_ = 0;
                    break;
                }
                if (IsIndependent(c)) {
                    st_ = 0;
                    break;
                }
                // cerr << "second before..." << endl;
                throw SyntaxError("");
            }
            st_ = 0;
            return;
        }
        if (st_ == 3) {
            st_ = 0;
            return;
        }
        if (st_ == 4) {
            if (last_ != EOF && std::isdigit(static_cast<char>(last_))) {
                char c = last_;
                cur_ = Token{ConstantToken((c - '0') * sgn)};
                st_ = 2;
                last_ = in_->get();
            } else {
                if (sgn == 1) {
                    cur_ = Token{SymbolToken("+")};
                } else {
                    cur_ = Token{SymbolToken("-")};
                }
                st_ = 0;
                return;
            }
        }
        if (st_ == 2) {
            while (last_ != EOF) {
                char c = last_;
                // cerr << "in loop2 (" << c << ")" << endl;
                if (std::isdigit(c)) {
                    std::get<ConstantToken>(cur_).value *= 10;
                    if (std::get<ConstantToken>(cur_).value < 0) {
                        std::get<ConstantToken>(cur_).value -= (c - '0');
                    } else {
                        std::get<ConstantToken>(cur_).value += (c - '0');
                    }
                    last_ = in_->get();
                    // cerr << last_ << endl;
                } else {
                    break;
                }
            }
            st_ = 0;
        }
        assert(st_ == 0);
    }

    Token GetToken() {
        return cur_;
    }

private:
    bool IsLetter(char c) {
        return ((c - 'a') >= 0 && (c - 'a') < 26) || ((c - 'A') >= 0 && (c - 'A') < 26);
    }

    // bool IsGood(char c) {
    //     return good_.find(c) != good_.end();
    // }

    bool IsIndependent(char c) {
        return (c == '.' || c == '(' || c == ')' || c == '\'' || c == '/');
    }

    bool IsSymbolStarter(char c) {
        if (IsLetter(c)) {
            return true;
        }
        return (c == '<') || (c == '>') || (c == '=') || (c == '*') || (c == '#');
    }

    bool IsSymbolContent(char c) {
        if (IsSymbolStarter(c)) {
            return true;
        }
        return std::isdigit(c) || (c == '?') || (c == '!') || (c == '-');
    }
    // static std::set<char> good_({'(', ')', '.', '\'', '+', '-', '<', '=', '>', '*', '#', '?',
    // '!'});
    Token cur_;
    int last_ = -1;
    int st_ = 0;
    // -1 EOF
    //  0 Searching for token

    std::istream* in_;
};
#include "parser.h"

#include <cassert>

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    // cerr << "ReadList" << endl;
    // tokenizer is currently is looking at '('
    // after that it should look at ')'

    DotToken* dot;
    BracketToken* brack;

    int state = 0;
    // 0 (
    // 1 (a b c
    // 2 (a b .
    // 3 (a b . c
    std::shared_ptr<Object> head;
    std::shared_ptr<Object> cur;

    while (true) {
        tokenizer->Next();
        if (tokenizer->IsEnd()) {
            break;
        }
        Token elem = tokenizer->GetToken();
        if (state == 0) {
            brack = std::get_if<BracketToken>(&elem);
            if (brack && *brack == BracketToken::CLOSE) {
                return nullptr;
            }
            head.reset(new Cell());
            As<Cell>(head)->GetFirst() = Read(tokenizer, true);

            As<Cell>(head)->GetSecond() = nullptr;
            cur = head;
            state = 1;
            // cerr << "here" << endl;
            continue;
        }
        if (state == 1) {
            brack = std::get_if<BracketToken>(&elem);
            if (brack && *brack == BracketToken::CLOSE) {
                return head;
            }
            dot = std::get_if<DotToken>(&elem);
            if (dot) {
                state = 2;
                continue;
            }
            (As<Cell>(cur)->GetSecond()).reset(new Cell());
            cur = As<Cell>(cur)->GetSecond();
            As<Cell>(cur)->GetFirst() = Read(tokenizer, true);
            continue;
        }
        if (state == 2) {
            As<Cell>(cur)->GetSecond() = Read(tokenizer, true);
            state = 3;
            continue;
        }
        if (state == 3) {
            brack = std::get_if<BracketToken>(&elem);
            if (brack && *brack == BracketToken::CLOSE) {
                return head;
            }
            throw SyntaxError("");
        }
    }
    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    }
}

void check_end(Tokenizer* tokenizer) {
    tokenizer->Next();
    if (!tokenizer->IsEnd()) {
        throw SyntaxError("");
    }
}

std::shared_ptr<Object> Read(Tokenizer* tokenizer, bool from_list) {
    SymbolToken* symb;
    BracketToken* brack;
    ConstantToken* numb;
    QuoteToken* quot;
    DotToken* dot;

    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    }
    Token elem = tokenizer->GetToken();
    brack = std::get_if<BracketToken>(&elem);
    if (brack) {
        if (*brack == BracketToken::OPEN) {
            std::shared_ptr<Object> res = ReadList(tokenizer);
            if (!from_list) {
                check_end(tokenizer);
            }
            return res;
        } else {
            throw SyntaxError("");
        }
    }
    numb = std::get_if<ConstantToken>(&elem);
    if (numb) {
        std::shared_ptr<Object> res(new Number(numb->value));
        if (!from_list) {
            check_end(tokenizer);
        }
        return res;
    }
    symb = std::get_if<SymbolToken>(&elem);
    if (symb) {
        std::shared_ptr<Object> res;
        if (symb->name == "#t") {
            res.reset(new Boolean(true));
        } else if (symb->name == "#f") {
            res.reset(new Boolean(false));
        } else {
            res.reset(new Symbol(symb->name));
        }
        if (!from_list) {
            check_end(tokenizer);
        }
        return res;
    }
    quot = std::get_if<QuoteToken>(&elem);
    if (quot) {
        tokenizer->Next();
        if (tokenizer->IsEnd()) {
            throw SyntaxError("");
        }
        Token next_elem = tokenizer->GetToken();
        dot = std::get_if<DotToken>(&next_elem);
        quot = std::get_if<QuoteToken>(&next_elem);
        if (dot || quot) {
            throw SyntaxError("");
        }
        brack = std::get_if<BracketToken>(&next_elem);
        std::shared_ptr<Object> content_token;
        if (brack) {
            if (*brack == BracketToken::OPEN) {
                content_token = ReadList(tokenizer);
                if (!from_list) {
                    check_end(tokenizer);
                }
            } else {
                throw SyntaxError("");
            }
        } else {
            symb = std::get_if<SymbolToken>(&next_elem);
            numb = std::get_if<ConstantToken>(&next_elem);
            if (symb) {
                content_token.reset(new Symbol(symb->name));
                if (!from_list) {
                    check_end(tokenizer);
                }
            } else if (numb) {
                content_token.reset(new Number(numb->value));
                if (!from_list) {
                    check_end(tokenizer);
                }
            } else {
                throw SyntaxError("");
            }
        }

        std::shared_ptr<Object> quote_token(new Symbol("quote"));
        std::shared_ptr<Object> res1(new Cell());
        std::shared_ptr<Object> res2(new Cell());
        As<Cell>(res1)->GetFirst() = quote_token;
        As<Cell>(res1)->GetSecond() = res2;
        As<Cell>(res2)->GetFirst() = content_token;
        return res1;

    }
    throw SyntaxError("");
}
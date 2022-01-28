#include "scheme.h"
#include <algorithm>
#include <math.h>

std::string Interpreter::Run(const std::string& input) {
    auto node = ReadFull(input);
    auto res = GetResult(node);
    if (!res) {
        return "()";
    }
    return res->Print();
}

int64_t Fold(int64_t x, int64_t y, std::string op) {
    if (op == "+") {
        return x + y;
    }
    if (op == "*") {
        return x * y;
    }
    if (op == "/") {
        return x / y;
    }
    if (op == "-") {
        return x - y;
    }
    if (op == "max") {
        return std::max(x, y);
    }
    if (op == "min") {
        return std::min(x, y);
    }
}

bool CheckPredicate(int64_t x, int64_t y, std::string c) {
    if (c == "=") {
        return x == y;
    }
    if (c == "<") {
        return x < y;
    }
    if (c == ">") {
        return x > y;
    }
    if (c == "<=") {
        return x <= y;
    }
    if (c == ">=") {
        return x >= y;
    }
}

bool FoldBool(bool a, bool b, std::string op) {
    if (op == "and") {
        return a && b;
    }
    if (op == "or") {
        return a || b;
    }
}

bool Interpreter::UnpackList(std::shared_ptr<Object> node,
                             std::vector<std::shared_ptr<Object>>& res) {
    // returns true if it is proper list
    Cell* ptr = As<Cell>(node).get();
    res.clear();
    do {
        if (!(ptr->GetSecond())) {
            return true;
        }
        if (!Is<Cell>(ptr->GetSecond())) {
            auto last = GetResult(ptr->GetSecond());
            res.push_back(last);
            return false;
        }
        ptr = As<Cell>(ptr->GetSecond()).get();
        auto first = GetResult(ptr->GetFirst());
        res.push_back(first);
    } while (true);
}

bool Interpreter::UnpackList2(std::shared_ptr<Object> node,
                              std::vector<std::shared_ptr<Object>>& res) {
    // returns true if it is proper list
    Cell* ptr = As<Cell>(node).get();
    res.clear();
    while (true) {
        auto first = GetResult(ptr->GetFirst());
        res.push_back(first);
        if (!(ptr->GetSecond())) {
            return true;
        }
        if (!Is<Cell>(ptr->GetSecond())) {
            auto last = GetResult(ptr->GetSecond());
            res.push_back(last);
            return false;
        }
        ptr = As<Cell>(ptr->GetSecond()).get();
    }
}

std::shared_ptr<Object> Interpreter::ReadFull(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};

    std::shared_ptr<Object> obj = Read(&tokenizer);
    return obj;
}

std::shared_ptr<Object> Interpreter::GetResult(std::shared_ptr<Object> node) {
    if (!node || Is<Symbol>(node)) {
        throw RuntimeError("");
    }
    if (Is<Number>(node) || Is<Boolean>(node)) {
        return node;
    }
    assert(Is<Cell>(node));
    auto com = (As<Cell>(node))->GetFirst();
    if (!Is<Symbol>(com)) {
        throw RuntimeError("");
    }

    std::shared_ptr<Object> res;
    std::string name = As<Symbol>(com)->GetName();

    if (name == "quote") {
        return As<Cell>((As<Cell>(node))->GetSecond())->GetFirst();
    }

    if (name == "and" || name == "or") {
        bool init = true;
        if (name == "or") {
            init = false;
        }
        bool cur = init;
        Cell* ptr = As<Cell>(node).get();
        if (!(ptr->GetSecond())) {
            res.reset(new Boolean(init));
            return res;
        }
        do {
            if (!(ptr->GetSecond())) {
                break;
            }
            if (!Is<Cell>(ptr->GetSecond())) {
                std::shared_ptr<Object> last = GetResult(ptr->GetSecond());
                if (Is<Boolean>(last)) {
                    bool val = (As<Boolean>(last))->GetValue();
                    cur = FoldBool(cur, val, name);
                }

                break;
            }
            ptr = As<Cell>(ptr->GetSecond()).get();
            std::shared_ptr<Object> first = GetResult(ptr->GetFirst());
            if (Is<Boolean>(first)) {
                bool val = (As<Boolean>(first))->GetValue();
                cur = FoldBool(cur, val, name);
            }
        } while (cur == init);
        if (cur != init) {
            res.reset(new Boolean(cur));
        } else {
            std::shared_ptr<Object> val;
            if (!(ptr->GetSecond())) {
                val = ptr->GetFirst();
            } else {
                val = ptr->GetSecond();
            }
            return GetResult(val);
        }
        return res;
    }

    std::vector<std::shared_ptr<Object>> args;
    UnpackList(node, args);

    // Arithmetic

    if (name == "number?") {
        if (args.size() != 1) {
            throw RuntimeError("");
        }
        bool ans = Is<Number>(args[0]);
        res.reset(new Boolean(ans));
        return res;
    }

    if (name == "+" || name == "*" || name == "-" || name == "/" || name == "min" ||
        name == "max") {
        int64_t ans = 0;
        size_t st = 0;
        if (name == "*") {
            ans = 1;
        }
        if (name == "-" || name == "/" || name == "min" || name == "max") {
            if (args.empty()) {
                throw RuntimeError("");
            }
            st = 1;
            CheckType<Number>(args[0]);
            ans = As<Number>(args[0])->GetValue();
        }
        for (size_t i = st; i < args.size(); ++i) {
            CheckType<Number>(args[i]);
            ans = Fold(ans, As<Number>(args[i])->GetValue(), name);
        }
        res.reset(new Number(ans));
        return res;
    }
    if (name == "<" || name == ">" || name == "=" || name == "<=" || name == ">=") {
        if (args.size() < 2) {
            res.reset(new Boolean(true));
            return res;
        }
        CheckType<Number>(args[0]);
        bool flag = true;
        for (size_t i = 1; i < args.size(); ++i) {
            CheckType<Number>(args[i]);

            flag = flag & CheckPredicate(As<Number>(args[i - 1])->GetValue(),
                                         As<Number>(args[i])->GetValue(), name);
            if (!flag) {
                break;
            }
        }
        res.reset(new Boolean(flag));
        return res;
    }
    if (name == "abs") {
        if (args.size() != 1) {
            throw RuntimeError("");
        }
        CheckType<Number>(args[0]);
        res.reset(new Number(abs(As<Number>(args[0])->GetValue())));
        return res;
    }

    if (name == "boolean?") {
        if (args.size() != 1) {
            throw RuntimeError("");
        }
        bool flag = false;
        if (Is<Boolean>(args[0])) {
            flag = true;
        }
        res.reset(new Boolean(flag));
        return res;
    }

    if (name == "not") {
        if (args.size() != 1) {
            throw RuntimeError("");
        }
        bool flag = false;
        if (Is<Boolean>(args[0])) {
            if (!(As<Boolean>(args[0])->GetValue())) {
                flag = true;
            }
        }
        res.reset(new Boolean(flag));
        return res;
    }

    // List functions

    if (name == "list") {
        std::shared_ptr<Object> cur;
        res.reset(new Cell());
        if (args.empty()) {
            return nullptr;
        }
        cur = res;
        As<Cell>(res)->GetFirst() = args[0];
        for (size_t i = 1; i < args.size(); ++i) {
            (As<Cell>(cur)->GetSecond()).reset(new Cell());
            cur = As<Cell>(cur)->GetSecond();
            As<Cell>(cur)->GetFirst() = args[i];
        }
        return res;
    }

    if (name == "cons") {
        if (args.size() != 2) {
            throw RuntimeError("");
        }
        res.reset(new Cell());
        As<Cell>(res)->GetFirst() = args[0];
        As<Cell>(res)->GetSecond() = args[1];
        return res;
    }

    if (args.empty()) {
        throw RuntimeError("");
    }

    if (name == "pair?" || name == "list?" || name == "null?") {
        if (args.size() != 1) {
            throw RuntimeError("");
        }
        if (!args[0] && name == "list?") {
            res.reset(new Boolean(true));
            return res;
        }
        if (name == "null?") {
            bool flag = true;
            if (args[0]) {
                flag = false;
            }
            res.reset(new Boolean(flag));
            return res;
        }
        if (!Is<Cell>(args[0])) {
            res.reset(new Boolean(false));
            return res;
        }
        std::vector<std::shared_ptr<Object>> vals;
        bool is_proper_list = UnpackList2(args[0], vals);
        if (name == "list?") {
            res.reset(new Boolean(is_proper_list));
            return res;
        }
        if (name == "pair?") {
            // cerr << vals.size() << " " << args.size() << endl;
            // cerr << args[0]->Print() << endl;
            // cerr << vals[0]->Print() << endl;
            bool flag = (vals.size() == 2);
            res.reset(new Boolean(flag));
            return res;
        }
        assert(0);
    }
    if (name == "car") {
        if (args.size() != 1) {
            throw RuntimeError("");
        }
        if (!Is<Cell>(args[0])) {
            throw RuntimeError("");
        }
        res = As<Cell>(args[0])->GetFirst();
        return res;
    }
    if (name == "cdr") {
        if (args.size() != 1) {
            throw RuntimeError("");
        }
        if (!Is<Cell>(args[0])) {
            throw RuntimeError("");
        }
        return As<Cell>(args[0])->GetSecond();
    }
    if (name == "list-ref") {
        if (args.size() != 2) {
            throw RuntimeError("");
        }
        if (!Is<Cell>(args[0]) || !Is<Number>(args[1])) {
            throw RuntimeError("");
        }
        int ind = As<Number>(args[1])->GetValue();
        std::vector<std::shared_ptr<Object>> vals;
        UnpackList2(args[0], vals);
        if ((int)vals.size() <= ind) {
            throw RuntimeError("");
        }
        res = vals[ind];
        return res;
    }
    if (name == "list-tail") {
        if (args.size() != 2) {
            throw RuntimeError("");
        }
        if (!Is<Cell>(args[0]) || !Is<Number>(args[1])) {
            throw RuntimeError("");
        }
        int ind = As<Number>(args[1])->GetValue();
        if (ind < 0) {
            throw RuntimeError("");
        }

        std::vector<std::shared_ptr<Object>> vals;
        UnpackList2(args[0], vals);
        if (ind > vals.size()) {
            throw RuntimeError("");
        }
        if (ind == vals.size()) {
            return nullptr;
        }
        std::shared_ptr<Object> cur;
        res.reset(new Cell());
        cur = res;
        As<Cell>(res)->GetFirst() = vals[ind];
        for (size_t i = ind + 1; i < vals.size(); ++i) {
            (As<Cell>(cur)->GetSecond()).reset(new Cell());
            cur = As<Cell>(cur)->GetSecond();
            As<Cell>(cur)->GetFirst() = vals[i];
        }
        return res;
    }
    throw RuntimeError("");
}
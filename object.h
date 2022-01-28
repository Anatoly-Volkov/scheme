#pragma once

#include <memory>
#include <string>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    virtual std::string Print() = 0;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    auto res = dynamic_cast<T*>(obj.get());
    return res != nullptr;
}

class Number : public Object {

public:
    Number() {
    }
    Number(int64_t numb) : numb_(numb) {
    }
    int64_t GetValue() const {
        return numb_;
    }
    std::string Print() override {
        return std::to_string(numb_);
    }

private:
    int64_t numb_;
};

class Boolean : public Object {

public:
    Boolean() {
    }
    Boolean(bool val) : val_(val) {}

    bool GetValue() const {
        return val_;
    }

    std::string Print() override {
        if (val_) {
            return "#t";
        } else {
            return "#f";
        }
    }

private:
    bool val_;

};

class Symbol : public Object {
public:
    Symbol() {
    }
    Symbol(const std::string& name) : name_(name) {
    }
    const std::string& GetName() const {
        return name_;
    }

    std::string Print() override {
        return name_;
    }

private:
    std::string name_;
};

class Cell : public Object {
public:
    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }

    std::shared_ptr<Object>& GetFirst() {
        return first_;
    }

    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }

    std::shared_ptr<Object>& GetSecond() {
        return second_;
    }

    std::string Print() override {
        std::string res = "(";
        const Cell* ptr = this;
        bool is_first = true;
        while (ptr) {
            if (!is_first) {
                res += " ";
            }
            is_first = false;
            if (ptr->first_) {
                res += ptr->first_->Print();
            } else {
                res += "()";
            }
            if (ptr->second_) {
                if (!Is<Cell>(ptr->second_)) {
                    res += " . ";
                    res += ptr->second_->Print();
                    break;
                }
                ptr = As<Cell>(ptr->second_).get();
            } else {
                break;
            }
        }
        res += ")";
        return res;
    }

private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
};


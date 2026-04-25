#pragma once

#include <cctype>
#include <istream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace nlohmann {

class json {
public:
    using array_t = std::vector<json>;
    using object_t = std::map<std::string, json>;

    json() = default;
    explicit json(array_t value) : type_(Type::Array), array_(std::move(value)) {}
    explicit json(object_t value) : type_(Type::Object), object_(std::move(value)) {}
    explicit json(std::string value) : type_(Type::String), string_(std::move(value)) {}
    explicit json(double value) : type_(Type::Number), number_(value) {}
    explicit json(bool value) : type_(Type::Bool), bool_(value) {}

    bool is_array() const { return type_ == Type::Array; }
    std::size_t size() const
    {
        if (type_ == Type::Array) {
            return array_.size();
        }
        if (type_ == Type::Object) {
            return object_.size();
        }
        return 0;
    }

    const json& at(const std::string& key) const
    {
        if (type_ != Type::Object) {
            throw std::runtime_error("json value is not an object");
        }
        const auto it = object_.find(key);
        if (it == object_.end()) {
            throw std::out_of_range("missing json key: " + key);
        }
        return it->second;
    }

    array_t::const_iterator begin() const
    {
        ensureArray();
        return array_.begin();
    }

    array_t::const_iterator end() const
    {
        ensureArray();
        return array_.end();
    }

    template <typename T>
    T value(const std::string& key, const T& defaultValue) const
    {
        if (type_ != Type::Object) {
            return defaultValue;
        }
        const auto it = object_.find(key);
        if (it == object_.end()) {
            return defaultValue;
        }
        return it->second.get<T>(defaultValue);
    }

    std::string value(const std::string& key, const char* defaultValue) const
    {
        return value<std::string>(key, std::string(defaultValue));
    }

    template <typename T>
    T get(const T& defaultValue) const
    {
        if constexpr (std::is_same_v<T, std::string>) {
            return type_ == Type::String ? string_ : defaultValue;
        } else if constexpr (std::is_same_v<T, bool>) {
            return type_ == Type::Bool ? bool_ : defaultValue;
        } else if constexpr (std::is_integral_v<T>) {
            return type_ == Type::Number ? static_cast<T>(number_) : defaultValue;
        } else if constexpr (std::is_floating_point_v<T>) {
            return type_ == Type::Number ? static_cast<T>(number_) : defaultValue;
        } else {
            return defaultValue;
        }
    }

    friend std::istream& operator>>(std::istream& input, json& output)
    {
        std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        Parser parser(text);
        output = parser.parse();
        return input;
    }

private:
    enum class Type {
        Null,
        Array,
        Object,
        String,
        Number,
        Bool
    };

    class Parser {
    public:
        explicit Parser(std::string text) : text_(std::move(text)) {}

        json parse()
        {
            skipBom();
            skipWhitespace();
            json value = parseValue();
            skipWhitespace();
            return value;
        }

    private:
        std::string text_;
        std::size_t pos_ = 0;

        void skipBom()
        {
            if (text_.size() >= 3 &&
                static_cast<unsigned char>(text_[0]) == 0xEF &&
                static_cast<unsigned char>(text_[1]) == 0xBB &&
                static_cast<unsigned char>(text_[2]) == 0xBF) {
                pos_ = 3;
            }
        }

        json parseValue()
        {
            skipWhitespace();
            if (pos_ >= text_.size()) {
                throw std::runtime_error("unexpected end of json");
            }
            const char ch = text_[pos_];
            if (ch == '{') {
                return parseObject();
            }
            if (ch == '[') {
                return parseArray();
            }
            if (ch == '"') {
                return json(parseString());
            }
            if (ch == 't') {
                consumeLiteral("true");
                return json(true);
            }
            if (ch == 'f') {
                consumeLiteral("false");
                return json(false);
            }
            if (ch == 'n') {
                consumeLiteral("null");
                return json();
            }
            return json(parseNumber());
        }

        json parseObject()
        {
            expect('{');
            object_t object;
            skipWhitespace();
            if (peek('}')) {
                expect('}');
                return json(std::move(object));
            }
            while (true) {
                const auto key = parseString();
                skipWhitespace();
                expect(':');
                object[key] = parseValue();
                skipWhitespace();
                if (peek('}')) {
                    expect('}');
                    break;
                }
                expect(',');
            }
            return json(std::move(object));
        }

        json parseArray()
        {
            expect('[');
            array_t array;
            skipWhitespace();
            if (peek(']')) {
                expect(']');
                return json(std::move(array));
            }
            while (true) {
                array.push_back(parseValue());
                skipWhitespace();
                if (peek(']')) {
                    expect(']');
                    break;
                }
                expect(',');
            }
            return json(std::move(array));
        }

        std::string parseString()
        {
            expect('"');
            std::string result;
            while (pos_ < text_.size()) {
                char ch = text_[pos_++];
                if (ch == '"') {
                    return result;
                }
                if (ch == '\\') {
                    if (pos_ >= text_.size()) {
                        throw std::runtime_error("bad json escape");
                    }
                    const char escaped = text_[pos_++];
                    switch (escaped) {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    case 'u':
                        result += "\\u";
                        for (int i = 0; i < 4 && pos_ < text_.size(); ++i) {
                            result.push_back(text_[pos_++]);
                        }
                        break;
                    default:
                        result.push_back(escaped);
                        break;
                    }
                } else {
                    result.push_back(ch);
                }
            }
            throw std::runtime_error("unterminated json string");
        }

        double parseNumber()
        {
            const std::size_t start = pos_;
            if (peek('-')) {
                ++pos_;
            }
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                ++pos_;
            }
            if (peek('.')) {
                ++pos_;
                while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                    ++pos_;
                }
            }
            if (peek('e') || peek('E')) {
                ++pos_;
                if (peek('+') || peek('-')) {
                    ++pos_;
                }
                while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                    ++pos_;
                }
            }
            return std::stod(text_.substr(start, pos_ - start));
        }

        void consumeLiteral(const char* literal)
        {
            while (*literal) {
                expect(*literal++);
            }
        }

        void skipWhitespace()
        {
            while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) {
                ++pos_;
            }
        }

        bool peek(char ch) const
        {
            return pos_ < text_.size() && text_[pos_] == ch;
        }

        void expect(char ch)
        {
            skipWhitespace();
            if (pos_ >= text_.size() || text_[pos_] != ch) {
                throw std::runtime_error("unexpected json token");
            }
            ++pos_;
        }
    };

    void ensureArray() const
    {
        if (type_ != Type::Array) {
            throw std::runtime_error("json value is not an array");
        }
    }

    Type type_ = Type::Null;
    array_t array_;
    object_t object_;
    std::string string_;
    double number_ = 0.0;
    bool bool_ = false;
};

} // namespace nlohmann

export module TextHolder;

import <string>;
import <cstring>;
import <cstdlib>;

export class TextHolder {
public:
    TextHolder() : textLength(0), alignedTextLength(0xF), pText(nullptr) {}

    TextHolder(const TextHolder& other) : TextHolder() {
        copyFrom(other);
    }

    explicit TextHolder(const std::string& str) : TextHolder() {
        setText(str);
    }

    ~TextHolder() {
        deleteText();
    }

    TextHolder& operator=(const TextHolder& other) {
        if (this != &other) {
            deleteText();
            copyFrom(other);
        }
        return *this;
    }

    char* getText() const {
        return (alignedTextLength < 16) ? const_cast<char*>(inlineText) : pText;
    }

    size_t getTextLength() const {
        return textLength;
    }

    void setText(const std::string& str) {
        deleteText();
        textLength = str.size();
        alignedTextLength = textLength | 0xF;
        if (textLength < 16) {
            memcpy(inlineText, str.c_str(), textLength);
            inlineText[textLength] = '\0';
        }
        else {
            pText = reinterpret_cast<char*>(malloc(textLength + 1));
            if (pText != nullptr) {
                memcpy(pText, str.c_str(), textLength);
                pText[textLength] = '\0';
            }
        }
    }

    void resetWithoutDelete() {
        memset(this, 0, sizeof(TextHolder));
    }

protected:
    void copyFrom(const TextHolder& other) {
        textLength = other.textLength;
        alignedTextLength = other.alignedTextLength;
        if (other.textLength < 16) {
            memcpy(inlineText, other.inlineText, 16);
        }
        else {
            pText = reinterpret_cast<char*>(malloc(other.textLength + 1));
            if (pText != nullptr) {
                memcpy(pText, other.pText, other.textLength);
                pText[other.textLength] = '\0';
            }
        }
    }

    void deleteText() {
        if (alignedTextLength >= 16 && pText != nullptr) {
            free(pText);
            pText = nullptr;
        }
    }

private:
    union {
        char inlineText[16];
        char* pText;
    };

    size_t textLength;
    size_t alignedTextLength;
};

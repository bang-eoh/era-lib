#ifndef INC_ERA_FLASH_ESP32_HPP_
#define INC_ERA_FLASH_ESP32_HPP_

#include <SPIFFS.h>
#include <Preferences.h>

class ERaFlash
{
    const char* TAG = "Flash";
public:
    ERaFlash()
        : initialized(false)
    {}
    ~ERaFlash()
    {}

    void begin();
    void end();
    void beginRead(const char* filename);
    char* readLine();
    void endRead();
    void beginWrite(const char* filename);
    void writeLine(const char* buf);
    void endWrite();
    char* readFlash(const char* filename);
    size_t readFlash(const char* key, void* buf, size_t maxLen);
    void writeFlash(const char* filename, const char* buf);
    size_t writeFlash(const char* key, const void* value, size_t len);

protected:
private:
    File file;
    Preferences preferences;
    bool initialized;
};

inline
void ERaFlash::begin() {
    if (this->initialized) {
        return;
    }
    this->end();
    if (!SPIFFS.begin(true)) {
        ERA_LOG(TAG, ERA_PSTR("ERa flash init FAILED"));
        return;
    }
    this->preferences.end();
    if (!this->preferences.begin("era", false)) {
        ERA_LOG(TAG, ERA_PSTR("ERa flash init FAILED"));
        return;
    }
    this->initialized = true;
}

inline
void ERaFlash::end() {
    SPIFFS.end();
    this->initialized = false;
}

inline
void ERaFlash::beginRead(const char* filename) {
    this->endRead();
    this->file = SPIFFS.open(filename, FILE_READ);
}

inline
char* ERaFlash::readLine() {
    if (!this->file) {
        return nullptr;
    }
    int c {0};
    size_t pos {0};
    size_t size = 1024;
    char* buffer = (char*)ERA_MALLOC(size);
    if (buffer == nullptr) {
        return nullptr;
    }
    do {
        c = this->file.read();
        if ((c != EOF) && (c != '\n')) {
            buffer[pos++] = (char)c;
        }
        if (pos >= size - 1) {
            size += size;
            char* copy = (char*)ERA_REALLOC(buffer, size);
            if (copy == nullptr) {
                free(buffer);
                buffer = nullptr;
                return nullptr;
            }
            buffer = copy;
        }
    } while ((c != EOF) && (c != '\n'));
    buffer[pos] = 0;
    if (!pos) {
        free(buffer);
        buffer = nullptr;
    }
    return buffer;
}

inline
void ERaFlash::endRead() {
    if (!this->file) {
        return;
    }
    this->file.close();
}

inline
void ERaFlash::beginWrite(const char* filename) {
    this->endWrite();
    this->file =  SPIFFS.open(filename, FILE_WRITE);
}

inline
void ERaFlash::writeLine(const char* buf) {
    if (!this->file) {
        return;
    }
    this->file.printf("%s\n", buf);
}

inline
void ERaFlash::endWrite() {
    if (!this->file) {
        return;
    }
    this->file.close();
}

inline
char* ERaFlash::readFlash(const char* filename) {
    File _file = SPIFFS.open(filename, FILE_READ);
    if (!_file) {
        return nullptr;
    }
    size_t size = _file.size();
    if (!size) {
        _file.close();
        return nullptr;
    }
    char* buf = (char*)ERA_MALLOC(size + 1);
    if (buf == nullptr) {
        _file.close();
        return nullptr;
    }
    buf[size] = '\0';
    _file.readBytes(buf, size);
    _file.close();
    return buf;
}

inline
size_t ERaFlash::readFlash(const char* key, void* buf, size_t maxLen) {
    return preferences.getBytes(key, buf, maxLen);
}

inline
void ERaFlash::writeFlash(const char* filename, const char* buf) {
    if (buf == nullptr) {
        return;
    }
    File _file = SPIFFS.open(filename, FILE_WRITE);
    if (!_file) {
        return;
    }
    _file.print(buf);
    _file.close();
}

inline
size_t ERaFlash::writeFlash(const char* key, const void* value, size_t len) {
    return preferences.putBytes(key, value, len);
}

#endif /* INC_ERA_FLASH_ESP32_HPP_ */

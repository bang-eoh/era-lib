#ifndef INC_ERA_PROPERTY_HPP_
#define INC_ERA_PROPERTY_HPP_

#include <ERa/ERaDefine.hpp>
#include <Utility/ERaUtility.hpp>
#include <ERa/ERaReport.hpp>
#include <ERa/ERaParam.hpp>
#include "types/WrapperTypes.hpp"

#if defined(__has_include) &&       \
    __has_include(<functional>) &&  \
    !defined(ERA_IGNORE_STD_FUNCTIONAL_STRING)
    #include <functional>
    #define PROPERTY_HAS_FUNCTIONAL_H
#endif

#if !defined(VIRTUAL)
    #define VIRTUAL             0xFF
#endif

#define addProperty(p, ...) addPropertyReal(ERA_F(#p), p, __VA_ARGS__)

enum PermissionT {
    PERMISSION_READ = 1,
    PERMISSION_WRITE = 2,
    PERMISSION_READ_WRITE = PERMISSION_READ | PERMISSION_WRITE
};

template <class Api>
class ERaProperty
{
#if defined(PROPERTY_HAS_FUNCTIONAL_H)
    typedef std::function<void(void)> PropertyCallback_t;
	typedef std::function<void(void*)> ReportPropertyCallback_t;
#else
    typedef void (*PropertyCallback_t)(void);
	typedef void (*ReportPropertyCallback_t)(void*);
#endif

	const static int MAX_PROPERTYS = ERA_MAX_PROPERTY;
    typedef struct __Property_t {
        ERaProperty::PropertyCallback_t callback;
        ERaReport::iterator report;
        unsigned long prevMillis;
        PermissionT permission;
        WrapperBase* value;
        ERaParam id;
    } Property_t;

public:
    class iterator
    {
    public:
        iterator()
            : prop(nullptr)
            , pProp(nullptr)
        {}
        iterator(ERaProperty* _prop, Property_t* _pProp)
            : prop(_prop)
            , pProp(_pProp)
        {}

		bool isValid() const {
			return ((this->prop != nullptr) && (this->pProp != nullptr));
		}

        operator Property_t*() const {
            return this->pProp;
        }

		operator bool() const {
			return this->isValid();
		}

        iterator& onUpdate(ERaProperty::PropertyCallback_t cb) {
            if (this->isValid()) {
                this->prop->onUpdate(this->pProp, cb);
            }
            return *this;
        }

        iterator& publishEvery(unsigned long interval = 1000) {
            if (this->isValid()) {
                this->prop->publishEvery(this->pProp, interval);
            }
            return *this;
        }

        iterator& publishOnChange(float minChange = 1.0f,
                                unsigned long minInterval = 1000,
                                unsigned long maxInterval = 60000) {
            if (this->isValid()) {
                this->prop->publishOnChange(this->pProp, minChange, minInterval, maxInterval);
            }
            return *this;
        }

    protected:
    private:
		void invalidate() {
			this->prop = nullptr;
			this->pProp = nullptr;
		}

        ERaProperty* prop;
        Property_t* pProp;
    };

    ERaProperty()
        : numProperty(0)
        , timeout(100L)
    {}
    ~ERaProperty()
    {}

    iterator addPropertyVirtual(uint8_t pin, bool& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperBool(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, int& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperInt(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, unsigned int& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedInt(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperLong(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, unsigned long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedLong(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, long long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperLongLong(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, unsigned long long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedLongLong(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, float& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperFloat(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, double& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperDouble(value);
        return iterator(this, this->setupProperty(pin, wrapper, permission));
    }

    iterator addPropertyVirtual(uint8_t pin, WrapperBase& value, PermissionT const permission) {
        return iterator(this, this->setupProperty(pin, &value, permission));
    }

    iterator addPropertyReal(const char* id, bool& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperBool(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, int& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperInt(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, unsigned int& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedInt(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperLong(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, unsigned long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedLong(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, long long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperLongLong(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, unsigned long long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedLongLong(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, float& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperFloat(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, double& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperDouble(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const char* id, WrapperBase& value, PermissionT const permission) {
        return iterator(this, this->setupProperty(id, &value, permission));
    }

#if defined(ERA_HAS_PROGMEM)
    iterator addPropertyReal(const __FlashStringHelper* id, bool& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperBool(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, int& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperInt(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, unsigned int& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedInt(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperLong(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, unsigned long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedLong(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, long long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperLongLong(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, unsigned long long& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperUnsignedLongLong(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, float& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperFloat(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, double& value, PermissionT const permission) {
        WrapperBase* wrapper = new WrapperDouble(value);
        return iterator(this, this->setupProperty(id, wrapper, permission));
    }

    iterator addPropertyReal(const __FlashStringHelper* id, WrapperBase& value, PermissionT const permission) {
        return iterator(this, this->setupProperty(id, &value, permission));
    }
#endif

protected:
    void run();
    void handler(uint8_t pin, const ERaParam& param);
    void handler(const char* id, const ERaParam& param);

private:
    void updateValue(const Property_t* pProp);
    Property_t* setupProperty(uint8_t pin, WrapperBase* value, PermissionT const permission);
    Property_t* setupProperty(const char* id, WrapperBase* value, PermissionT const permission);
#if defined(ERA_HAS_PROGMEM)
    Property_t* setupProperty(const __FlashStringHelper* id, WrapperBase* value, PermissionT const permission);
#endif

    void getValue(Property_t* pProp, const ERaParam& param);
    bool onUpdate(Property_t* pProp, ERaProperty::PropertyCallback_t cb);
    bool publishEvery(Property_t* pProp, unsigned long interval = 1000);
    bool publishOnChange(Property_t* pProp, float minChange = 1.0f,
                        unsigned long minInterval = 1000,
                        unsigned long maxInterval = 60000);
    bool isPropertyFree();

    void onCallbackVirtual(const Property_t* const pProp);
    void onCallbackReal(const Property_t* const pProp);
    void onCallback(void* args);
#if !defined(PROPERTY_HAS_FUNCTIONAL_H)
    static void _onCallback(void* args);
#endif

    bool isValidProperty(const Property_t* pProp) {
        if (pProp == nullptr) {
            return false;
        }
        return true;
    }

    bool getFlag(uint8_t flags, uint8_t mask) {
        return (flags & mask) == mask;
    }

	inline
	const Api& thisApi() const {
		return static_cast<const Api&>(*this);
	}

	inline
	Api& thisApi() {
		return static_cast<Api&>(*this);
	}

#if defined(PROPERTY_HAS_FUNCTIONAL_H)
    ReportPropertyCallback_t propertyCb = [=](void* args) {
		this->onCallback(args);
	};
#else
    ReportPropertyCallback_t propertyCb = [](void* args) {
		ERaProperty::_onCallback(args);
	};
#endif

    ERaList<Property_t*> ERaProp;
	ERaReport ERaPropRp;
	unsigned int numProperty;
    unsigned long timeout;
};

template<class Api>
void ERaProperty<Api>::run() {
	unsigned long currentMillis = ERaMillis();
    const typename ERaList<Property_t*>::iterator* e = this->ERaProp.end();
    for (typename ERaList<Property_t*>::iterator* it = this->ERaProp.begin(); it != e; it = it->getNext()) {
        Property_t* pProp = it->get();
        if (!this->isValidProperty(pProp)) {
            continue;
        }
        if (currentMillis - pProp->prevMillis < this->timeout) {
            continue;
        }
        unsigned long skipTimes = (currentMillis - pProp->prevMillis) / this->timeout;
        // update time
        pProp->prevMillis += this->timeout * skipTimes;
        // update value
        if (!this->getFlag(pProp->permission, PermissionT::PERMISSION_READ)) {
            continue;
        }
        if (pProp->value == nullptr) {
            continue;
        }
        this->updateValue(pProp);
    }

    this->ERaPropRp.run();
}

template<class Api>
void ERaProperty<Api>::updateValue(const Property_t* pProp) {
    switch (pProp->value->getType()) {
        case WrapperTypeT::WRAPPER_TYPE_BOOL:
            pProp->report.updateReport(pProp->value->getBool(), false, false);
            break;
        case WrapperTypeT::WRAPPER_TYPE_INT:
            pProp->report.updateReport(pProp->value->getInt(), false, false);
            break;
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_INT:
            pProp->report.updateReport(pProp->value->getUnsignedInt(), false, false);
            break;
        case WrapperTypeT::WRAPPER_TYPE_LONG:
            pProp->report.updateReport(pProp->value->getLong(), false, false);
            break;
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_LONG:
            pProp->report.updateReport(pProp->value->getUnsignedLong(), false, false);
            break;
        case WrapperTypeT::WRAPPER_TYPE_LONG_LONG:
            pProp->report.updateReport(pProp->value->getLongLong(), false, false);
            break;
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_LONG_LONG:
            pProp->report.updateReport(pProp->value->getUnsignedLongLong(), false, false);
            break;
        case WrapperTypeT::WRAPPER_TYPE_FLOAT:
        case WrapperTypeT::WRAPPER_TYPE_DOUBLE:
            pProp->report.updateReport(pProp->value->getFloat(), false, false);
            break;
        case WrapperTypeT::WRAPPER_TYPE_STRING:
            if (pProp->value->isUpdated()) {
                pProp->report();
            }
            break;
        default:
            break;
    }
}

template<class Api>
void ERaProperty<Api>::handler(uint8_t pin, const ERaParam& param) {
    const typename ERaList<Property_t*>::iterator* e = this->ERaProp.end();
    for (typename ERaList<Property_t*>::iterator* it = this->ERaProp.begin(); it != e; it = it->getNext()) {
        Property_t* pProp = it->get();
        if (this->isValidProperty(pProp)) {
            if (pProp->id == pin) {
                this->getValue(pProp, param);
            }
        }
    }
}

template<class Api>
void ERaProperty<Api>::handler(const char* id, const ERaParam& param) {
    const typename ERaList<Property_t*>::iterator* e = this->ERaProp.end();
    for (typename ERaList<Property_t*>::iterator* it = this->ERaProp.begin(); it != e; it = it->getNext()) {
        Property_t* pProp = it->get();
        if (this->isValidProperty(pProp)) {
            if (pProp->id == id) {
                this->getValue(pProp, param);
            }
        }
    }
}

template<class Api>
void ERaProperty<Api>::getValue(Property_t* pProp, const ERaParam& param) {
    if (pProp == nullptr) {
        return;
    }
    if (pProp->value == nullptr) {
        return;
    }
    if (!this->getFlag(pProp->permission, PermissionT::PERMISSION_WRITE)) {
        return;
    }

    switch (pProp->value->getType()) {
        case WrapperTypeT::WRAPPER_TYPE_BOOL:
        case WrapperTypeT::WRAPPER_TYPE_INT:
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_INT:
            if (param.isNumber()) {
                *pProp->value = param.getInt();
                pProp->report.updateReport(param.getInt());
            }
            break;
        case WrapperTypeT::WRAPPER_TYPE_LONG:
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_LONG:
        case WrapperTypeT::WRAPPER_TYPE_LONG_LONG:
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_LONG_LONG:
        case WrapperTypeT::WRAPPER_TYPE_FLOAT:
        case WrapperTypeT::WRAPPER_TYPE_DOUBLE:
            if (param.isNumber()) {
                *pProp->value = param.getFloat();
                pProp->report.updateReport(param.getFloat());
            }
            break;
        case WrapperTypeT::WRAPPER_TYPE_STRING:
            if (param.isString()) {
                *pProp->value = param.getString();
                if (pProp->value->isUpdated()) {
                    pProp->report();
                }
            }
            break;
        default:
            return;
    }
    if (pProp->callback != nullptr) {
        pProp->callback();
    }
}

template<class Api>
typename ERaProperty<Api>::Property_t* ERaProperty<Api>::setupProperty(uint8_t pin, WrapperBase* value,
                                                                    PermissionT const permission) {
    if (!this->isPropertyFree()) {
        return nullptr;
    }
    if (value == nullptr) {
        return nullptr;
    }

    Property_t* pProp = new Property_t();
    if (pProp == nullptr) {
        return nullptr;
    }

    pProp->id = pin;
    pProp->value = value;
    pProp->callback = nullptr;
    pProp->prevMillis = ERaMillis();
    pProp->permission = permission;
    pProp->report = ERaReport::iterator();
    this->publishOnChange(pProp);
    this->ERaProp.put(pProp);
    this->numProperty++;
    return pProp;
}

template<class Api>
typename ERaProperty<Api>::Property_t* ERaProperty<Api>::setupProperty(const char* id, WrapperBase* value,
                                                                    PermissionT const permission) {
    if (!this->isPropertyFree()) {
        return nullptr;
    }
    if (id == nullptr) {
        return nullptr;
    }
    if (value == nullptr) {
        return nullptr;
    }

    Property_t* pProp = new Property_t();
    if (pProp == nullptr) {
        return nullptr;
    }

    pProp->id = id;
    pProp->value = value;
    pProp->callback = nullptr;
    pProp->prevMillis = ERaMillis();
    pProp->permission = permission;
    pProp->report = ERaReport::iterator();
    this->publishOnChange(pProp);
    this->ERaProp.put(pProp);
    this->numProperty++;
    return pProp;
}

#if defined(ERA_HAS_PROGMEM)
    template<class Api>
    typename ERaProperty<Api>::Property_t* ERaProperty<Api>::setupProperty(const __FlashStringHelper* id,
                                                                            WrapperBase* value, PermissionT const permission) {
        if (id == nullptr) {
            return nullptr;
        }
        PGM_P p = reinterpret_cast<PGM_P>(id);
        size_t size = strlen_P(p) + sizeof("");
        char* str = (char*)malloc(size);
        if (str == nullptr) {
            return nullptr;
        }
        memcpy_P(str, p, size);
        return this->setupProperty(str, value, permission);
    }
#endif

template<class Api>
bool ERaProperty<Api>::onUpdate(Property_t* pProp, ERaProperty::PropertyCallback_t cb) {
    if (pProp == nullptr) {
        return false;
    }
    if (!this->getFlag(pProp->permission, PermissionT::PERMISSION_WRITE)) {
        return false;
    }

    pProp->callback = cb;
    return true;
}

template<class Api>
bool ERaProperty<Api>::publishEvery(Property_t* pProp, unsigned long interval) {
    if (pProp == nullptr) {
        return false;
    }
    if (!this->getFlag(pProp->permission, PermissionT::PERMISSION_READ)) {
        return false;
    }

    if (pProp->report) {
        pProp->report.reportEvery(interval);
    }
    else {
        this->publishOnChange(pProp, 1.0f, 1000, interval);
    }
    return true;
}

template<class Api>
bool ERaProperty<Api>::publishOnChange(Property_t* pProp, float minChange,
                                        unsigned long minInterval,
                                        unsigned long maxInterval) {
    if (pProp == nullptr) {
        return false;
    }
    if (!this->getFlag(pProp->permission, PermissionT::PERMISSION_READ)) {
        return false;
    }

    if (pProp->report) {
        pProp->report.changeReportableChange(minInterval, maxInterval, minChange);
    }
    else {
        pProp->report = this->ERaPropRp.setReporting(minInterval, maxInterval, minChange, this->propertyCb, pProp);
    }
    if (pProp->value->isString()) {
        pProp->report.updateReport(0.0f, false, false);
    }
    else {
        this->updateValue(pProp);
    }
    return true;
}

template<class Api>
void ERaProperty<Api>::onCallbackVirtual(const Property_t* const pProp) {
    switch (pProp->value->getType()) {
        case WrapperTypeT::WRAPPER_TYPE_BOOL:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getBool());
            break;
        case WrapperTypeT::WRAPPER_TYPE_INT:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getInt());
            break;
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_INT:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getUnsignedInt());
            break;
        case WrapperTypeT::WRAPPER_TYPE_LONG:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getLong());
            break;
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_LONG:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getUnsignedLong());
            break;
        case WrapperTypeT::WRAPPER_TYPE_LONG_LONG:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getLongLong());
            break;
        case WrapperTypeT::WRAPPER_TYPE_UNSIGNED_LONG_LONG:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getUnsignedLongLong());
            break;
        case WrapperTypeT::WRAPPER_TYPE_FLOAT:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getFloat());
            break;
        case WrapperTypeT::WRAPPER_TYPE_DOUBLE:
            this->thisApi().virtualWrite(pProp->id, pProp->value->getDouble());
            break;
        case WrapperTypeT::WRAPPER_TYPE_STRING:
            if (pProp->value->getString() != nullptr) {
                this->thisApi().virtualWrite(pProp->id, pProp->value->getString());
            }
            break;
        default:
            break;
    }
}

template<class Api>
void ERaProperty<Api>::onCallbackReal(const Property_t* const pProp) {
    // Update later
    ERA_FORCE_UNUSED(pProp);
}

template<class Api>
void ERaProperty<Api>::onCallback(void* args) {
    ERaProperty::Property_t* pProp = (ERaProperty::Property_t*)args;
    if (pProp == nullptr) {
        return;
    }
    if (pProp->value == nullptr) {
        return;
    }
    if (pProp->id.isString()) {
        this->onCallbackReal(pProp);
    }
    else if (pProp->id.isNumber()) {
        this->onCallbackVirtual(pProp);
    }
}

template<class Api>
bool ERaProperty<Api>::isPropertyFree() {
    if (this->numProperty >= MAX_PROPERTYS) {
        return false;
    }

    return true;
}

#endif /* INC_ERA_PROPERTY_HPP_ */

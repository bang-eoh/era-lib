#ifndef INC_ERA_OTA_PORTENTA_H7_HPP_
#define INC_ERA_OTA_PORTENTA_H7_HPP_

#include <Arduino_Portenta_OTA.h>
#include <Utility/ERaUtility.hpp>

template <class Proto, class Flash>
class ERaOTA
{
    const char* TAG = "OTA";

public:
    ERaOTA(Flash& _flash)
        : flash(_flash)
    {}
    ~ERaOTA()
    {}

    void begin(const char* url = nullptr) {
        if (url == nullptr) {
            url = this->createUrl();
        }

        this->thisProto().getTransp().disconnect();
        ERA_LOG(TAG, ERA_PSTR("Firmware update URL: %s"), url);

        Arduino_Portenta_OTA::Error err = Arduino_Portenta_OTA::Error::None;
        Arduino_Portenta_OTA_QSPI qspiOTA(QSPI_FLASH_FATFS_MBR, 2);

        err = qspiOTA.begin();
        if (err != Arduino_Portenta_OTA::Error::None) {
            ERA_LOG(TAG, ERA_PSTR("Update begin failed"));
            return;
        }

        this->flash.end();

        remove("/fs/UPDATE.BIN");
        remove("/fs/UPDATE.BIN.LZSS");

#if defined(ERA_OTA_VIA_ETHERNET)
        MbedSocketClass* otaSocket = static_cast<MbedSocketClass*>(&Ethernet);
#else
        MbedSocketClass* otaSocket = static_cast<MbedSocketClass*>(&WiFi);
#endif
#if defined(ERA_OTA_SSL)
        int retCode = qspiOTA.download(url, true, otaSocket);
#else
        int retCode = qspiOTA.download(url, false, otaSocket);
#endif
        if (retCode < 0) {
            this->flash.begin();
            ERA_LOG(TAG, ERA_PSTR("Download file failed"));
            return;
        }
        retCode = qspiOTA.decompress();
        if (retCode < 0) {
            this->flash.begin();
            ERA_LOG(TAG, ERA_PSTR("Decompress file failed"));
            return;
        }

        err = qspiOTA.update();
        if (err != Arduino_Portenta_OTA::Error::None) {
            this->flash.begin();
            ERA_LOG(TAG, ERA_PSTR("Update failed"));
            return;
        }

        ERA_LOG(TAG, ERA_PSTR("Update successfully. Rebooting!"));
        ERaDelay(1000);
        ERaRestart(true);
    }

protected:
private:
    const char* createUrl() {
        static char url[128] {0};
        if (!strlen(url)) {
#if defined(ERA_OTA_SSL)
            FormatString(url, "https://" ERA_SSL_DOMAIN);
#else
            FormatString(url, "http://" ERA_NOSSL_DOMAIN);
#endif
            FormatString(url, "/api/chip_manager");
            FormatString(url, "/firmware?code=%s", this->thisProto().getAuth());
            FormatString(url, "&firm_version=%d.%d", ERA_FIRMWARE_MAJOR, ERA_FIRMWARE_MINOR);
            FormatString(url, "&board=%s", ERA_MODEL_TYPE);
        }
        return url;
    }

	inline
	const Proto& thisProto() const {
		return static_cast<const Proto&>(*this);
	}

	inline
	Proto& thisProto() {
		return static_cast<Proto&>(*this);
	}

    Flash& flash;
};

#endif /* INC_ERA_OTA_PORTENTA_H7_HPP_ */

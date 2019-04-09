/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license
 **************************************************************/

#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <memory>

extern "C" {
  #include "user_interface.h"
}

const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:tahoma;} button{border: 2px solid black;border-radius:0.3rem;background-color:#297bb7;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;border-style: solid;padding: 20px;'><img style='margin-left: 190px;'src='data:image/png;base64, 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'/><h2>IoT temperature sensor configuration portal</h2><p style='font-style: italic;'>Hello friend. You can use this portal to configure the following things: <ul style='font-style: italic;'><li>Scan Wifi networks and configure credentials</li><li>Stablish delays betwheen measures</li><li>Configure a temperature calibration offset</li><li>Set custom device ID</li><li>Set IoT Server and port</li></ul></p>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure All</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div>Parameters Saved<br />Trying to connect Rhomb.io device to network.<br />If it fails reconnect to AP to try again</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";

#ifndef WIFI_MANAGER_MAX_PARAMS
#define WIFI_MANAGER_MAX_PARAMS 10
#endif

class WiFiManagerParameter {
  public:
    /** 
        Create custom parameters that can be added to the WiFiManager setup web page
        @id is used for HTTP queries and must not contain spaces nor other special characters
    */
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);
    ~WiFiManagerParameter();

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    friend class WiFiManager;
};


class WiFiManager
{
  public:
    WiFiManager();
    ~WiFiManager();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

    //if you want to always start the config portal, without trying to connect first
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //useful for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, useful if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManager*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter, returns false on failure
    bool          addParameter(WiFiManagerParameter *p);
    //if this is set, it will exit after config, even if connection is unsuccessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot();
    void          handleWifi(boolean scan);
    void          handleWifiSave();
    void          handleInfo();
    void          handleReset();
    void          handleNotFound();
    void          handle204();
    boolean       captivePortal();
    boolean       configPortalHasTimeout();

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    int                    _max_params;
    WiFiManagerParameter** _params;

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};

#endif
